/*
 * drivers/input/touchscreen/tsc2007.c
 *
 * Copyright (c) 2008 MtekVision Co., Ltd.
 *	Kwangwoo Lee <kwlee@mtekvision.com>
 *
 * Using code from:
 *  - ads7846.c
 *	Copyright (c) 2005 David Brownell
 *	Copyright (c) 2006 Nokia Corporation
 *  - corgi_ts.c
 *	Copyright (C) 2004-2005 Richard Purdie
 *  - omap_ts.[hc], ads7846.h, ts_osk.c
 *	Copyright (C) 2002 MontaVista Software
 *	Copyright (C) 2004 Texas Instruments
 *	Copyright (C) 2005 Dirk Behme
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 
 2010-7-13 ㄩwifi power control by yqcui
 */

#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/i2c/tsc2007.h>
#include <mach/imapx_gpio.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/major.h>
#include <asm/uaccess.h>


static DECLARE_MUTEX(nas_mutil_mutex);

#define ON_TOUCH_INT /*IRQ_EINT1*/IRQ_EINT4

struct nastech_ts_setup_data {
	int i2c_bus;	
	unsigned short i2c_address;
};
static struct nastech_ts_setup_data nastech_ts_setup = {
	.i2c_address = 0x5c,
	.i2c_bus = 0x2,
};

static int nas_mutil_write(struct nas_ts_priv *clinet, u8 reg, u8 value)
{
	u8 data[2];
	int ret;
	down(&nas_mutil_mutex);
	data[0] = reg;
	data[1] = value;
	
	ret = i2c_master_send(clinet, data, 2);
	if (ret < 0){
		printk("tsc2007_write is error!\r\n");	
	}
	
	up(&nas_mutil_mutex);
	return ret;
}

static inline int nas_mutil_xfer(struct nas_ts_priv *clinet, u8 cmd)
{
	u8 buf[1];
	int ret;
	u8 data = cmd;
	
	//down(&nas_mutil_mutex);
	/* Set address */
	ret = i2c_master_send(clinet, &data, 1);
	if (ret < 0) 
		goto ret_error;
	ret = i2c_master_recv(clinet, &buf, 1);
	if (ret < 0)
		goto ret_error;
		
	ret = buf[0];
	//printk("nas_mutil_xfer : buf[0] = 0x%x\r\n",buf[0]);
	
ret_error:
	//up(&nas_mutil_mutex);
	return ret;
}

// config interrupt for GPO2
static void config_gpio()
{
	unsigned long rEINTCON_value, rEINTFLTCON0_value, rGPFCON_value, rGPFDAT_value, rGPGDAT_value;

    /*
	rEINTCON_value = __raw_readl(rEINTCON);
	rEINTCON_value |= 0x02<<4;
	__raw_writel(rEINTCON_value, rEINTCON);           //EINTCON

	rEINTFLTCON0_value = __raw_readl(rEINTFLTCON0);
	rEINTFLTCON0_value |= 0xff<<8;
	__raw_writel(rEINTFLTCON0_value, rEINTFLTCON0);        //EINTFILTER
    */
    rEINTCON_value = __raw_readl(rEINTCON);
    rEINTCON_value &= ~(0xF<<16);
    rEINTCON_value |= 0x02<<16;
    __raw_writel(rEINTCON_value, rEINTCON);           //EINTCON
    
    rEINTFLTCON0_value = __raw_readl(rEINTFLTCON1);
    rEINTFLTCON0_value |= 0xff;
    __raw_writel(rEINTFLTCON0_value, rEINTFLTCON1);        //EINTFILTER


    if (gpio_request(IMAPX200_GPE(10), "nas_touch")) {
        printk("motor is error\r\n");
        return ;
    }
    // reset
    gpio_direction_output(IMAPX200_GPE(10),1);
}


//#define MY_DEBUG	1


#define	EVENT_PENUP	3
#define	EVENT_PENDOWN	1

static int nastech_ts_open(struct input_dev *dev);
static void nastech_ts_close(struct input_dev *dev);
static irqreturn_t nastech_ts_isr(int irq, void *dev_id);

static struct workqueue_struct *nastech_wq;


