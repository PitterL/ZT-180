
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
#include <plat/dmalib.h>
#ifdef CONFIG_IMAPX200_DMA_DEBUG
#define pr_debug(x...) printk(x)
#else
#define pr_debug(x...)
#endif
/* pool to provide LLI buffers */
static struct dma_pool *dma_pool;
struct imapx200_dma_chan imapx200_chans[IMAP_DMA_CHANNELS];
//struct imapx200_dma_chan *imap_dma_chan_map[IMAP_DMA_CHANNELS];

extern u32 imapx200_dma_init_lli(struct imapx200_dma_chan *chan);

extern u32 imapx200_dma_init_cmn(struct imapx200_dma_chan *chan);
//char *imapx200_dma_client_name[]
static irq_number = 0;


struct imapx200_dma_chan *imap_dma_lookup_channel(unsigned int channel)
{
		return &imapx200_chans[channel];

}


static struct imapx200_dma_chan *imapx200_dma_map_channel(unsigned int channel)
{
	struct imapx200_dma_chan *chan;
	unsigned int start, offs;

	start = channel;

	for (offs = 0; offs < (8-channel); offs++) {
		chan = &imapx200_chans[start + offs];
		if (!chan->in_use)
			goto found;
	}

	return NULL;

found:
	return chan;
}

static void dbg_showchan(struct imapx200_dma_chan *chan)
{

	pr_debug("DMA%d: %08x->%08x L %08x C %08x,%08x \n",
		 chan->number,
		 chan_readl(chan,SAR),
		 chan_readl(chan,DAR),
		 chan_readl(chan,LLP),
		 chan_readl(chan,CTL_LO),
		 chan_readl(chan,CTL_HI)
		 );
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
	dma_writel(dmac, CFG, DW_CFG_DMA_EN);
	//dma channel 
	dma_set_bit(dmac,CH_EN,bit);

	return 0;
}

static int imapx200_dma_stop(struct imapx200_dma_chan *chan)
{
	struct imapx200_dmac *dmac= chan->dmac;
	u32 config;
	int timeout, i;

	pr_debug("%s: stopping channel\n", __func__);

	irq_number = 0;
//	config = chan_readl(chan,CFG_LO);
//	config |= DWC_CFGL_CH_SUSP;
	config = DWC_CFGL_CH_SUSP;
	chan_writel(chan,CFG_LO,config);
	if (chan->plli)
	kfree(chan->plli);
	chan->plli = NULL;
	
	timeout = 1000;
	do {
		config = chan_readl(chan,CFG_LO);
		//if (config & DWC_CFGL_CH_SUSP)
		if (config & DWC_CFGL_FIFO_EMPTY)
			break;
			//udelay(10);
		else
			//break;
			udelay(10);
	} while (--timeout > 0);

	if (config & DWC_CFGL_CH_SUSP) {
		dma_close_bit(dmac,CH_EN,chan->bit);
		return -EFAULT;
	}

	dma_writel(dmac, CFG, 0);
	dma_close_bit(dmac,CH_EN,chan->bit);
	return 0;
}

static int imapx200_dma_flush(struct imapx200_dma_chan *chan)
{
	struct imapx200_dmac *dmac= chan->dmac;
	struct imapx200_dma_buff *buff, *next;

	pr_debug("%s: flushing channel\n", __func__);


	//dma_clear_bit(dmac,CH_EN,chan->bit);
	dma_close_bit(dmac,CH_EN,chan->bit);
	/* dump all the buffers associated with this channel */
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

	if (chan->client->type == IMAPX200_DMA_CMN) {
		imapx200_dma_init_cmn(chan);
	} else {
		imapx200_dma_init_lli(chan);		
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
	chan->pw = id;

	return 0;
}
EXPORT_SYMBOL(imapx200_dma_enqueue);

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

	pr_debug("dma%d: imapx200_request_dma: client->name=%s, dev=%p\n",
		 channel, client->name, dev);

	local_irq_save(flags);
	chan = imapx200_dma_map_channel(channel);
	if (chan == NULL) {
		local_irq_restore(flags);
		return -EBUSY;
	}

	chan->client = client;
	chan->in_use = 1;
	chan->peripheral = channel;
	chan->plli = NULL;
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
/*
	if (!channel )
		imap_dma_chan_map[channel] = NULL;
*/
	local_irq_restore(flags);


	return 0;
}

