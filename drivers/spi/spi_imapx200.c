/* linux/drivers/spi/spi_s3c24xx.c
 *
 * Copyright (c) 2006 Ben Dooks
 * Copyright (c) 2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>

#include <asm/io.h>
#include <asm/dma.h>
#include <mach/hardware.h>

#include <mach/spi.h>

struct s3c24xx_spi {
	/* bitbang has to be first */
	struct spi_bitbang	 bitbang;
	struct completion	 done;

	void __iomem		*regs;
	int			 irq;
	int			 len;
	int			 count;

	void			(*set_cs)(struct s3c2410_spi_info *spi,
					  int cs, int pol);

	/* data buffers */
	const unsigned char	*tx;
	unsigned char		*rx;

	struct clk		*clk;
	struct resource		*ioarea;
	struct spi_master	*master;
	struct spi_device	*curdev;
	struct device		*dev;
	struct s3c2410_spi_info *pdata;
};

static inline struct s3c24xx_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static void s3c24xx_spi_gpiocs(struct s3c2410_spi_info *spi, int cs, int pol)
{
	int tmp = 0;
	printk(KERN_DEBUG"[drivers/spi/spi_imapx200.c]:s3c24xx_spi_gpiocs\n");
	//gpio_set_value(spi->pin_cs, pol);

	if (pol)
	{
		printk(KERN_DEBUG"~cs: low!\n");
		tmp = readl(rGPEDAT);
		tmp &= ~(1 <<7);
		writel(tmp , rGPEDAT);
	}
	else
	{
		printk(KERN_DEBUG"~cs: high!\n");
		tmp = readl(rGPEDAT);
		tmp |= (1 << 7);
		writel(tmp , rGPEDAT);
	}
}

static void s3c24xx_spi_chipsel(struct spi_device *spi, int value)
{
	struct s3c24xx_spi *hw = to_hw(spi);
	//unsigned int cspol = spi->mode & SPI_CS_HIGH ? 1 : 0;
	unsigned int cspol = spi->mode & SPI_CS_HIGH ? 0 : 1;
	//unsigned int spcon;

	printk(KERN_DEBUG"[drivers/spi/spi_imapx200.c]:s3c24xx_spi_chipsel\n");
	printk(KERN_DEBUG"value:%s,cspol:%d \n",(value ? "BITBANG_CS_ACTIVE" : "BITBANG_CS_INACTIVE") ,cspol);
	switch (value) {
	case BITBANG_CS_INACTIVE:
		hw->set_cs(hw->pdata, spi->chip_select, cspol^1);
		break;

	case BITBANG_CS_ACTIVE:
//		spcon = readb(hw->regs + S3C2410_SPCON);
//
//		if (spi->mode & SPI_CPHA)
//			spcon |= S3C2410_SPCON_CPHA_FMTB;
//		else
//			spcon &= ~S3C2410_SPCON_CPHA_FMTB;
//
//		if (spi->mode & SPI_CPOL)
//			spcon |= S3C2410_SPCON_CPOL_HIGH;
//		else
//			spcon &= ~S3C2410_SPCON_CPOL_HIGH;
//
//		spcon |= S3C2410_SPCON_ENSCK;
//
//		/* write new configration */
//
//		writeb(spcon, hw->regs + S3C2410_SPCON);
		hw->set_cs(hw->pdata, spi->chip_select, cspol);

		break;
	}
}

static int s3c24xx_spi_setupxfer(struct spi_device *spi,
				 struct spi_transfer *t)
{
	struct s3c24xx_spi *hw = to_hw(spi);
	unsigned int bpw;
	unsigned int hz;
	unsigned int div;
	unsigned long clk;
	
	printk(KERN_DEBUG"[drivers/spi/spi_imapx200.c]:s3c24xx_spi_setupxfer\n");

	bpw = t ? t->bits_per_word : spi->bits_per_word;
	hz  = t ? t->speed_hz : spi->max_speed_hz;

	if (!bpw)
		bpw = 8;

	if (!hz)
		hz = spi->max_speed_hz;

	if (bpw != 8) {
		dev_err(&spi->dev, "invalid bits-per-word (%d)\n", bpw);
		return -EINVAL;
	}

	clk = clk_get_rate(hw->clk);
	div = DIV_ROUND_UP(clk, hz * 2) - 1;

	if (div > 255)
		div = 255;

	dev_dbg(&spi->dev, "setting pre-scaler to %d (wanted %d, got %ld)\n",
		div, hz, clk / (2 * (div + 1)));

	//writel(div, hw->regs + rSSI_BAUDR_M);	
	//writeb(div, hw->regs + S3C2410_SPPRE);
	