struct nas_ts_priv {
	struct i2c_client *client;
	struct input_dev *input;
	//struct input_dev *input0;
	//struct delayed_work work;
	struct hrtimer timer;
	struct work_struct  nas_work;
	int reported_finger_count;
	int irq;
};
static void nastech_ts_work(struct work_struct *work)
{
	unsigned short xpos, ypos;
	unsigned short xpos0, ypos0;
	unsigned char event;
	unsigned char Finger; // Number of fingers touching
	unsigned char ret;
	
	struct nas_ts_priv *nas_priv = container_of(work,struct nas_ts_priv,nas_work);


	ret=nas_mutil_xfer(nas_priv->client, 0x0);
	if(ret<0)
	{
		printk("nastech_ts_work: i2c_transfer Error !\n");
		goto out;
	}
	Finger=(unsigned char)ret;
	
	/* x */
	ret = nas_mutil_xfer(nas_priv->client, 0x03);
	if(ret<0)
	{
		printk("nastech_ts_work: i2c_transfer Error !\n");
		goto out;
	}	
	xpos = ( unsigned short )( ret << 8 );
	

	ret = nas_mutil_xfer(nas_priv->client, 0x02);
	if(ret<0)
	{
		printk("nastech_ts_work: i2c_transfer Error !\n");
		goto out;
	}
	xpos = xpos | ret;

	/* y */
	ret = nas_mutil_xfer(nas_priv->client, 0x05);
	if(ret<0)
	{
		printk("nastech_ts_work: i2c_transfer Error !\n");
		goto out;
	}	
	ypos=( unsigned short )( ret << 8 );
	
	ret = nas_mutil_xfer(nas_priv->client, 0x04);
	if(ret<0)
	{
		printk("nastech_ts_work: i2c_transfer Error !\n");
		goto out;
	}	
	ypos = ypos | ret;
	ypos=480-ypos;

	//--------------------------------------------------------------------
	
	if(Finger>1)
	{
		/* x */
		ret = nas_mutil_xfer(nas_priv->client, 0x07);
		if(ret<0)
		{
			printk("nastech_ts_work: i2c_transfer Error !\n");
			goto out;
		}		
		xpos0 = ( unsigned short )( ret << 8 );
		
	
		ret = nas_mutil_xfer(nas_priv->client, 0x06);
		if(ret<0)
		{
			printk("nastech_ts_work: i2c_transfer Error !\n");
			goto out;
		}
		xpos0 = xpos0 | ret;
		
	
		/* y */
		ret = nas_mutil_xfer(nas_priv->client, 0x09);
		if(ret<0)
		{
			printk("nastech_ts_work: i2c_transfer Error !\n");
			goto out;
		}
		ypos0 = ( unsigned short )( ret << 8 );
		
		ret = nas_mutil_xfer(nas_priv->client, 0x08);
		if(ret<0)
		{
			printk("nastech_ts_work: i2c_transfer Error !\n");
			goto out;
		}
		ypos0 = ypos0 | ret;
		ypos0=480-ypos0;
	}
	#if MY_DEBUG
		printk("		nastech_ts_work: Finger : %d\n",Finger);
		printk("		nastech_ts_work: X = 0x%x , Y = 0x%x\n",xpos,ypos);
		printk("		nastech_ts_work: X = %d , Y = %d\n",xpos,ypos);	
		printk("		nastech_ts_work: X0 = 0x%x , Y0 = 0x%x\n",xpos0,ypos0);
		printk("		nastech_ts_work: X0 = %d , Y0 = %d\n",xpos0,ypos0);
    #endif
	
	if(!Finger)//(xpos==0xFFFF || ypos==0xFFFF)
	{
		input_report_key(nas_priv->input, BTN_TOUCH, 0);
	}
	else
	{
		input_report_key(nas_priv->input, BTN_TOUCH, 1);
		input_report_abs(nas_priv->input, ABS_X, xpos);
		input_report_abs(nas_priv->input, ABS_Y, ypos);
	}
	
	if(Finger>1)//(xpos0==0xFFFF || ypos0==0xFFFF)
	{
		input_report_key(nas_priv->input, BTN_2, 0);
	}
	else
	{
		input_report_key(nas_priv->input, BTN_2, 1);
		input_report_abs(nas_priv->input, ABS_HAT0X, xpos0);
		input_report_abs(nas_priv->input, ABS_HAT0Y, ypos0);
	}
	{
		int z,w;
		if(!Finger)
		{
			z=0;
			w=0;
		}
		else
		{
			z=255;
			w=15;
		}
		input_report_abs(nas_priv->input, ABS_MT_TOUCH_MAJOR, z);
		input_report_abs(nas_priv->input, ABS_MT_WIDTH_MAJOR, w);
		input_report_abs(nas_priv->input, ABS_MT_POSITION_X, xpos);
		input_report_abs(nas_priv->input, ABS_MT_POSITION_Y, ypos);
		input_mt_sync(nas_priv->input);
		if(Finger>1)
		{
			input_report_abs(nas_priv->input, ABS_MT_TOUCH_MAJOR, z);
			input_report_abs(nas_priv->input, ABS_MT_WIDTH_MAJOR, w);
			input_report_abs(nas_priv->input, ABS_MT_POSITION_X, xpos0);
			input_report_abs(nas_priv->input, ABS_MT_POSITION_Y, ypos0);
			input_mt_sync(nas_priv->input);
		}
		else if(nas_priv->reported_finger_count > 1)
		{
			input_report_abs(nas_priv->input, ABS_MT_TOUCH_MAJOR, 0);
			input_report_abs(nas_priv->input, ABS_MT_WIDTH_MAJOR, 0);
			input_mt_sync(nas_priv->input);
		}
		nas_priv->reported_finger_count = Finger;
		
	}
	input_sync(nas_priv->input);
	
out:
	//enable_irq(ON_TOUCH_INT);
	return;
}
static int nastech_ts_probe(struct i2c_client *client,const struct i2c_device_id *idp)
{
	struct nas_ts_priv *nas_priv;
	struct input_dev *nas_input;
	int error;

	printk("nastech_ts_probe\r\n");
	
	nas_priv = kzalloc(sizeof(*nas_priv), GFP_KERNEL);
	if (!nas_priv)
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: kzalloc Error!\n");
		#endif
		error=-ENODEV;
		goto	err0;
	}
	else
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: kzalloc OK!\n");
		#endif
	}
	dev_set_drvdata(&client->dev, nas_priv);

	nas_priv->client = client;
	i2c_set_clientdata(client, nas_priv);

	
	nas_input = input_allocate_device();
	if (!nas_input)
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: input_allocate_device Error\n");
		#endif
		error=-ENODEV;
		goto	err1_1;
		//return -ENODEV;
	}
	else
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: input_allocate_device OK\n");
		#endif
	}

	nas_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN) ;
	nas_input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) | BIT_MASK(BTN_2);
	//-------------------------------------------------------------------------------------------
		input_set_abs_params(nas_input, ABS_X, 0,800, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_Y, 0, 480, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_HAT0X, 0,800, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_HAT0Y, 0, 480, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_MT_POSITION_X, 0,800, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_MT_POSITION_Y, 0, 480, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_MT_TOUCH_MAJOR, 0,255, 0, 0);
		input_set_abs_params(nas_input, ABS_MT_WIDTH_MAJOR, 0,15, 0, 0);
	//-------------------------------------------------------------------------------------------
	
	nas_input->name = client->name;
	nas_input->id.bustype = BUS_I2C;
	nas_input->dev.parent = &client->dev;
	//nas_input->open = nastech_ts_open;
	//nas_input->close = nastech_ts_close;
	input_set_drvdata(nas_input, nas_priv);

	nas_priv->client = client;
	nas_priv->input = nas_input;
	//nas_priv->input0 = nas_input0;

	//INIT_DELAYED_WORK(&priv->work, migor_ts_poscheck);
	INIT_WORK(&nas_priv->nas_work, nastech_ts_work);

	//nas_priv->irq = client->irq;
	nas_priv->irq=ON_TOUCH_INT;
	#if MY_DEBUG
		//printk("		nastech_ts_probe: irq : %d\n",client->irq);
		printk("		nastech_ts_probe: nas_priv->irq : %d\n",nas_priv->irq);
	#endif
	
	error = input_register_device(nas_input);
	if(error)
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: input_register_device input Error!\n");
		#endif
		error=-ENODEV;
		goto	err1;
	}
	else
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: input_register_device input OK!\n");
		#endif
	}


	
	error = nas_mutil_write(nas_priv->client,0x37,0x03);
	printk("nas_mutil_write error = %d\r\n",error);
	msleep(1100);
	error = nas_mutil_write(nas_priv->client,0x15,0x09);
	printk("nas_mutil_write error = %d\r\n",error);	
	
	config_gpio();
	
	error = request_irq(/*IRQ_EINT1*/nas_priv->irq, nastech_ts_isr,IRQF_DISABLED|IRQF_TRIGGER_FALLING, client->name, nas_priv);
	//error = request_irq(IRQ_EINT1, nastech_ts_isr,IRQF_DISABLED, client->name, nas_priv);
	
	if (error)
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: request_irq Error!\n");
		#endif
		error=-ENODEV;
		goto err2;
	}
	else
	{
		#if MY_DEBUG
			printk("		nastech_ts_probe: request_irq OK!\n");
		#endif
	}

	return 0;
