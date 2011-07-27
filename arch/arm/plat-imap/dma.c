
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/dmapool.h>

#include <mach/hardware.h>
#include <asm/irq.h>

#include <plat/imapx.h>
#include <plat/dma.h>



/* pool to provide LLI buffers */
static struct dma_pool *dma_pool;
struct imapx200_dma_chan imapx200_chans[IMAP_DMA_CHANNELS];
struct imapx200_dma_chan *imap_dma_chan_map[IMAP_DMA_CHANNELS];
#define imapx200_dma_debug
#ifdef imapx200_dma_debug
#define pr_debug(fmt...) printk(fmt)
#else
#define pr_debug(fmt...) 
#endif

//char *imapx200_dma_client_name[]



struct imapx200_dma_chan *imap_dma_lookup_channel(unsigned int channel)
{
	if (channel)
		return &imapx200_chans[channel];
	else
		return imap_dma_chan_map[channel];
}




static void imapx200_dma_fill_lli(struct imapx200_dma_chan *chan,
				 struct dw_lli *lli,
				 dma_addr_t data, int size)
{
	dma_addr_t src, dst;
	u32 control0, control1;

	control0 = chan_readl(chan,CTL_LO); 

	switch (chan->source) {
	case IMAPX200_DMA_P2M:
		src = chan->dev_addr;
		dst = data;
		control0 |= DWC_CTLL_DST_WIDTH(chan->hw_width);
		control0 |= DWC_CTLL_SRC_WIDTH(2);
		control0 |= DWC_CTLL_DST_INC;
		break;

	case IMAPX200_DMA_M2P:
		src = data;
		dst = chan->dev_addr;
		control0 |= DWC_CTLL_DST_WIDTH(2);
		control0 |= DWC_CTLL_SRC_WIDTH(chan->hw_width);
		control0 |= DWC_CTLL_SRC_INC;
		break;
	default:
		BUG();
	}

	/* note, we do not currently setup any of the burst controls */

	control1 = size >> chan->hw_width;	/* size in no of xfers */

	lli->sar = src;
	lli->dar = dst;
	lli->llp = 0;
	lli->ctllo = control0;
	lli->ctlhi = control1;
}

static void imapx200_lli_to_regs(struct imapx200_dma_chan *chan,
				struct dw_lli *lli)
{
	pr_debug("%s: LLI %p => regs\n", __func__, lli);

	chan_writel(chan,SAR,lli->sar);
	chan_writel(chan,DAR,lli->dar);
	chan_writel(chan,LLP,lli->llp);
	chan_writel(chan,CTL_LO,lli->ctllo);
	chan_writel(chan,CTL_HI,lli->ctlhi);
}


static void dbg_showchan(struct imapx200_dma_chan *chan)
{

	pr_debug("DMA%d: %08x->%08x L %08x CTL_L0 %08x, CTL_H0%08x \n",
		 chan->number,
		 chan_readl(chan,SAR),
		 chan_readl(chan,DAR),
		 chan_readl(chan,LLP),
		 chan_readl(chan,CTL_LO),
		 chan_readl(chan,CTL_HI)
		 );
}


static struct imapx200_dma_chan *imapx200_dma_map_channel(unsigned int channel)
{
	struct imapx200_dma_chan *chan;
	unsigned int start, offs;

	start = 0;

	pr_debug("%s: map_channel!\n", __func__);
	for (offs = 0; offs < 8; offs++) {
		chan = &imapx200_chans[start + offs];
		if (!chan->in_use)
			goto found;
	}

	return NULL;

found:
	imap_dma_chan_map[channel] = chan;
	return chan;
}

static inline void imapx200_dma_bufffdone(struct imapx200_dma_chan *chan,
					 struct imapx200_dma_buff *buf,
					 enum imapx200_dma_buffresult result)
{
	if (chan->callback_fn != NULL)
		(chan->callback_fn)(chan, buf->pw, 0, result);
}

static void imapx200_dma_freebuff(struct imapx200_dma_buff *buff)
{
	dma_pool_free(dma_pool, buff->lli, buff->lli_dma);
	kfree(buff);
}