	spin_lock(&hw->bitbang.lock);
	if (!hw->bitbang.busy) {
		hw->bitbang.chipselect(spi, BITBANG_CS_INACTIVE);
		/* need to ndelay for 0.5 clocktick ? */
	}
	spin_unlock(&hw->bitbang.lock);
	
	return 0;
}

static int s3c24xx_spi_setup(struct spi_device *spi)
{
	int ret;
	printk("[drivers/spi/spi_imapx200.c]:s3c24xx_spi_setup\n");
	ret = s3c24xx_spi_setupxfer(spi, NULL);
	if (ret < 0) {
		dev_err(&spi->dev, "setupxfer returned %d\n", ret);
		return ret;
	}

	return 0;
}

static inline unsigned int hw_txbyte(struct s3c24xx_spi *hw, int count)
{
	return hw->tx ? hw->tx[count] : 0;
}

static int s3c24xx_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct s3c24xx_spi *hw = to_hw(spi);
	unsigned int tmp = 0;
	printk(KERN_DEBUG"[drivers/spi/spi_imapx200.c]:s3c24xx_spi_txrx\n");
	
	//dev_info(&spi->dev, "txrx: tx %p, rx %p, len %d\n",\
			t->tx_buf, t->rx_buf, t->len);
	unsigned int spsta;
	unsigned int rxflr;

	hw->tx = t->tx_buf;
	hw->rx = t->rx_buf;
	hw->len = t->len;
	hw->count = 0;

	/*******************************************************************
	 * use interrupt mode START
	 */
	init_completion(&hw->done);
	writeb(hw_txbyte(hw, 0), hw->regs + rSSI_DR_M);
	wait_for_completion(&hw->done);
	return hw->count;

	/*
	 * use interrupt mode END
	 *******************************************************************/

	/*******************************************************************
	 * use query mode START
	 */

	/*
	writel(0, (hw->regs + rSSI_IMR_M));

	if (hw->tx)
	{
		// send the first byte
		printk(KERN_DEBUG"command:0x%x\n",hw_txbyte(hw, 0));
		writeb(hw_txbyte(hw, 0), hw->regs + rSSI_DR_M);
		
		rxflr = readb(hw->regs + rSSI_RXFLR_M);
		while (!rxflr)
		{
			rxflr = readb(hw->regs + rSSI_RXFLR_M);
		}
		spsta = readb(hw->regs + rSSI_SR_M);
		while (!(spsta & (1 << 2))) {
			dev_debug(hw->dev, "spi not ready for tx?\n");
			spsta = readb(hw->regs + rSSI_SR_M);
		}
		
		hw->count++;
		printk(KERN_DEBUG">>>>>>>>>>>>>>read invalid data!\n");
		tmp = readb(hw->regs + rSSI_DR_M);
	}
	if (hw->rx)
	{
		// send the second two byte & receive the valid data
		printk(KERN_DEBUG"command:0x%x\n",hw_txbyte(hw, 0));
		writeb(hw_txbyte(hw, 0), hw->regs + rSSI_DR_M);

		rxflr = readb(hw->regs + rSSI_RXFLR_M);
		while (!rxflr)
		{
			rxflr = readb(hw->regs + rSSI_RXFLR_M);
		}
		spsta = readb(hw->regs + rSSI_SR_M);
		while (!(spsta & (1 << 2))) {
			dev_debug(hw->dev, "spi not ready for tx?\n");
			spsta = readb(hw->regs + rSSI_SR_M);
		}
	
		hw->rx[hw->count] = readb(hw->regs + rSSI_DR_M);
		printk(KERN_DEBUG">>>>>>>>>>>>>>>>>>>>>>read valid data!%d....0x%x\n",
				hw->count,hw->rx[hw->count]);
		hw->count++;

		printk(KERN_DEBUG"command:0x%x\n",hw_txbyte(hw, hw->count));
		writeb(hw_txbyte(hw, hw->count), hw->regs + rSSI_DR_M);

		rxflr = readb(hw->regs + rSSI_RXFLR_M);
		while (!rxflr)
		{
			rxflr = readb(hw->regs + rSSI_RXFLR_M);
		}
		spsta = readb(hw->regs + rSSI_SR_M);
		while (!(spsta & (1 << 2))) {
			dev_debug(hw->dev, "spi not ready for tx?\n");
			spsta = readb(hw->regs + rSSI_SR_M);
		}

		hw->rx[hw->count] = readb(hw->regs + rSSI_DR_M);
		printk(KERN_DEBUG">>>>>>>>>>>>>>>>>>>>>>read valid data!%d....0x%x\n",
				hw->count,hw->rx[hw->count]);
		hw->count++;
	}
	return hw->count;
	*/

	/*
	 * use query mode END
	 * **********************************************************************/
}

