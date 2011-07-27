//#include <plat/regs-dma.h>
#include <linux/sysdev.h>
#include <plat/dmalib.h>
#define IMAP_DMA_CHANNELS   8	

enum imapx200_dma_type {
	IMAPX200_DMA_CMN,
	IMAPX200_DMA_LLI
};

enum imapx200_dma_buffresult {
	IMAPX200_RES_OK,
	IMAPX200_RES_ERR,
	IMAPX200_RES_ABORT
};

/* enum imapx200_chan_op
 *
 * operation codes passed to the DMA code by the user, and also used
 * to inform the current channel owner of any changes to the system state
*/

enum imapx200_chan_op {
	IMAPX200_DMAOP_START,
	IMAPX200_DMAOP_STOP,
	IMAPX200_DMAOP_PAUSE,
	IMAPX200_DMAOP_RESUME,
	IMAPX200_DMAOP_FLUSH,
	IMAPX200_DMAOP_TIMEOUT,		/* internal signal to handler */
	IMAPX200_DMAOP_STARTED,		/* indicate channel started */
};


enum imapx200_dmafc {
	IMAPX200_DMA_M2M,		/* source is memory */
	IMAPX200_DMA_M2P,		/* source is hardware */
	IMAPX200_DMA_P2M,
	IMAPX200_DMA_P2P,
	IMAPX200_PHI_P2M,
	IMAPX200_SPHI_P2P,
	IMAPX200_PHI_M2P,
	IMAPX200_DPHI_P2P,
};


struct imapx200_dma_ch {
	u32 block_size;
	//
	u32 src_addr;
	u32 dst_addr;
	//
	u8 src_datawidth;	//bit length, 8/16/32...
	u8 dst_datawidth;
	//
	u8 src_burstlen;
	u8 dst_burstlen;

	u8 flow_ctrl;

	u8 src_incre;
	u8 dst_incre;
	u8 intr_en;
	
	//HANDSHAKE
	u8 hs_src_type;	// 0=no, 1=hard, 2=soft
	u8 hs_src_index;	//hard-handshake's interface index
	u8 hs_dst_type;	// 0=no, 1=hard, 2=soft
	u8 hs_dst_index;	//hard-handshake's interface index};

	u8 trans_type;
	u8 reload_type;
	// 2D-transfer(Scatter/Gather)
	u8 enable_2d;
	u32 src2d_size;
	u32 dst2d_size;
	u32 src2d_interval;
	u32 dst2d_interval;
	//write back
	u8 enable_wb;
	u32 srcwb_addr;
	u32 dstwb_addr;
};	


struct imapx200_dma_client {
	char	*name;

	//
	enum imapx200_dma_type type;		//Common(0) or LLI(1)
	//
	u32 block_num;
//	u8 intr_en;
	
	//MULTI
	u8 trans_type;		//SINGLE/LLI=0/Multi/
	u8 reload_type;	//SRC/DST

	struct imapx200_dma_ch *pch;
	u8 is_loop; //cyclic 
};


/* imapx200_dma_buf
 *
 * internally used buffer structure to describe a queued or running
 * buffer.
*/
struct imapx200_dma_buff {
	struct imapx200_dma_buff	*next;
    void		*pw;
	struct dw_lli *lli;
	dma_addr_t		lli_dma;
};

struct imapx200_dma_chan;
/* imapx200_dma_cbfn_t
 *
 * buffer callback routine type
*/

typedef void (*imapx200_dma_cbfn_t)(struct imapx200_dma_chan *,
				   void *buf, int size,
				   enum imapx200_dma_buffresult result);

typedef int  (*imapx200_dma_opfn_t)(struct imapx200_dma_chan *,
				   enum imapx200_chan_op );


/* struct imapx200_dma_chan
 *
 * full state information for each DMA channel
*/
struct imapx200_dma_chan {
	/* channel state flags and information */
	unsigned char		 number;      /* number of this dma channel */
	unsigned char		 in_use;      /* channel allocated */
	unsigned char		 bit;
	unsigned char		 hw_width;
	unsigned char		 peripheral;