static int imapx200_dma_start(struct imapx200_dma_chan *chan)
{
	struct imapx200_dmac *dmac= chan->dmac;
	u32 bit = chan->bit;

	pr_debug("%s: clearing interrupts\n", __func__);

	/* clear interrupts */
	dma_writel(dmac,CLEAR.XFER ,bit);
	dma_writel(dmac,CLEAR.ERROR ,bit);

	pr_debug("%s: starting channel\n", __func__);
	//enable dma channel 
	dma_set_bit(dmac,CH_EN,bit);

	return 0;
}

static int imapx200_dma_stop(struct imapx200_dma_chan *chan)
{
	struct imapx200_dmac *dmac= chan->dmac;
	u32 config;
	int timeout;

	pr_debug("%s: stopping channel\n", __func__);

	config = chan_readl(chan,CFG_LO);
	config |= DWC_CFGL_CH_SUSP;
	chan_writel(chan,CFG_LO,config);

	timeout = 1000;
	do {
		config = chan_readl(chan,CFG_LO);
		pr_debug("%s: %d - config %08x\n", __func__, timeout, config);
		if (config & DWC_CFGL_CH_SUSP)
			udelay(10);
		else
			break;
	} while (--timeout > 0);

	if (config & DWC_CFGL_CH_SUSP) {
		printk(KERN_ERR "%s: channel still active\n", __func__);
		return -EFAULT;
	}

	dma_clear_bit(dmac,CH_EN,chan->bit);
	return 0;
}

static int imapx200_dma_flush(struct imapx200_dma_chan *chan)
{
	struct imapx200_dmac *dmac= chan->dmac;
	struct imapx200_dma_buff *buff, *next;

	pr_debug("%s: flushing channel\n", __func__);


	dma_clear_bit(dmac,CH_EN,chan->bit);
	/* dump all the buffers associated with this channel */

	for (buff = chan->curr; buff != NULL; buff = next) {
		next = buff->next;
		pr_debug("%s: buff %p (next %p)\n", __func__, buff, buff->next);

		imapx200_dma_bufffdone(chan, buff, IMAPX200_RES_ABORT);
		imapx200_dma_freebuff(buff);
	}

	chan->curr = chan->next = chan->end = NULL;


	return 0;
}


//
//  dma extern api
//

int imapx200_dma_config(unsigned int channel, int xferunit)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	switch (xferunit) {
	case 1:
		chan->hw_width = 0;
		break;
	case 2:
		chan->hw_width = 1;
		break;
	case 4:
		chan->hw_width = 2;
		break;
	default:
		printk(KERN_ERR "%s: illegal width %d\n", __func__, xferunit);
		return -EINVAL;
	}
	
	return 0;
}
EXPORT_SYMBOL(imapx200_dma_config);


int imapx200_dma_ctrl(unsigned int channel, enum imapx200_chan_op op)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);

	WARN_ON(!chan);
	if (!chan)
		return -EINVAL;

	switch (op) {
	case IMAPX200_DMAOP_START:
		return imapx200_dma_start(chan);

	case IMAPX200_DMAOP_STOP:
		return imapx200_dma_stop(chan);

	case IMAPX200_DMAOP_FLUSH:
		return imapx200_dma_flush(chan);

	/* belive PAUSE/RESUME are no-ops */
	case IMAPX200_DMAOP_PAUSE:
	case IMAPX200_DMAOP_RESUME:
	case IMAPX200_DMAOP_STARTED:
	case IMAPX200_DMAOP_TIMEOUT:
		return 0;
	}


	return -ENOENT;
}
EXPORT_SYMBOL(imapx200_dma_ctrl);

int imapx200_dma_set_buffdone_fn(unsigned int channel, imapx200_dma_cbfn_t rtn)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);

	if (chan == NULL)
		return -EINVAL;

	pr_debug("%s: chan=%p, callback rtn=%p\n", __func__, chan, rtn);

	chan->callback_fn = rtn;

	return 0;
}
EXPORT_SYMBOL(imapx200_dma_set_buffdone_fn);