err2:
	input_unregister_device(nas_input);
	//nas_input = NULL; /* so we dont try to free it below */
err1:
	//input_free_device(nas_input0);
err1_1:
	input_free_device(nas_input);
	kfree(nas_priv);

err0:
	dev_set_drvdata(&client->dev, NULL);
	return error;
}
static int nastech_ts_remove(struct i2c_client *client)
{
	struct nas_ts_priv *nas_priv = dev_get_drvdata(&client->dev);

	#if MY_DEBUG
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_remove !               |\n");
		printk("+-----------------------------------------+\n");
	#endif

	nas_priv->irq=ON_TOUCH_INT;
	free_irq(nas_priv->irq, nas_priv);//釋放中斷
	input_unregister_device(nas_priv->input);
	kfree(nas_priv);

	dev_set_drvdata(&client->dev, NULL);

	return 0;
}
static int nastech_ts_open(struct input_dev *dev)
{
	#if MY_DEBUG
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_open!                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	return 0;
}
static void nastech_ts_close(struct input_dev *dev)
{
	#if MY_DEBUG
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_close!                 |\n");
		printk("+-----------------------------------------+\n");
	#endif
}
static irqreturn_t nastech_ts_isr(int irq, void *dev_id)
{
	struct nas_ts_priv *nas_priv = dev_id;
	int ret;

	//disable_irq_nosync(ON_TOUCH_INT);//關避中斷

	ret=queue_work(nastech_wq, &nas_priv->nas_work);
  #if MY_DEBUG	
	if(ret){
	    printk("nastech_ts_isr: queue_work non-zero otherwise!\n");
	}else{
	    printk("nastech_ts_isr: queue_work work was already on a queue!\n");
	}
  #endif	
	//enable_irq(ON_TOUCH_INT);
	return IRQ_HANDLED;
}
static enum hrtimer_restart nastech_ts_timer(struct hrtimer *timer)
{
	struct nas_ts_priv *nas_priv = container_of(timer, struct nas_ts_priv, timer);
	