static irqreturn_t s3c24xx_spi_irq(int irq, void *dev)
{
	struct s3c24xx_spi *hw = dev;
	//unsigned int spsta = readb(hw->regs + S3C2410_SPSTA);
	unsigned int spsta = readb(hw->regs + rSSI_SR_M);
	unsigned int count = hw->count;
	int tmp;

	printk(KERN_DEBUG"[drivers/spi/spi_imapx200.c]:s3c24xx_spi_irq\n");
	writel(0, (hw->regs + rSSI_IMR_M));

	//if (spsta & S3C2410_SPSTA_DCOL) {
	if (spsta & (1 << 6)) {
		dev_info(hw->dev, "data-collision\n");
		complete(&hw->done);
		goto irq_done;
	}

	if (!(spsta & (1 << 2))) {
		dev_info(hw->dev, "spi not ready for tx?\n");
		complete(&hw->done);
		goto irq_done;
	}

	hw->count++;

	if (hw->tx)
	{
		printk(KERN_DEBUG"read invalid data!\n");
		tmp = readb(hw->regs + rSSI_DR_M);
	}

	if (hw->rx)
	{
		//hw->rx[count] = readb(hw->regs + S3C2410_SPRDAT);
		hw->rx[count] = readb(hw->regs + rSSI_DR_M);
		printk(KERN_DEBUG"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~read valid data!0x%x\n",hw->rx[count]);
	}
	count++;

	if (count < hw->len)
		//writeb(hw_txbyte(hw, count), hw->regs + S3C2410_SPTDAT);
	{
		writeb(hw_txbyte(hw, count), hw->regs + rSSI_DR_M);
		printk(KERN_DEBUG"in irq,command:0x%x\n",hw_txbyte(hw, count));
	}
	else
		complete(&hw->done);

 irq_done:
	writel((1 << 4), (hw->regs + rSSI_IMR_M));
	return IRQ_HANDLED;
}

static void s3c24xx_spi_initialsetup(struct s3c24xx_spi *hw)
{
	/* for the moment, permanently enable the clock */
	printk("[drivers/spi/spi_imapx200.c]:imapx200_spi_initialsetup!\r\n");
	clk_enable(hw->clk);

	/* program defaults into the registers */
	writel(0, (hw->regs + rSSI_ENR_M));
	writel((1 << 4), (hw->regs + rSSI_IMR_M));
	writel(30, (hw->regs + rSSI_BAUDR_M));
	writel(8, (hw->regs + rSSI_TXFTLR_M));
	writel(0, (hw->regs + rSSI_RXFTLR_M));
	writel(((0x0 << 8) | (0x0 << 7) | (0x0 << 6) | (0x0 << 4) | (0x7 << 0)), (hw->regs + rSSI_CTLR0_M));
	writel(1, (hw->regs + rSSI_SER_M));
	writel(1, (hw->regs + rSSI_ENR_M));

//	writeb(0xff, hw->regs + S3C2410_SPPRE);
//	writeb(SPPIN_DEFAULT, hw->regs + S3C2410_SPPIN);
//	writeb(SPCON_DEFAULT, hw->regs + S3C2410_SPCON);

	if (hw->pdata) {
		if (hw->set_cs == s3c24xx_spi_gpiocs)
			gpio_direction_output(hw->pdata->pin_cs, 1);

		if (hw->pdata->gpio_setup)
			hw->pdata->gpio_setup(hw->pdata, 1);
	}
}