int imapx200_dma_enqueue(unsigned int channel, void *id,
			dma_addr_t data, int size)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);
	struct imapx200_dma_buff *next;
	struct imapx200_dma_buff *buff;
	struct dw_lli *lli;
	int ret;

	WARN_ON(!chan);
	if (!chan)
		return -EINVAL;

	buff = kzalloc(sizeof(struct imapx200_dma_buff), GFP_KERNEL);
	if (!buff) {
		printk(KERN_ERR "%s: no memory for buffer\n", __func__);
		return -ENOMEM;
	}


	lli = dma_pool_alloc(dma_pool, GFP_KERNEL, &buff->lli_dma);
	if (!lli) {
		printk(KERN_ERR "%s: no memory for lli\n", __func__);
		ret = -ENOMEM;
		goto err_buff;
	}

	pr_debug("%s: buff %p, dp %08x lli (%p, %08x) %d\n",
		 __func__, buff, data, lli, (u32)buff->lli_dma, size);

	buff->lli = lli;
	buff->pw = id;

	imapx200_dma_fill_lli(chan, lli, data, size);

	if ((next = chan->next) != NULL) {
		struct imapx200_dma_buff *end = chan->end;
		struct dw_lli *endlli = end->lli;

		pr_debug("enquing onto channel\n");

		end->next = buff;
		endlli->llp = buff->lli_dma;

		if (chan->flags ) {
			struct imapx200_dma_buff *curr = chan->curr;
			lli->llp = curr->lli_dma;
		}

		if (next == chan->curr) {
			chan_writel(chan,LLP,buff->lli_dma);
			chan->next = buff;
		}

		chan->end = buff;
	} else {
		pr_debug("enquing onto empty channel\n");

		chan->curr = buff;
		chan->next = buff;
		chan->end = buff;

		imapx200_lli_to_regs(chan, lli);
	}

	dbg_showchan(chan);
	return 0;

err_buff:
	kfree(buff);
	
	return ret;
}
EXPORT_SYMBOL(imapx200_dma_enqueue);


int imapx200_dma_devconfig(int channel,
			  enum imapx200_dmafc source,
			  unsigned long devaddr)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);
	u32 peripheral;
	u32 ctlx = 0;
	u32 intrx = 0;
	u32 cfg_hi = 0;
	u32 cfg_lo = 0;

	pr_debug("%s: channel %d, source %d, dev %08lx, chan %p\n",
		 __func__, channel, source, devaddr, chan);

	WARN_ON(!chan);
	if (!chan)
		return -EINVAL;

	peripheral = (chan->peripheral & 0x7);
	chan->source = source;
	chan->dev_addr = devaddr;

	pr_debug("%s: peripheral %d\n", __func__, peripheral);
	ctlx = chan_readl(chan,CTL_LO);
	pr_debug("devconfig_1\n");
	ctlx &= ~DWC_CTLL_FC_MASK;
	cfg_hi = chan_readl(chan,CFG_HI);
	pr_debug("CFG_HI is %x\n", cfg_hi);
	cfg_lo = chan_readl(chan,CFG_LO);
	pr_debug("devconfig_3\n");
	switch (source) {
	case IMAPX200_DMA_M2M:
		ctlx |= DWC_CTLL_FC_M2M;
		break;
	case IMAPX200_DMA_M2P:
		ctlx |= DWC_CTLL_FC_M2P;
		cfg_lo &= ~DWC_CFGL_HS_DST;
		cfg_hi |= DWC_CFGH_DST_PER(chan->client->handshake);
		break;
	case IMAPX200_DMA_P2M:
		ctlx |= DWC_CTLL_FC_P2M; 
		cfg_lo &= ~DWC_CFGL_HS_SRC;
		cfg_hi |= DWC_CFGH_SRC_PER(chan->client->handshake);
		break;
	default:
		printk(KERN_ERR "%s: bad source\n", __func__);
		return -EINVAL;
	}
	/*set dma flow control bit*/
	chan_writel(chan,CTL_LO,ctlx);
	pr_debug("devconfig_4\n");
	chan_writel(chan,CFG_LO,cfg_lo);
	pr_debug("devconfig_5\n");
	chan_writel(chan,CFG_HI,cfg_hi);