EXPORT_SYMBOL(imapx200_dma_free);

static void imapx200_dma_buffdone(struct imapx200_dma_chan *chan,enum imapx200_dma_buffresult result)
{

	if (chan->callback_fn != NULL)
		(chan->callback_fn)(chan, chan->pw, 0, result);
}

static void imapx200_dma_errirq(struct imapx200_dmac *dmac, int offs)
{
	pr_debug(KERN_INFO"%s: offs %d\n", __func__, offs);
}

static irqreturn_t imapx200_dma_tfre(int irq, void *pw)
{
	struct imapx200_dmac *dmac = pw;
	u32 tcstat;
	u32 bit;
	int offs;
	struct imapx200_dma_chan *chan = NULL;
	unsigned int channel = 0;
	tcstat = dma_readl(dmac,STATUS.XFER);
	for (offs = 0, bit = 1; offs < 8; offs++, bit <<= 1) {
		if (tcstat & bit) {
			dma_set_bit(dmac, CLEAR.XFER, bit);
			pr_debug(KERN_INFO "-----clear.xfer!\n");
		//	channel = offs;
		//	chan = &imapx200_chans[channel];
		//	imapx200_dma_buffdone(chan,pw,IMAPX200_RES_OK);
		}
	}
	return IRQ_HANDLED;
}

static irqreturn_t imapx200_dma_blk(int irq, void *pw)
{
	struct imapx200_dmac *dmac = pw;
	u32 tcstat;
	u32 bit;
	int offs;
	struct imapx200_dma_chan *chan = NULL;
	unsigned int channel = 0;
	tcstat = dma_readl(dmac,STATUS.BLOCK );
	for (offs = 0, bit = 1; offs < 8; offs++, bit <<= 1) {
		if (tcstat & bit) {
			channel = offs;
			chan = &imapx200_chans[channel];
			pr_debug("DAR is %x, SAR is %x\n", chan_readl(chan, DAR), chan_readl(chan, SAR));
			dma_set_bit(dmac, CLEAR.BLOCK, bit);
			imapx200_dma_buffdone(chan,IMAPX200_RES_OK);
		}
		//	dma_set_bit(dmac, CLEAR.BLOCK, bit);
	}
	return IRQ_HANDLED;
}
static irqreturn_t imapx200_dma_irq(int irq, void *pw)
{
	struct imapx200_dmac *dmac = pw;
	u32 intrstat, tcstat, errstat;
	u32 bit;
	int offs;
	struct imapx200_dma_chan *chan = NULL;
	unsigned int channel = 0;
	irq_number=irq_number+1;
	irqreturn_t ret = IRQ_HANDLED;
	intrstat = dma_readl(dmac,STATUS_INT);
	tcstat = dma_readl(dmac,STATUS.XFER );
	errstat = dma_readl(dmac,STATUS.ERROR);
		switch(intrstat & 0x1f){
			case STATUSINT_TFRE :
				ret = imapx200_dma_tfre(irq, pw);
				break;
			case STATUSINT_BLK:
				ret = imapx200_dma_blk(irq, pw);
				break;
			case STATUSINT_ERR:
			imapx200_dma_errirq(dmac, offs);
			dma_set_bit(dmac, CLEAR.ERROR, bit);
				break;
			case STATUSINT_SRCT:
			case STATUSINT_DSTT:
				break;
			default:
				//printk(KERN_INFO "----the value is invalid!\n");
				break;
		}

	return ret;
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
		pr_debug(KERN_INFO "%s: registering DMA %d (%p)\n",
		       __func__, chno, regptr);

		chptr->bit = 1 << ch;
		chptr->number = chno;
		chptr->dmac = dmac;
		chptr->regs = regptr;
		regptr += DW_CH_STRIDE;
		//dma_clear_bit(dmac,CH_EN,chptr->bit);
		dma_close_bit(dmac,CH_EN,chptr->bit);
	}

	/* for the moment, permanently enable the controller */
//	dma_writel(dmac, CFG, DW_CFG_DMA_EN);

	pr_debug(KERN_INFO "DW: IRQ %d, at %p\n", irq, regs);

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
	printk(KERN_INFO "%s: Registering DMA channels: ok!\n", __func__);

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