static int __init s3c24xx_spi_probe(struct platform_device *pdev)
{
	struct s3c2410_spi_info *pdata;
	struct s3c24xx_spi *hw;
	struct spi_master *master;
	struct resource *res;
	int err = 0;
	int gpecon;

	printk("[drivers/spi/spi_imapx200.c]:imapx200_spi_probe!\r\n");
	master = spi_alloc_master(&pdev->dev, sizeof(struct s3c24xx_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct s3c24xx_spi));

	hw->master = spi_master_get(master);
	hw->pdata = pdata = pdev->dev.platform_data;
	hw->dev = &pdev->dev;

	if (pdata == NULL) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		err = -ENOENT;
		goto err_no_pdata;
	}

	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	/* setup the master state. */

	/* the spi->mode bits understood by this driver: */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;

	master->num_chipselect = hw->pdata->num_cs;
	master->bus_num = pdata->bus_num;

	/* setup the state for the bitbang driver */

	hw->bitbang.master         = hw->master;
	hw->bitbang.setup_transfer = s3c24xx_spi_setupxfer;
	hw->bitbang.chipselect     = s3c24xx_spi_chipsel;
	hw->bitbang.txrx_bufs      = s3c24xx_spi_txrx;
	hw->bitbang.master->setup  = s3c24xx_spi_setup;

	dev_info(hw->dev, "bitbang at %p\n", &hw->bitbang);
	/* find and map our resources */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	hw->ioarea = request_mem_region(res->start, (res->end - res->start)+1,
					pdev->name);

	if (hw->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	hw->regs = ioremap(res->start, (res->end - res->start)+1);
	if (hw->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_no_iomap;
	}

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	err = request_irq(hw->irq, s3c24xx_spi_irq, IRQF_DISABLED, pdev->name, hw);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_irq;
	}

	hw->clk = clk_get(&pdev->dev, "spi");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_no_clk;
	}

	/* setup any gpio we can */

	if (!pdata->set_cs) {
		if (pdata->pin_cs < 0) {
			dev_err(&pdev->dev, "No chipselect pin\n");
			goto err_register;
		}

//		err = gpio_request(pdata->pin_cs, dev_name(&pdev->dev));
//		if (err) {
//			dev_err(&pdev->dev, "Failed to get gpio for cs\n");
//			goto err_register;
//		}

		hw->set_cs = s3c24xx_spi_gpiocs;
		gpio_direction_output(pdata->pin_cs, 1);
	} else
		hw->set_cs = pdata->set_cs;

	gpecon = readl(rGPECON);
	gpecon &= ~((0x3 << 14) | (0x3 << 12) | (0x3 << 10) | (0x3 << 8));
	gpecon |= ((0x1 <<14) | (0x2 << 12) | (0x2 << 10) | (0x2 << 8));
	writel(gpecon,rGPECON);

	s3c24xx_spi_initialsetup(hw);

	/* register our spi controller */

	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

	return 0;

 err_register:
	if (hw->set_cs == s3c24xx_spi_gpiocs)
		gpio_free(pdata->pin_cs);

	clk_disable(hw->clk);
	clk_put(hw->clk);

 err_no_clk:
	free_irq(hw->irq, hw);

 err_no_irq:
	iounmap(hw->regs);

 err_no_iomap:
	release_resource(hw->ioarea);
	kfree(hw->ioarea);

 err_no_iores:
 err_no_pdata:
	spi_master_put(hw->master);;

 err_nomem:
	return err;
}

static int __exit s3c24xx_spi_remove(struct platform_device *dev)
{
	struct s3c24xx_spi *hw = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

	clk_disable(hw->clk);
	clk_put(hw->clk);

	free_irq(hw->irq, hw);
	iounmap(hw->regs);

	if (hw->set_cs == s3c24xx_spi_gpiocs)
		gpio_free(hw->pdata->pin_cs);

	release_resource(hw->ioarea);
	kfree(hw->ioarea);

	spi_master_put(hw->master);
	return 0;
}


#ifdef CONFIG_PM

static int s3c24xx_spi_suspend(struct platform_device *pdev, pm_message_t msg)
{
	struct s3c24xx_spi *hw = platform_get_drvdata(pdev);

	if (hw->pdata && hw->pdata->gpio_setup)
		hw->pdata->gpio_setup(hw->pdata, 0);

	clk_disable(hw->clk);
	return 0;
}

static int s3c24xx_spi_resume(struct platform_device *pdev)
{
	struct s3c24xx_spi *hw = platform_get_drvdata(pdev);

	s3c24xx_spi_initialsetup(hw);
	return 0;
}

#else
#define s3c24xx_spi_suspend NULL
#define s3c24xx_spi_resume  NULL
#endif

MODULE_ALIAS("platform:imapx200-spi");
static struct platform_driver s3c24xx_spi_driver = {
	.remove		= __exit_p(s3c24xx_spi_remove),
	.suspend	= s3c24xx_spi_suspend,
	.resume		= s3c24xx_spi_resume,
	.driver		= {
		.name	= "imapx200_ssim0",
		.owner	= THIS_MODULE,
	},
};

static int __init s3c24xx_spi_init(void)
{
	printk("[drivers/spi/spi_imapx200.c]:imapx200_spi_init!\r\n");
        return platform_driver_probe(&s3c24xx_spi_driver, s3c24xx_spi_probe);
}

static void __exit s3c24xx_spi_exit(void)
{
        platform_driver_unregister(&s3c24xx_spi_driver);
}

module_init(s3c24xx_spi_init);
module_exit(s3c24xx_spi_exit);

MODULE_DESCRIPTION("Imapx200 SPI Driver");
MODULE_AUTHOR("John Zhao, <John.zhao@infotmic.com.cn>");
MODULE_LICENSE("GPL");