	printk("nastech_ts_timer!\n");
	queue_work(nastech_wq, &nas_priv->nas_work);
	hrtimer_start(&nas_priv->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}
static const struct i2c_device_id nastech_ts_id[] = {
	{ "nastech-ts", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, nastech_ts_id);

static struct i2c_driver nastech_ts_driver = {
	.driver = {
		.name = "nastech-ts",
	},
	.probe = nastech_ts_probe,
	.remove = nastech_ts_remove,
	.id_table = nastech_ts_id,
};

static char banner[] __initdata = KERN_INFO "nastech Touchscreen driver, (c) 2010 nas Technologies Corp. \n";
static int __init nastech_ts_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int ret;
	
	printk(banner);
	
	nastech_wq = create_singlethread_workqueue("nastech_wq");
	if (!nastech_wq)
	{
		printk("nastech_ts_init: create_singlethread_workqueue Error!\n");
		return -ENOMEM;
	}
	else
	{
		printk("nastech_ts_init: create_singlethread_workqueue OK!\n");
	}
	
	ret=i2c_add_driver(&nastech_ts_driver);//註冊 i2c_Driver
	if(ret)
	{
		printk("nastech_ts_init: i2c_add_driver Error! \n");
	}
	else
	{
		printk("nastech_ts_init: i2c_add_driver OK! \n");
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = nastech_ts_setup.i2c_address;
	strlcpy(info.type, "nastech-ts", I2C_NAME_SIZE);

	adapter = i2c_get_adapter(nastech_ts_setup.i2c_bus);
	if (!adapter) {
		printk("nastech_ts_init : can't get i2c adapter %d\n",
			nastech_ts_setup.i2c_bus);
		goto err_driver;
	}
	client = i2c_new_device(adapter, &info);

	i2c_put_adapter(adapter);
	if (!client) {
		printk("nastech_ts_init : can't add i2c device at 0x%x\n",
			(unsigned int)info.addr);
		goto err_driver;
	}

	printk("...... success\r\n");
	return 0;

err_driver:
	i2c_del_driver(&nastech_ts_driver);
	return -ENODEV;
}

static void __exit nastech_ts_exit(void)
{
	printk("nastech_ts_exit!\n");

	i2c_del_driver(&nastech_ts_driver);//註銷 i2c_Driver
	if (nastech_wq)
		destroy_workqueue(nastech_wq);
}

module_init(nastech_ts_init);
module_exit(nastech_ts_exit);

MODULE_AUTHOR("nastech Ming");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("nastech Touchscreen Driver");

