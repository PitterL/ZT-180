/* linux/drivers/mmc/host/sdhci-imap.c
 *
 * Copyright 2008 Openmoko Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * SDHCI (HSMMC) support for infoTM 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <linux/mmc/host.h>

#include <plat/sdhci.h>
#include <plat/regs-sdhci.h>

#include "sdhci.h"

#define MAX_BUS_CLK	(2)

#if defined(CONFIG_SDI2_USE_EPLL_CLOCK)
    #define CLOCK_SRC 1      //in devices order
    #define CLOCK_DIVIDER 4  //max is 7
#else
    #define CLOCK_SRC 0
#endif

#ifdef CONFIG_ZT_ENCRYPT
extern int rt_jia_sd;
#endif

struct sdhci_imap {
	struct sdhci_host	*host;
	struct platform_device	*pdev;
	struct resource		*ioarea;
	struct imapx_sdi_platdata  *pdata;
	//unsigned int		cur_clk;

	struct clk		*clk_io;
	struct clk		*clk_bus[MAX_BUS_CLK];
};

static inline u32 sdhci_readl_wrapper(struct sdhci_host *host, int reg)
{
    u32 val;
    val=(u32)readl(host->ioaddr + reg);

    if(host->mmc->index==0){
        //skip
    }else if((host->mmc->index==1)||(host->mmc->index==2)){
        if(reg==SDHCI_PRESENT_STATE){
		    val|=SDHCI_CARD_PRESENT;
		    val|=SDHCI_WRITE_PROTECT;
		}    
    }
    
    return val;
}

static inline struct sdhci_imap *to_imap(struct sdhci_host *host)
{
	return sdhci_priv(host);
}

/*
static void imapx_cfg_card(struct platform_device *dev,
				  struct sdhci_host *host)
{
	u32 ctrl2;
	//struct sdhci_imap *ourhost = to_imap(host); 
	void __iomem *r = host->ioaddr;
	unsigned int clk_src;

	ctrl2 = readl(r + IMAP_SDHCI_CONTROL2);

	ctrl2 &= ~IMAP_SDHCI_CTRL2_SELBASECLK_MASK;

	ctrl2 |= (IMAP_SDHCI_CTRL2_ENSTAASYNCCLR |
		  IMAP_SDHCI_CTRL2_ENCMDCNFMSK |
		  IMAP_SDHCI_CTRL2_ENFBCLKRX |
		  IMAP_SDHCI_CTRL2_DFCNT_NONE |
		  IMAP_SDHCI_CTRL2_ENCLKOUTHOLD);

	writel(ctrl2, r + IMAP_SDHCI_CONTROL2);

	clk_src = readl(r + IMAP_SDHCI_CONTROL2);
}*/

				
/**
 * sdhci_imap_get_max_clk - callback to get maximum clock frequency.
 * @host: The SDHCI host instance.
 *
 * Callback to return the maximum clock rate acheivable by the controller.
*/
static unsigned int sdhci_imap_get_max_clk(struct sdhci_host *host)
{
	struct sdhci_imap *ourhost = to_imap(host);
	struct clk *busclk;
	unsigned int rate;

	busclk = ourhost->clk_io;
	rate = clk_get_rate(busclk);

    #if defined(CONFIG_SDI2_USE_EPLL_CLOCK)
        rate /= (CLOCK_DIVIDER+1);  //epll is too large,divide it at init
	#endif


	return rate;
}

static unsigned int sdhci_imap_get_timeout_clk(struct sdhci_host *host)
{
	return sdhci_imap_get_max_clk(host) / 1000000;
}

static void sdhci_imap_set_clk_src(struct sdhci_host *host)
{
	unsigned int ctrl2;
    
	ctrl2 = readl(host->ioaddr + IMAP_SDHCI_CONTROL2);
	
  #if defined(CONFIG_SDI2_USE_EPLL_CLOCK)
    ctrl2 |= IMAP_SDHCI_CTRL2_SELBASECLK_MASK;
  #else
    ctrl2 &= ~IMAP_SDHCI_CTRL2_SELBASECLK_MASK;
  #endif

	writel(ctrl2,host->ioaddr + IMAP_SDHCI_CONTROL2);
}

static struct sdhci_ops sdhci_imap_ops = {
    .sdhci_readl        = sdhci_readl_wrapper,
	.get_max_clock		= sdhci_imap_get_max_clk,
	.get_timeout_clock	= sdhci_imap_get_timeout_clk,
	.set_clk_src		= sdhci_imap_set_clk_src,
};

