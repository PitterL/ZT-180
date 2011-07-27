/*
 * linux/drivers/input/keyboard/pxa27x_keypad.c
 *
 * Driver for the pxa27x matrix keyboard controller.
 *
 * Created:	Feb 22, 2007
 * Author:	Rodolfo Giometti <giometti@linux.it>
 *
 * Based on a previous implementations by Kevin O'Connor
 * <kevin_at_koconnor.net> and Alex Osborne <bobofdoom@gmail.com> and
 * on some suggestions by Nicolas Pitre <nico@cam.org>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/imapx200_keybd.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <asm/imapx200_keybd.h>
 
#define keypad_readl(off)	__raw_readl(keypad->mmio_base + (off))
#define keypad_writel(off, v)	__raw_writel((v), keypad->mmio_base + (off))

volatile int flag_NUML = 0;

struct imapx200_keybd{
	struct imapx200_keybd_platform_data *pdata;

	struct input_dev *input_dev;
	struct clk *clk;

	void __iomem *mmio_base;

	int irq;

	unsigned int matrix_keycodes[MAX_MATRIX_KEY_NUM];
	unsigned int matrix_key_state[MAX_MATRIX_KEY_ROWS];

	int suspend;
};


static void imapx200_keybd_build_keycode(struct imapx200_keybd *keypad)
{
	struct imapx200_keybd_platform_data *pdata = keypad->pdata;
	struct input_dev *input_dev = keypad->input_dev;
	unsigned int *key;
	int i;

	key = &pdata->matrix_key_map[0];
	for (i = 0; i < pdata->matrix_key_map_size; i++, key++) {
		int row = ((*key) >> 24) & 0xff;
		int col = ((*key) >> 20) & 0xf;
		int code = (*key) & 0xfffff;

		keypad->matrix_keycodes[(row << 3) + col] = code;
//		printk("scancode...row:%x,col:%x....keycode:%x\r\n",row,col,code);
		set_bit(code, input_dev->keybit);
	}
}

static inline unsigned int lookup_matrix_keycode(
		struct imapx200_keybd *keypad, int row, int col)
{
	unsigned int ret = keypad->matrix_keycodes[(row << 3) + col];
	//printk("lookup_matrix_keycode ... row:%x,col:%x....keycode:%x\r\n",row,col,ret);
	return ret;
}

static void imapx200_keybd_scan_matrix(struct imapx200_keybd *keypad)
{
	struct imapx200_keybd_platform_data *pdata = keypad->pdata;
	int iRow, iCol;
	uint32_t kbRowData[MAX_MATRIX_KEY_ROWS];
	volatile int key;
	memset(kbRowData, 0, sizeof(kbRowData));
	kbRowData[0] = keypad_readl(rKBROWD0) & 0xff;
	kbRowData[1] = (keypad_readl(rKBROWD0) & (0xff<<8))>>8;
        kbRowData[2] = (keypad_readl(rKBROWD0) & (0xff<<16))>>16;
        kbRowData[3] = (keypad_readl(rKBROWD0) & (0xff<<24))>>24;
        kbRowData[4] = keypad_readl(rKBROWD1) & 0xff;
        kbRowData[5] = (keypad_readl(rKBROWD1) & (0xff<<8))>>8;
        kbRowData[6] = (keypad_readl(rKBROWD1) & (0xff<<16))>>16;
        kbRowData[7] = (keypad_readl(rKBROWD1) & (0xff<<24))>>24;
        kbRowData[8] = keypad_readl(rKBROWD2) & 0xff;
        kbRowData[9] = (keypad_readl(rKBROWD2) & (0xff<<8))>>8;
        kbRowData[10] = (keypad_readl(rKBROWD2) & (0xff<<16))>>16;
        kbRowData[11] = (keypad_readl(rKBROWD2) & (0xff<<24))>>24;
        kbRowData[12] = keypad_readl(rKBROWD3) & 0xff;
        kbRowData[13] = (keypad_readl(rKBROWD3) & (0xff<<8))>>8;
        kbRowData[14] = (keypad_readl(rKBROWD3) & (0xff<<16))>>16;
        kbRowData[15] = (keypad_readl(rKBROWD3) & (0xff<<24))>>24;
        kbRowData[16] = keypad_readl(rKBROWD4) & 0xff;
        kbRowData[17] = (keypad_readl(rKBROWD4) & (0xff<<8))>>8;

	for (iRow = 0; iRow < pdata->matrix_key_rows; iRow++) 
	{
		uint32_t bits_changed;

		bits_changed = keypad->matrix_key_state[iRow] ^ kbRowData[iRow];
		if (bits_changed == 0)
			continue;

		for (iCol = 0; iCol < pdata->matrix_key_cols; iCol++)
		{
			if ((bits_changed & (1 << iCol)) == 0)
				continue;
			
			key =lookup_matrix_keycode(keypad, iRow, iCol);
			if (key == KEY_NUMLOCK)
			{
				flag_NUML++;
				if (flag_NUML == 4)
					flag_NUML = 0;
//				printk("flag_NUML:%d\r\n",flag_NUML);
			}
			if (flag_NUML == 3)
			{
//				printk("NumLock is set!\r\n");
				switch ( key )
				{
#ifdef CONFIG_MATRIXKEY_FOR_PRODUCT
					case KEY_7:
						input_report_key(keypad->input_dev,KEY_KP7,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_8:
						input_report_key(keypad->input_dev,KEY_KP8,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_9:
						input_report_key(keypad->input_dev,KEY_KP9,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_0:
						input_report_key(keypad->input_dev,KEY_KPSLASH,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_U:
						input_report_key(keypad->input_dev,KEY_KP4,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_I:
						input_report_key(keypad->input_dev,KEY_KP5,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_O:
						input_report_key(keypad->input_dev,KEY_KP6,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_P:
						input_report_key(keypad->input_dev,KEY_KPASTERISK,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_J:
						input_report_key(keypad->input_dev,KEY_KP1,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_K:
						input_report_key(keypad->input_dev,KEY_KP2,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_L:
						input_report_key(keypad->input_dev,KEY_KP3,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_SEMICOLON:
						input_report_key(keypad->input_dev,KEY_KPMINUS,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_M:
						input_report_key(keypad->input_dev,KEY_KP0,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_DOT:
						input_report_key(keypad->input_dev,KEY_KPDOT,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_SLASH:
						input_report_key(keypad->input_dev,KEY_KPPLUS,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					default:
						input_report_key(keypad->input_dev,
							lookup_matrix_keycode(keypad, iRow, iCol),
							!(kbRowData[iRow] & (1 << iCol)));
						break;

#else
					case KEY_7:
						printk("KEY_7 is down!\r\n");
						input_report_key(keypad->input_dev,KEY_KP7,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_8:
						input_report_key(keypad->input_dev,KEY_KP8,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_9:
						input_report_key(keypad->input_dev,KEY_KP9,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_0:
						input_report_key(keypad->input_dev,KEY_KPASTERISK,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_U:
						input_report_key(keypad->input_dev,KEY_KP4,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_I:
						input_report_key(keypad->input_dev,KEY_KP5,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_O:
						input_report_key(keypad->input_dev,KEY_KP6,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_P:
						input_report_key(keypad->input_dev,KEY_KPMINUS,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_J:
						input_report_key(keypad->input_dev,KEY_KP1,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_K:
						input_report_key(keypad->input_dev,KEY_KP2,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_L:
						input_report_key(keypad->input_dev,KEY_KP3,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_SEMICOLON:
						input_report_key(keypad->input_dev,KEY_KPPLUS,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_M:
						input_report_key(keypad->input_dev,KEY_KP0,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_DOT:
						input_report_key(keypad->input_dev,KEY_KPDOT,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					case KEY_SLASH:
						input_report_key(keypad->input_dev,KEY_KPSLASH,
							!(kbRowData[iRow] & (1 << iCol)));
						break;
					default:
						input_report_key(keypad->input_dev,
							lookup_matrix_keycode(keypad, iRow, iCol),
							!(kbRowData[iRow] & (1 << iCol)));
						break;
#endif
				}
			}
//			printk("input_report_key: value is 0x%x , status is %x!\r\n",
//					lookup_matrix_keycode(keypad, iRow, iCol),
//					!(kbRowData[iRow] & (1 << iCol)));
			else
				input_report_key(keypad->input_dev,
					lookup_matrix_keycode(keypad, iRow, iCol),
					!(kbRowData[iRow] & (1 << iCol)));
		}
	}
	input_sync(keypad->input_dev);
	memcpy(keypad->matrix_key_state, kbRowData, sizeof(kbRowData));
}


static irqreturn_t imapx200_keybd_irq_handler(int irq, void *dev_id)
{
	struct imapx200_keybd *keypad = dev_id;

	//clear keybd interrupt
	keypad_writel(rKBINT , 0x1ffff);

	while(1)
	{
		if(keypad_readl(rKBINT) == 0)
		{
			break;
		}
		else
		{
			 keypad_writel(rKBINT , 0x1ffff);
		}
	}

	imapx200_keybd_scan_matrix(keypad);

//	//clear keybd interrupt
//	keypad_writel(rKBINT , KBDCNT_DRDYINT);	
	return IRQ_HANDLED;
}

static int imapx200_keybd_open(struct input_dev *dev)
{
	struct imapx200_keybd *keypad = input_get_drvdata(dev);
	
	/* Enable unit clock */
	clk_enable(keypad->clk);

	/* enable matrix keys with automatic scan */
	keypad_writel(rKBCKD , 1024);
	keypad_writel(rKBDCNT , 100);
	keypad_writel(rKBCOLD , 0);

	keypad_writel(rKBRPTC , 1024*16);	
	keypad_writel(rKBCON , (KBCON_RPTEN |KBCON_FCEN| KBCON_DFEN | KBCON_DRDYINTEN));
	keypad_writel(rKBCOEN , (KBCOEN_COLNUM | KBCOEN_COLOEN));

        //clear keybd interrupt
        keypad_writel(rKBINT , 0x1ffff);
         while(1)
         {
                 if(keypad_readl(rKBINT) == 0)
                 {
                         break;
                 }
                 else
                 {
                          keypad_writel(rKBINT , 0x1ffff);
                 }
         }
	
	return 0;
}

