/***************************************************************************** 
 * sensor.c 
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: main file of simple media sensor driver
 *
 * Author:
 *     Sololz <sololz@infotm.com>
 *      
 * Revision History: 
 * 颅颅颅颅颅颅颅颅颅颅颅颅颅颅颅颅颅 
 * 1.1  12/9/2009 Sololz 
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/poll.h>  
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/types.h>    
#include <linux/interrupt.h>
#include <linux/init.h>      
#include <linux/string.h>
#include <linux/mm.h>             
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <asm/delay.h>
#include <mach/imapx_gpio.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/time.h>

/*
 * XXX: sensor hard ware directly connect to system bus, there is 
 * no use to clock set
 */ 
#include <plat/clock.h>

#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>

#include "common_sensor.h"

//#define TEST_SENSOR 1

#define DEV_NAME "sensor"


/*
 * functions delare
 */
//static int reset_hw_reg_sensor(void);		/* this function just set all resigster to be 0 */
static int sensor_driver_register(void);	/* register driver as an char device */
static int sensor_driver_unregister(void);

/*
 * this structure include global varaibles
 */
sensor_param_t	sensor_param;		/* global variables group */
//static struct	clk *imap_vdec_clock;	/* sensor hardware clock */

//static wait_queue_head_t wait_sensor;	/* a wait queue for poll system call */
static struct completion sensor_comp;
struct sensor_orientation {
	int x;
	/*int y;
	int z;*/
	//struct semaphore sem;
	struct delayed_work work;
};

#define ROTATION_0 1
#define ROTATION_90 2
#define ROTATION_180 3
#define ROTATION_270 4

//static int sensor_state_ori = 0;
//static int sensor_state = 0;

static struct sensor_orientation sensor_dev; 

//blow must be same as framework
//"acceleration"
//"magnetic-field"
#define SENSER_NAME_ORI "orientation"
//"temperature"


/* return the current time in nanoseconds */
static int64_t time_to_ns(void)
{
    struct timeval tv;

    do_gettimeofday(&tv);
    
    return timeval_to_ns(&tv);
}


static unsigned long sensor_poll_time=MAX_SCHEDULE_TIMEOUT;

static void set_poll_time(unsigned long timeout)
{
    sensor_poll_time=timeout;
}
static unsigned long get_poll_time(void)
{
    return sensor_poll_time;
}

static spinlock_t irq_lock;

void sensor_interrupt_disable(void)
{
    unsigned long  value,flags;

    spin_lock_irqsave(&irq_lock,flags);
    
	value = __raw_readl(rEINTG6MASK);
	value |= (0x1<<27);
	value |= (0x1<<28);
	__raw_writel(value, rEINTG6MASK);
        
    spin_unlock_irqrestore(&irq_lock,flags);
}

void sensor_interrupt_enable(void)
{
    unsigned long  value,flags;

    spin_lock_irqsave(&irq_lock,flags);
    
	value = __raw_readl(rEINTG6MASK);
	value &= ~(0x1<<27);
	value &= ~(0x1<<28);
	__raw_writel(value, rEINTG6MASK);
        
    spin_unlock_irqrestore(&irq_lock,flags);
}

int sensor_interrupt_clear(void)
{
    unsigned long  flags;
    int value;

    spin_lock_irqsave(&irq_lock,flags);
    
	value = __raw_readl(rEINTG6PEND);
	if(value&((0x1<<28)|(0x1<<27))){
	    __raw_writel(((0x1<<28)|(0x1<<27)), rEINTG6PEND);
    }
    spin_unlock_irqrestore(&irq_lock,flags);

    //printk("%s value=0x%x\n",__func__,value);
    return value&((0x1<<27)|(0x1<<28));
}


/*
 * open system call just mark file private data as a sensor 
 * instance by default, and you can change it by a ioctl call
 */
static int simple_sensor_open(struct inode *inode, struct file *file)
{	
	/* dec instance by default, you can change it by ioctl pp instance */
	file->private_data = &(sensor_param.dec_instance);

    //schedule_delayed_work(&(sensor_dev.work),HZ/100);
	return 0;
}

/*
 * fasync system call be called here
 */
static int simple_sensor_release(struct inode *inode, struct file *file)
{    
	cancel_work_sync(&(sensor_dev.work.work));
	return 0;
}

static int simple_sensor_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{

	char data[255];

	wait_for_completion_timeout(&sensor_comp,get_poll_time());	
		
	sprintf(data,"%s:%d:%d:%d sync:%lld",
	    SENSER_NAME_ORI,sensor_dev.x,/*sensor_dev.y,sensor_dev.z*/0,0,
	    //0,0,0,
	    time_to_ns());
	    
	count=strlen(data);
	
	//sensor_debug("read data: %s count=%d\n",data,count);
	
	if (copy_to_user(buf, data, count)) {
		return -EFAULT;
	}
	return count;
}

