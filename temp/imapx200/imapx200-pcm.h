#ifndef _IMAPX200_PCM_H
#define _IMAPX200_PCM_H

#define ST_RUNNING		(1<<0)
#define ST_OPENED		(1<<1)
#include <linux/dmaengine.h>
struct imapx200_pcm_dma_params {
	struct imapx200_dma_client *client;	/* stream identifier */
	int channel;				/* Channel ID */
	dma_addr_t dma_addr;
        struct	dma_chan *chan;
	int dma_size;			/* Size of the DMA transfer */
};

//#define S3C24XX_DAI_I2S			0

/* platform data */
extern struct snd_soc_platform imapx200_soc_platform;
extern struct snd_ac97_bus_ops imapx200_ac97_ops;

#endif