static void imapx200_keybd_close(struct input_dev *dev)
{
	struct imapx200_keybd *keypad = input_get_drvdata(dev);
	
	/* Disable clock unit */
	clk_disable(keypad->clk);
}

//#ifdef CONFIG_PM
#if 0
static int imapx200_keybd_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct imapx200_keybd *keypad = platform_get_drvdata(pdev);

	clk_disable(keypad->clk);

	if (device_may_wakeup(&pdev->dev))
		enable_irq_wake(keypad->irq);

	return 0;
}

static int imapx200_keybd_resume(struct platform_device *pdev)
{
	struct imapx200_keybd *keypad = platform_get_drvdata(pdev);
	struct input_dev *input_dev = keypad->input_dev;

	if (device_may_wakeup(&pdev->dev))
		disable_irq_wake(keypad->irq);

	mutex_lock(&input_dev->mutex);

	if (input_dev->users) {
		/* Enable unit clock */
		clk_enable(keypad->clk);
		pxa27x_keypad_config(keypad);
	}

	mutex_unlock(&input_dev->mutex);

	return 0;
}
#else
#define imapx200_keybd_suspend	NULL
#define imapx200_keybd_resume	NULL
#endif

#define res_size(res)	((res)->end - (res)->start + 1)

static int __devinit imapx200_keybd_probe(struct platform_device *pdev)
{
	struct imapx200_keybd *keypad;
	struct input_dev *input_dev;
	struct resource *res;
	int irq, error;
	int gphcon, gpicon, gpjcon;

	printk("imapx200_keybd_probe \r\n");

	//config the GPIO port for keyboard 
	gphcon = readl(rGPHCON);
	gphcon |= 0xff;
	writel(gphcon , rGPHCON);

	gpicon = readl(rGPICON);
	gpicon |= 0xfffffff;
	writel(gpicon , rGPICON);

	gpjcon = readl(rGPJCON);
	gpjcon |= 0xffff;
	writel(gpjcon , rGPJCON);

	keypad = kzalloc(sizeof(struct imapx200_keybd), GFP_KERNEL);
	if (keypad == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	keypad->pdata = pdev->dev.platform_data;
	if (keypad->pdata == NULL) {
		dev_err(&pdev->dev, "no platform data defined\n");
		error = -EINVAL;
		goto failed_free;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get keypad irq\n");
		error = -ENXIO;
		goto failed_free;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get I/O memory\n");
		error = -ENXIO;
		goto failed_free;
	}

	res = request_mem_region(res->start, res_size(res), pdev->name);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request I/O memory\n");
		error = -EBUSY;
		goto failed_free;
	}

	keypad->mmio_base = ioremap(res->start, res_size(res));
	if (keypad->mmio_base == NULL) {
		dev_err(&pdev->dev, "failed to remap I/O memory\n");
		error = -ENXIO;
		goto failed_free_mem;
	}

        keypad->clk = clk_get(&pdev->dev, "kb");
	if (IS_ERR(keypad->clk)) {
		dev_err(&pdev->dev, "failed to get keypad clock\n");
		error = PTR_ERR(keypad->clk);
		goto failed_free_io;
	}

	/* Create and register the input driver. */
	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&pdev->dev, "failed to allocate input device\n");
		error = -ENOMEM;
		goto failed_put_clk;
	}

	input_dev->name = pdev->name;
	input_dev->id.bustype = BUS_HOST;
	input_dev->open = imapx200_keybd_open;
	input_dev->close = imapx200_keybd_close;
	input_dev->dev.parent = &pdev->dev;

	keypad->input_dev = input_dev;
	input_set_drvdata(input_dev, keypad);

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);

	imapx200_keybd_build_keycode(keypad);

	platform_set_drvdata(pdev, keypad);

	error = request_irq(irq, imapx200_keybd_irq_handler, IRQF_DISABLED,
			    pdev->name, keypad);
	if (error) {
		dev_err(&pdev->dev, "failed to request IRQ\n");
		goto failed_free_dev;
	}

	keypad->irq = irq;

	/* Register the input device */
	error = input_register_device(input_dev);
	if (error) {
		dev_err(&pdev->dev, "failed to register input device\n");
		goto failed_free_irq;
	}

	device_init_wakeup(&pdev->dev, 1);

	return 0;