//	cfg_hi = chan_readl(chan,CFG_HI);
//	pr_debug("CFG_HI is %x\n", cfg_hi);
	/* allow TC and ERR interrupts */
	intrx = 1<<(chan->number);
	pr_debug("devconfig_6\n");
	dma_set_bit(chan->dmac,MASK.XFER,intrx);
	pr_debug("devconfig_7\n");
	dma_set_bit(chan->dmac,MASK.BLOCK,intrx);
	dma_set_bit(chan->dmac,MASK.ERROR,intrx);

	pr_debug("devconfig_8\n");
	return 0;
}
EXPORT_SYMBOL(imapx200_dma_devconfig);


int imapx200_dma_getposition(unsigned int channel,
			    dma_addr_t *src, dma_addr_t *dst)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);

	WARN_ON(!chan);
	if (!chan)
		return -EINVAL;

	if (src != NULL)
		*src = chan_readl(chan,SAR);

	if (dst != NULL)
		*dst = chan_readl(chan,DAR);

	return 0;
}
EXPORT_SYMBOL(imapx200_dma_getposition);

int imapx200_dma_request(unsigned int channel,
			struct imapx200_dma_client *client,
			void *dev)
{
	struct imapx200_dma_chan *chan;
	unsigned long flags;

	pr_debug("dma%d: imapx200_request_dma: client->name=%s,client->handshake=%d\n",
		 channel, client->name,client->handshake);

	local_irq_save(flags);

	chan = imapx200_dma_map_channel(channel);
	if (chan == NULL) {
		local_irq_restore(flags);
		return -EBUSY;
	}

	//dbg_showchan(chan);
//	pr_debug("dma_request_2\n");
	chan->client = client;
//	chan->client->name = client->name;
//	pr_debug("dma_request_3\n");
//	chan->client->handshake = client->handshake;
//	pr_debug("dma_request_4\n");
	chan->in_use = 1;
	chan->peripheral = channel;
	local_irq_restore(flags);

	/* need to setup */
	pr_debug("%s: channel initialised, %p\n", __func__, chan);
	return chan->number;
}
EXPORT_SYMBOL(imapx200_dma_request);

int imapx200_dma_free(unsigned int channel, struct imapx200_dma_client *client)
{
	struct imapx200_dma_chan *chan = imap_dma_lookup_channel(channel);
	unsigned long flags;

	if (chan == NULL)
		return -EINVAL;

	local_irq_save(flags);

	if (chan->client != client) {
		printk(KERN_WARNING "dma%d: possible free from different client (channel %p, passed %p)\n",
		       channel, chan->client, client);
	}

	/* sort out stopping and freeing the channel */


	chan->client = NULL;
	chan->in_use = 0;

	if (!channel )
		imap_dma_chan_map[channel] = NULL;

	local_irq_restore(flags);


	return 0;
}

EXPORT_SYMBOL(imapx200_dma_free);

static void imapx200_dma_buffdone(struct imapx200_dma_chan *chan, struct imapx200_dma_buff *buf,enum imapx200_dma_buffresult result)
{

	if (chan->callback_fn != NULL)
		(chan->callback_fn)(chan, buf->pw, 0, result);
}

static void imapx200_dma_errirq(struct imapx200_dmac *dmac, int offs)
{
	printk(KERN_DEBUG "%s: offs %d\n", __func__, offs);
}


static irqreturn_t imapx200_dma_irq(int irq, void *pw)
{
	struct imapx200_dmac *dmac = pw;
	u32 tcstat, errstat;
	u32 bit;
	int offs;
	struct imapx200_dma_chan *chan = NULL;
	struct imapx200_dma_buff *buf;
	unsigned int channel = 0;

	tcstat = dma_readl(dmac,STATUS.XFER );
	errstat = dma_readl(dmac,STATUS.ERROR);

	for (offs = 0, bit = 1; offs < 8; offs++, bit <<= 1) {
		if (tcstat & bit) {
			dma_set_bit(dmac, CLEAR, bit);
			channel = offs;
			chan = &imapx200_chans[channel];
			buf = chan->curr;
			if(buf != NULL)
			{
				chan->curr = buf->next;
				buf->next = NULL;
				imapx200_dma_buffdone(chan,buf,IMAPX200_RES_OK);
				imapx200_dma_freebuff(buf);
			}
			else
			{
			}
		}


		if (errstat & bit) {
			imapx200_dma_errirq(dmac, offs);
			dma_set_bit(dmac, CLEAR, bit);
		}
	}

	return IRQ_HANDLED;
}