static int simple_sensor_write(struct file *filp,const char __user *buf, size_t count, loff_t *f_pos)
{
    char    command[128];    
    int     params;

    size_t len;

    len=min(sizeof(command)-1,count);
    if(copy_from_user(command,buf,len)){
        return -EFAULT;
    }

    /* read the next event */
    sensor_debug("count=%d len=%d\n",count,len);

    command[len] = 0;

    /* "set-delay:%d" corresponds to control__activate() */
    if (sscanf(command, "set:" SENSER_NAME_ORI ":%d",&params) == 1) {
        sensor_debug("%s enable=%d\n",__func__,params);
        if(params == 1)
            sensor_interrupt_enable();
        else if(params == 0)
            sensor_interrupt_disable();
        else
            sensor_debug("%s unknow cmd para params=%d\n",__func__,params);
            
    /* "set-delay:%d" corresponds to control__set_delay() */        
    }else if (sscanf(command, "set-delay:%d", &params) == 1) {
        sensor_debug("%s set delay %d\n",__func__,params);
        set_poll_time((unsigned long)params);
        complete(&sensor_comp);
    /* "set-delay:%d" corresponds to control__wake() */
    }else if (!strcmp((const char*)command, "wake")) {
        sensor_debug("%s set wake\n",__func__);
        complete(&sensor_comp);
    }else{
        sensor_debug("%s unknow command %s\n",__func__,command);
        len=0;
    }

    return len;
}

static int is_open_gsensor = 0;
static int simple_sensor_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	//printk("%s cmd = %d, arg = %d \n",__func__,cmd,arg);
	switch(cmd)
	{
		case 0 :
			is_open_gsensor	= arg;
			break;
			
		case 1:
			arg = is_open_gsensor;		
			break;
		default :
			is_open_gsensor = 0;
	}
	return 0;	
}

static struct file_operations simple_sensor_fops = 
{
	owner:		THIS_MODULE,
	open:		simple_sensor_open,
	read:		simple_sensor_read,
	write:		simple_sensor_write,
	release:	simple_sensor_release,
	ioctl: 		simple_sensor_ioctl,
};


static void sensor_event(struct work_struct *work){
	
	volatile unsigned int g,g1, g2;
	
	g = __raw_readl(rGPLDAT);
	g1 = (g >> 0) & 1;
	g2 = (g >> 1) & 1;
	//printk("GPIDAT is %x, g1 = %d , g2 = %d\r\n", g,g1,g2);

	if( ( 0 == g1 ) && ( 0 == g2 ) ){
        sensor_dev.x=270;

		//sensor_state = ROTATION_270;
		/*
		sensor_dev.x=0; 
		sensor_dev.y=270;
		sensor_dev.z=0;*/		
	}
	else if( ( 1 == g1 ) && ( 0 == g2 ) ){
        sensor_dev.x=0;
		//sensor_state = ROTATION_0;
		/*sensor_dev.x=0; 
		sensor_dev.y=0;
		sensor_dev.z=0;	*/	
	}	
	else if( ( 1 == g1 ) && ( 1 == g2 ) ){
        sensor_dev.x=90;
		//sensor_state = ROTATION_90;
		/*sensor_dev.x=0; 
		sensor_dev.y=90;
		sensor_dev.z=0;	*/	
	}
	else if( ( 0 == g1 ) && ( 1 == g2 ) ){
        sensor_dev.x=180;
		//sensor_state = ROTATION_180;
		/*sensor_dev.x=0; 
		sensor_dev.y=180;
		sensor_dev.z=0;		*/
	}
	

	//if( ( sensor_state != 0 ) && ( sensor_state != sensor_state_ori ) ){
		//printk("direction is %d\r\n",sensor_state);
	//    sensor_state_ori = sensor_state;
		
		complete(&sensor_comp);	
	//}
}
static irqreturn_t sensor_keys_orientation(int irq, void *dev_id)
{
    /*
	volatile unsigned int tmp;
	
	tmp = __raw_readl(rEINTG6PEND);
	//printk("sensor_keys_orientation , tmp = 0x%x\r\n",tmp);
	if(  0 == ( tmp & 0x18000000) )
		return IRQ_NONE;
	
	// clear interrupt
	tmp = 0xffffffff;
	__raw_writel(tmp, rEINTG6PEND);

	schedule_delayed_work(&(sensor_dev.work),HZ/100);

	return IRQ_HANDLED;*/


    irqreturn_t ret=IRQ_NONE;

    if(sensor_interrupt_clear()){
        schedule_delayed_work(&sensor_dev.work,HZ/100);

        ret=IRQ_HANDLED;
    }

	return ret;
}