failed_free_irq:
	free_irq(irq, pdev);
	platform_set_drvdata(pdev, NULL);
failed_free_dev:
	input_free_device(input_dev);
failed_put_clk:
	clk_put(keypad->clk);
failed_free_io:
	iounmap(keypad->mmio_base);
failed_free_mem:
	release_mem_region(res->start, res_size(res));
failed_free:
	kfree(keypad);
	return error;
}

static int __devexit imapx200_keybd_remove(struct platform_device *pdev)
{
	struct imapx200_keybd *keypad = platform_get_drvdata(pdev);
	struct resource *res;

	free_irq(keypad->irq, pdev);

	clk_disable(keypad->clk);
	clk_put(keypad->clk);

	input_unregister_device(keypad->input_dev);
	input_free_device(keypad->input_dev);

	iounmap(keypad->mmio_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, res_size(res));

	platform_set_drvdata(pdev, NULL);
	kfree(keypad);
	return 0;
}

/* work with hotplug and coldplug */
MODULE_ALIAS("platform:imapx200_keypad");

static struct platform_driver imapx200_keybd_driver = {
	.probe		= imapx200_keybd_probe,
	.remove		= __devexit_p(imapx200_keybd_remove),
	.suspend	= imapx200_keybd_suspend,
	.resume		= imapx200_keybd_resume,
	.driver		= {
		.name	= "imapx200_keybd",
		.owner	= THIS_MODULE,
	},
};

static int __init imapx200_keybd_init(void)
{
	return platform_driver_register(&imapx200_keybd_driver);
}

static void __exit imapx200_keybd_exit(void)
{
	platform_driver_unregister(&imapx200_keybd_driver);
}

module_init(imapx200_keybd_init);
module_exit(imapx200_keybd_exit);

MODULE_DESCRIPTION("imapx200 keybd Controller Driver");
MODULE_LICENSE("GPL");
