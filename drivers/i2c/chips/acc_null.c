/*
 * Copyright (C) 2010 MEMSIC, Inc.
 *
 * Initial Code:
 *	Robbie Cao
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */


//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/uaccess.h>

#include "acc_null.h"

#define DEBUG							0

#define MAX_FAILURE_COUNT				3

#define ACC_NULL_DELAY_PWRON				300							/* ms, >= 300 ms */
#define ACC_NULL_DELAY_PWRDN				1							/* ms */
#define ACC_NULL_DELAY_SETDETECTION		ACC_NULL_DELAY_PWRON

#define ACC_NULL_RETRY_COUNT				3

#define GPIO_SENSOR_AXIS0   IMAPX200_GPL(0)
#define GPIO_SENSOR_AXIS1   IMAPX200_GPL(1)


static int reg_read_axis(int * data)
{
    int axis;
    
    axis = gpio_get_value(GPIO_SENSOR_AXIS0);
    axis |= gpio_get_value(GPIO_SENSOR_AXIS1)<<1;

    switch(axis){
        case 0x0:
            data[0]=1;
            data[1]=0;
        break;
        case 0x1:
            data[0]=0;
            data[1]=1;
        break;
        case 0x3:
            data[0]=-1;
            data[1]=0;
        break;
        case 0x2:
            data[0]=0;
            data[1]=-1;
        break;
        default:
            printk("reg_read_axis failed axis=0x%x\n",axis);
            data[0]=data[1]=0;
    }

    return 0;    
}

static int acc_null_open(struct inode *inode, struct file *file)
{
	printk("acc_null open\n");


	return nonseekable_open(inode, file);
}

static int acc_null_release(struct inode *inode, struct file *file)
{

	printk("acc_null release\n");

	return 0;
}

static int acc_null_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	void __user *pa = (void __user *)arg;
	char data[0];
	int vec[3] = {0};


    static int sensor_sts=SENSOR_UNKNOW;  //when power on, reset z value to 0,up level will reset cache 

	switch (cmd) 
	{
		case ACC_NULL_IOC_PWRON:
			printk("acc_null power on\n");

            if(sensor_sts == SENSOR_UNKNOW){
                //init reg if first
            }
            sensor_sts = SENSOR_OPENED;
            
            // sensor start
            
			msleep(ACC_NULL_DELAY_PWRON);
			break;

		case ACC_NULL_IOC_PWRDN:

			printk("acc_null power down\n");

            sensor_sts = SENSOR_CLOSED;

            //sensor stop

/* wait PWRDN done */
			msleep(ACC_NULL_DELAY_PWRDN);
			break;

		case ACC_NULL_IOC_READXYZ:
            if(reg_read_axis(vec)<0) {
                return -EFAULT;
            }

            vec[0]*=16; //current count/g = 1, mutiply 4 to make pass filter
            vec[1]*=16;

            vec[2] = 5;  //make z more than 1/3

            if(sensor_sts == SENSOR_OPENED){
                vec[2]=0;
                sensor_sts = SENSOR_WORKING;
            }
            
#if DEBUG
            printk("[X: %08x] [Y: %08x] [Z: %08x],[X: %d] [Y: %d] [Z: %d]\n", 
                vec[0], vec[1], vec[2],vec[0], vec[1], vec[2]);
#endif
            if (copy_to_user(pa, vec, sizeof(vec))) {
                return -EFAULT;
            }
            break;


		case ACC_NULL_IOC_READSTATUS:
			printk("acc_null read status\n");

            if(reg_read_axis(vec)<0) {
                return -EFAULT;
            }
		
			vec[2] = 0;  //status
#if DEBUG
			printk("[X - %04x] [Y - %04x] [STATUS - %04x]\n", 
				vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) {
				return -EFAULT;
			}
			break;

		case ACC_NULL_IOC_SETDETECTION:
			printk("acc_null set detection\n");

			break;
        case ACC_NULL_IOC_GET_DIR:
            //printk("acc_null get dir\n");

          #if defined(CONFIG_BOARD_B0)
            data[0] = 0;
          #endif    

			if (copy_to_user(pa, data, 1)) {
				return -EFAULT;
			}
            break;

		default:
			break;
	}

	return 0;
}

static ssize_t acc_null_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "ACC_NULL");
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(acc_null, S_IRUGO, acc_null_show, NULL);

static struct file_operations acc_null_fops = {
	.owner		= THIS_MODULE,
	.open		= acc_null_open,
	.release	= acc_null_release,
	.ioctl		= acc_null_ioctl,
};

static struct miscdevice acc_null_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = ACC_NULL_NAME,
	.fops = &acc_null_fops,
};

int sensor_gpio_init(void)
{
#if defined(CONFIG_BOARD_B0)

    if (gpio_request(GPIO_SENSOR_AXIS0, "sensor axis 0"))
        return -1;
    
    if (gpio_request(GPIO_SENSOR_AXIS1, "sensor axis 1"))
        return -1;
        
    gpio_direction_input(GPIO_SENSOR_AXIS0);
    gpio_direction_input(GPIO_SENSOR_AXIS1);

    return 0;
#else
    return -1;
#endif

}

int acc_null_probe(struct platform_device *pdev)
{
	int res = 0;

	printk("acc_null_probe start\n");

    res = sensor_gpio_init();
	if (res) {
		pr_err("%s: acc_null_device register gpio failed\n", __FUNCTION__);
		goto out;
	}

	res = misc_register(&acc_null_device);
	if (res) {
		pr_err("%s: acc_null_device register misc failed\n", __FUNCTION__);
		goto out;
	}

	res = device_create_file(acc_null_device.this_device, &dev_attr_acc_null);
	if (res) {
		pr_err("%s: device_create_file failed\n", __FUNCTION__);
		goto out_deregister;
	}

	printk("acc_null_probe start ok\n");
	return 0;

out_deregister:
	misc_deregister(&acc_null_device);
out:
	return res;
}

static int acc_null_remove(struct platform_device *pdev)
{
	device_remove_file(acc_null_device.this_device, &dev_attr_acc_null);
	misc_deregister(&acc_null_device);

	return 0;
}


static struct platform_driver acc_null_driver = {
	.probe 		= acc_null_probe,
	.remove 	= acc_null_remove,
	.driver 	= {
		.owner	= THIS_MODULE,
		.name = ACC_NULL_NAME,
	},
};


static int __init acc_null_init(void)
{
	printk("acc_null driver: init\n");
	/* call probe */
	if(platform_driver_register(&acc_null_driver)) {
		pr_err("Fail to register platform driver for sensor driver\n");
		return -EPERM;
	}
	
	return 0;

}

static void __exit acc_null_exit(void)
{
	printk("acc_null driver: exit\n");
	/* call remove */
	platform_driver_unregister(&acc_null_driver);
}

module_init(acc_null_init);
module_exit(acc_null_exit);

MODULE_AUTHOR("atp <dyliao@zenithink.com>");
MODULE_DESCRIPTION("ZENITHINK ACC_NULL (DTOS) simple Sensor Driver");
MODULE_LICENSE("GPL");