	/* channel state */
	struct imapx200_dma_client *client;
	struct imapx200_dmac *dmac;

	/* lli point */
	struct dma_lli *plli;
	
	/* channel configuration */
	unsigned int		 flags;		/* channel flags */
	enum imapx200_dmafc	 source;
	dma_addr_t	 dev_addr;

	/* channel's hardware position and configuration */
	struct dw_dma_chan_regs __iomem		*regs;		/* channels registers */

	/* driver handles */
	imapx200_dma_cbfn_t	 callback_fn;	/* buffer done callback */
	imapx200_dma_opfn_t	 op_fn;		/* channel op callback */
	void *pw;
	/* buffer list and information */
	struct imapx200_dma_buff	*curr;		/* current dma buffer */
	struct imapx200_dma_buff	*next;		/* next buffer to load */
	struct imapx200_dma_buff	*end;		/* end of queue */

};

/* dma channel state information */
struct imapx200_dmac {
	struct	sys_device	 sysdev;
	struct	clk		*clk;
	struct	dw_dma_regs  __iomem		*regs;
	struct	imapx200_dma_chan *channels;
	int		dma_ch;
};


static inline struct dw_dma_regs __iomem*
__dma_regs(struct imapx200_dmac *chan)
{
	return chan->regs;
}

static inline struct dw_dma_chan_regs __iomem*
__chan_regs(struct imapx200_dma_chan *chan)
{
	return chan->regs;
}

#define chan_readl(chan,reg)\
	__raw_readl(&(__chan_regs(chan)->reg))
#define chan_writel(chan,reg,val)\
	__raw_writel((val),&(__chan_regs(chan)->reg))

#define dma_readl(chan,reg)\
	__raw_readl(&(__dma_regs(chan)->reg))
#define dma_writel(chan,reg,val)\
	__raw_writel((val),&(__dma_regs(chan)->reg))
#define dma_set_bit(chan,reg,mask)\
	dma_writel(chan,reg,((mask)<<8) | (mask))
#define dma_clear_bit(chan,reg,mask)\
	dma_writel(chan,reg,(~(mask)<<8) | 0)			
#define dma_close_bit(chan,reg,mask)\
	dma_writel(chan,reg,((mask)<<8) | 0)			





/* imapx200_dma_config
 *
 * configure the dma channel
*/

extern int imapx200_dma_config(unsigned int channel, int xferunit);

/* imapx200_dma_ctrl
 *
 * change the state of the dma channel
*/

extern int imapx200_dma_ctrl(unsigned int channel, enum imapx200_chan_op op);

/* imapx200_dma_enqueue
 *
 * place the given buffer onto the queue of operations for the channel.
 * The buffer must be allocated from dma coherent memory, or the Dcache/WB
 * drained before the buffer is given to the DMA system.
*/

extern int imapx200_dma_enqueue(unsigned int channel, void *id,
			       dma_addr_t data, int size);

/* imapx200_dma_devconfig
 *
 * configure the device we're talking to
*/

extern int imapx200_dma_devconfig(int channel, enum imapx200_dmafc source,
				 unsigned long devaddr);

/* imapx200_dma_getposition
 *
 * get the position that the dma transfer is currently at
*/

extern int imapx200_dma_getposition(unsigned int channel,
				   dma_addr_t *src, dma_addr_t *dest);


/* imapx200_dma_request
 *
 * request a dma channel exclusivley
*/

extern int imapx200_dma_request(unsigned int channel,
			       struct imapx200_dma_client *, void *dev);

/* imapx200_dma_free
 *
 * free the dma channel (will also abort any outstanding operations)
*/

extern int imapx200_dma_free(unsigned int channel, struct imapx200_dma_client *);

int imapx200_dma_set_buffdone_fn(unsigned int channel, imapx200_dma_cbfn_t rtn);