static struct sysdev_class dma_sysclass = {
	.name		= "imapx200-dma",
};


static int imapx200_dma_init_xxx(int chno, int dma_ch,
			     int irq, unsigned int base)
{
	struct imapx200_dma_chan *chptr = &imapx200_chans[chno];
	struct imapx200_dmac *dmac;
	char clkname[16];
	void __iomem *regs;
	void __iomem *regptr;
	int err, ch;

	dmac = kzalloc(sizeof(struct imapx200_dmac), GFP_KERNEL);
	if (!dmac) {
		printk(KERN_ERR "%s: failed to alloc mem\n", __func__);
		return -ENOMEM;
	}

	dmac->sysdev.id = chno / 8;
	dmac->sysdev.cls = &dma_sysclass;

	err = sysdev_register(&dmac->sysdev);
	if (err) {
		printk(KERN_ERR "%s: failed to register sysdevice\n", __func__);
		goto err_alloc;
	}

	regs = ioremap(base, DW_REGLEN);
	if (!regs) {
		printk(KERN_ERR "%s: failed to ioremap()\n", __func__);
		err = -ENXIO;
		goto err_dev;
	}

	snprintf(clkname, sizeof(clkname), "dma%d", dmac->sysdev.id);
/*
	dmac->clk = clk_get(NULL, clkname);
	if (IS_ERR(dmac->clk)) {
		printk(KERN_ERR "%s: failed to get clock %s\n", __func__, clkname);
		err = PTR_ERR(dmac->clk);
		goto err_map;
	}

	clk_enable(dmac->clk);
*/
	dmac->regs = regs;
	dmac->dma_ch = dma_ch;
	dmac->channels = chptr;

	err = request_irq(irq, imapx200_dma_irq, IRQF_DISABLED, "DMA", dmac);
	if (err < 0) {
		printk(KERN_ERR "%s: failed to get irq\n", __func__);
		goto err_clk;
	}
 
	regptr = regs + (0);

	for (ch = 0; ch < 8; ch++, chno++, chptr++) {
		printk(KERN_INFO "%s: registering DMA %d (%p)\n",
		       __func__, chno, regptr);

		chptr->bit = 1 << ch;
		chptr->number = chno;
		chptr->regs = regptr;
		regptr += DW_CH_STRIDE;
		dma_clear_bit(dmac,CH_EN,chptr->bit);
	}

	/* for the moment, permanently enable the controller */
	dma_writel(dmac, CFG, DW_CFG_DMA_EN);

	printk(KERN_INFO "DW: IRQ %d, at %p\n", irq, regs);

	return 0;
err_clk:
	clk_disable(dmac->clk);
	clk_put(dmac->clk);
err_map:
	iounmap(regs);
err_dev:
	sysdev_unregister(&dmac->sysdev);
err_alloc:
	kfree(dmac);
	return err;	
}



static int __init imapx200_dma_init(void)
{
	int ret;
	printk(KERN_INFO "%s: Registering DMA channels\n", __func__);

	dma_pool = dma_pool_create("DMA-LLI", NULL, 32, 16, 0);
	if (!dma_pool) {
		printk(KERN_ERR "%s: failed to create pool\n", __func__);
		return -ENOMEM;
	}

	ret = sysdev_class_register(&dma_sysclass);
	if (ret) {
		printk(KERN_ERR "%s: failed to create sysclass\n", __func__);
		return -ENOMEM;
	}

	imapx200_dma_init_xxx(0,0,IRQ_DMA,DMA_BASE_REG_PA);

	return 0;
}
arch_initcall(imapx200_dma_init);