static int __devinit sdhci_imap_probe(struct platform_device *pdev)
{
	struct imapx_sdi_platdata *pdata = pdev->dev.platform_data;
	struct device *dev = &pdev->dev;
	struct sdhci_host *host;
	struct sdhci_imap *sc;
	struct resource *res;
	int ret, irq;
    uint32_t val;

	printk(KERN_INFO "[SDHCI2]:infoTM sdhci host driver init.\n");
    #ifdef CONFIG_ZT_ENCRYPT	
        if( 99 != rt_jia_sd)
            return 0;
    #endif
	if (!pdata) {
		dev_err(dev, "no device data specified\n");
		return -ENOENT;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "no irq specified\n");
		return irq;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "no memory specified\n");
		return -ENOENT;
	}

	host = sdhci_alloc_host(dev, sizeof(struct sdhci_imap));
	if (IS_ERR(host)) {
		dev_err(dev, "sdhci_alloc_host() failed\n");
		return PTR_ERR(host);
	}

	sc = sdhci_priv(host);

	sc->host = host;
	sc->pdev = pdev;
	sc->pdata = pdata;

	platform_set_drvdata(pdev, host);

    //config clock
    local_irq_disable();
    val = readl(rDIV_CFG3);
    val &= ~(0xff << 16);
  #if defined(CONFIG_SDI2_USE_EPLL_CLOCK)
    val |= (CLOCK_DIVIDER<<18)|(0x2<<16);
  #else
    val |= (0x3<<16);
  #endif
    writel(val, rDIV_CFG3);
    local_irq_enable();
    
	sc->clk_io = clk_get(dev,pdata->clocks[CLOCK_SRC]);
	if (IS_ERR(sc->clk_io)) {
		dev_err(dev, "failed to get io clock\n");
		ret = PTR_ERR(sc->clk_io);
		goto err_io_clk;
	}

	clk_enable(sc->clk_io);
	sc->ioarea = request_mem_region(res->start, resource_size(res),
					mmc_hostname(host->mmc));
	if (!sc->ioarea) {
		dev_err(dev, "failed to reserve register area\n");
		ret = -ENXIO;
		goto err_no_busclks;
	}

	host->ioaddr = ioremap_nocache(res->start, resource_size(res));
	if (!host->ioaddr) {
		dev_err(dev, "failed to map registers\n");
		ret = -ENXIO;
		goto err_no_busclks;
	}

	//imapx_set_gpio(pdata->hw_port,pdata->width);
	//imapx_cfg_card(pdev,host);

	host->hw_name = "info_sdi2";
	host->ops = &sdhci_imap_ops;
	host->quirks = 0;
	host->irq = irq;

	/* Setup quirks for the controller */

	/* Currently with ADMA enabled we are getting some length
	 * interrupts that are not being dealt with, do disable
	 * ADMA until this is sorted out. */
#ifndef	CONFIG_MMC_SDHCI_IMAP_ADMA
	host->quirks |= SDHCI_QUIRK_BROKEN_ADMA;
	host->quirks |= SDHCI_QUIRK_32BIT_ADMA_SIZE;
#endif

#ifndef CONFIG_MMC_SDHCI_IMAP_SDMA

	/* we currently see overruns on errors, so disable the SDMA
	 * support as well. */
	host->quirks |= SDHCI_QUIRK_BROKEN_DMA;

	/* PIO currently has problems with multi-block IO */
	host->quirks |= SDHCI_QUIRK_NO_MULTIBLOCK;

#endif /* CONFIG_MMC_SDHCI_IMAP_SDMA */

	/* It seems we do not get an DATA transfer complete on non-busy
	 * transfers, not sure if this is a problem with this specific
	 * SDHCI block, or a missing configuration that needs to be set. */
	host->quirks |= SDHCI_QUIRK_NO_BUSY_IRQ;

	host->quirks |= (SDHCI_QUIRK_32BIT_DMA_ADDR |
			 SDHCI_QUIRK_32BIT_DMA_SIZE);

    //temp force here
	//host->quirks |= SDHCI_QUIRK_FORCE_1_BIT_DATA;  //here is some problem at 4 bit wifi

	ret = sdhci_add_host(host);
	if (ret) {
		dev_err(dev, "sdhci_add_host() failed\n");
		goto err_add_host;
	}

	return 0;

 err_add_host:
	release_resource(sc->ioarea);
	kfree(sc->ioarea);
  
 err_no_busclks:
	clk_disable(sc->clk_io);
	clk_put(sc->clk_io);

 err_io_clk:
	sdhci_free_host(host);


	return ret;
}

static int __devexit sdhci_imap_remove(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_PM

static int sdhci_imap_suspend(struct platform_device *dev, pm_message_t pm)
{
	struct sdhci_host *host = platform_get_drvdata(dev);

	sdhci_suspend_host(host, pm);
	return 0;
}

static int sdhci_imap_resume(struct platform_device *dev)
{
	struct sdhci_host *host = platform_get_drvdata(dev);

	sdhci_resume_host(host);
	return 0;
}

#else
#define sdhci_imap_suspend NULL
#define sdhci_imap_resume NULL
#endif

static struct platform_driver sdhci_imap_driver = {
	.probe		= sdhci_imap_probe,
	.remove		= __devexit_p(sdhci_imap_remove),
	.suspend	= sdhci_imap_suspend,
	.resume	        = sdhci_imap_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "imapx200_sdi2",
	},
};

static int __init sdhci_imap_init(void)
{
	return platform_driver_register(&sdhci_imap_driver);
}

static void __exit sdhci_imap_exit(void)
{
	platform_driver_unregister(&sdhci_imap_driver);
}

module_init(sdhci_imap_init);
module_exit(sdhci_imap_exit);

MODULE_AUTHOR("Ben Dooks, <ben@simtec.co.uk>");
MODULE_LICENSE("GPL v2");