/*
 * platform operation relate functions
 */
static int simple_sensor_probe(struct platform_device *pdev)
{
	int ret, error =0;
	 unsigned long  rEINTGCON_value, rEINTGFLTCON0_value;

	/* initualize wait queue */
	//init_waitqueue_head(&wait_sensor);
	init_completion(&sensor_comp);
	INIT_DELAYED_WORK(&(sensor_dev.work), sensor_event);
	spin_lock_init(&irq_lock);
	ret = 0;

	/*config GPI1 ->eint group6[5] and GPI13 ->eint group6[13] as input gpio*/

	if (gpio_request(IMAPX200_GPL(0), "sensor g1"))
		return error;

	if (gpio_request(IMAPX200_GPL(1), "sensor g2"))
		return error;
		
	gpio_direction_input(IMAPX200_GPL(0));
	gpio_direction_input(IMAPX200_GPL(1));


	imapx200_gpio_setpull(IMAPX200_GPL(0), 0);
	imapx200_gpio_setpull(IMAPX200_GPL(1), 0);

	/*setting the method of the eint group 6*/
	rEINTGCON_value = __raw_readl(rEINTGCON);
	rEINTGCON_value |= (0x7<<20);
	__raw_writel(rEINTGCON_value, rEINTGCON);
	/*setting the external group filter control register*/
	rEINTGFLTCON0_value = __raw_readl(rEINTGFLTCON1);
	rEINTGFLTCON0_value |= (0xff<<8);
	__raw_writel(rEINTGFLTCON0_value, rEINTGFLTCON1);	

	sensor_interrupt_disable();
	sensor_interrupt_clear();

	error = request_irq(IRQ_GPIO, sensor_keys_orientation,IRQF_SHARED,"sensor_keys_orientation",pdev);
	if (error) {
		pr_err("gpio-keys: Unable to claim irq %d; error %d\n",
			IRQ_GPIO, error);
		return error;
	}

	if(ret)
	{
		sensor_error("Fail to request irq for sensor device\n");
		return ret;
	}
	/* register char device driver */
	/*
	ret = sensor_driver_register();
	if(ret)
	{
		sensor_error("Fail to register char device for sensro device\n");
		return ret;
	}*/
	return 0;
}

static int simple_sensor_remove(struct platform_device *pdev)
{
	/* release irq */
	sensor_interrupt_disable();
	sensor_interrupt_clear();
	free_irq(IRQ_GPIO, pdev);
	//sensor_driver_unregister();
	return 0;
}

#ifdef CONFIG_PM
static int simple_sensor_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int simple_sensor_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

static struct platform_driver simple_sensor_driver = 
{
	.probe		= simple_sensor_probe,
	.remove		= simple_sensor_remove,
#ifdef CONFIG_PM
	.suspend	= simple_sensor_suspend,
	.resume		= simple_sensor_resume,
#endif
	.driver		=
	{
		.owner		= THIS_MODULE,
		.name		= "sensor_acc",
	},
};

/*
 * init and exit
 */
static int __init simple_sensor_init(void)
{
	/* call probe */
	//enum sensor_state sensor_state_ori;
	if(platform_driver_register(&simple_sensor_driver))
	{
		sensor_error("Fail to register platform driver for sensor driver\n");
		return -EPERM;
	}

	/* register char device driver */
	if(sensor_driver_register()){
		sensor_error("Fail to register char device for sensor device\n");
		return -1;
	}
	//init_MUTEX(&(sensor_dev.sem));
	
	return 0;
}

static void __exit simple_sensor_exit(void)
{
	/* call remove */
	platform_driver_unregister(&simple_sensor_driver);
	sensor_driver_unregister();
}

module_init(simple_sensor_init);
module_exit(simple_sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZT");
MODULE_DESCRIPTION("simple sensor driver");


/*
 * this function do driver register to regist a node under /dev
 */
static struct class *sensor_class;

int sensor_driver_register(void)
{
	if(register_chrdev(SENSOR_DEFAULT_MAJOR, "simple-sensor", &simple_sensor_fops)){
		sensor_error("register char deivce error\n");
		return -1;
	}

	sensor_class = class_create(THIS_MODULE, "simple-sensor");
	device_create(sensor_class, NULL, MKDEV(SENSOR_DEFAULT_MAJOR, SENSOR_DEFAULT_MINOR), NULL, DEV_NAME);

	return 0;
}

int sensor_driver_unregister(void)
{
	device_destroy(sensor_class, MKDEV(SENSOR_DEFAULT_MAJOR, SENSOR_DEFAULT_MINOR));
	class_destroy(sensor_class);
	unregister_chrdev(SENSOR_DEFAULT_MAJOR, "simple-sensor");

	return 0;
}


