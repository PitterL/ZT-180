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
#include <asm/uaccess.h>

#include "mxc622x.h"

#define DEBUG							0
#define IIC_ADDRESS     MXC622X_I2C_ADDR
#define IIC_BUS         0x1

#define MAX_FAILURE_COUNT				3

#define MXC622X_DELAY_PWRON				300							/* ms, >= 300 ms */
#define MXC622X_DELAY_PWRDN				1							/* ms */
#define MXC622X_DELAY_SETDETECTION		MXC622X_DELAY_PWRON

#define MXC622X_RETRY_COUNT				3

static struct i2c_client *this_client;

static int mxc622x_i2c_tx_data(char *buf, int len)
{
	int ret;
	char buff[2];
	buff[0] = buf[0];
	buff[1] = buf[1];
	
	ret = i2c_master_send(this_client, buff, 2);
	if (ret < 0){
		printk("%s reg 0x%x data 0x%x failed\n",__func__,buf[0],buf[1]);
		goto error;
	}

	//printk("%s reg 0x%x : 0x%x\n",__func__,buf[0],buf[1]);
error:	
	return ret;
}


static int reg_read_n(u8 reg,u8 * data,int n)
{
	int ret;
	
	/* Set address */
	ret = i2c_master_send(this_client, &reg, 1);
	if (ret != 1){
		printk("%s reg 0x%x failed #1 ret %d\n",__func__,reg ,ret);
		ret = -1;
		goto error;
	}

	ret = i2c_master_recv(this_client, data, n);
	if (ret != n){
		printk("%s reg 0x%x failed #2 ret %d\n",__func__,reg ,ret);
		ret = -1;
		goto error;
	}

    //printk("%s reg 0x%x : 0x%x %d\n",__func__,reg,*data,ret);

error:
	return ret;
}


static int mxc622x_open(struct inode *inode, struct file *file)
{
	printk("mxc622x open\n");


	return nonseekable_open(inode, file);
}

static int mxc622x_release(struct inode *inode, struct file *file)
{

	printk("mxc622x release\n");

	return 0;
}

static int mxc622x_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	void __user *pa = (void __user *)arg;
	unsigned char data[16] = {0};
	int vec[3] = {0};

    static int sensor_sts=SENSOR_UNKNOW;  //when power on, reset z value to 0,up level will reset cache 

	switch (cmd) 
	{
		case MXC622X_IOC_PWRON:
			printk("mxc622X power on\n");

            if(sensor_sts == SENSOR_UNKNOW){
                //put init code here
			}
            sensor_sts = SENSOR_OPENED;
            
			data[0] = MXC622X_REG_CTRL;
			data[1] = MXC622X_CTRL_PWRON;
			if (mxc622x_i2c_tx_data(data, 2) < 0) {
				return -EFAULT;
			}
/*	wait PWRON done */
			msleep(MXC622X_DELAY_PWRON);
			break;

		case MXC622X_IOC_PWRDN:

			printk("mxc622X power down\n");

            sensor_sts = SENSOR_CLOSED;

			data[0] = MXC622X_REG_CTRL;
			data[1] = MXC622X_CTRL_PWRDN;
			if (mxc622x_i2c_tx_data(data, 2) < 0) {
				return -EFAULT;
			}
/* wait PWRDN done */
			msleep(MXC622X_DELAY_PWRDN);
			break;

		case MXC622X_IOC_READXYZ:

			if (reg_read_n(MXC622X_REG_DATA,data, 2) < 0){
				return -EFAULT;
			}

			vec[0] =(int)GET_BITS(data[MXC_XOUT],MXC_xOUT_DATA);  //abs value
			vec[1] =(int)GET_BITS(data[MXC_YOUT],MXC_xOUT_DATA);

            if(GET_BITS(data[MXC_XOUT],MXC_xOUT_DIRECT)){   //pola
                vec[0]-=ADC_MAX;
            }
            if(GET_BITS(data[MXC_YOUT],MXC_xOUT_DIRECT)){
                vec[1]-=ADC_MAX;
            }

			vec[2] = vec[0]*vec[0] + vec[1]*vec[1];
			if(vec[2] < 441)   //make z more than 1/3 G (64 count/g)
                vec[2] = 441;

            //64 count/g
            if(sensor_sts == SENSOR_OPENED){
                vec[2]=0;
                sensor_sts = SENSOR_WORKING;
            }/*else{
                vec[2] = 1<<3;  //no z info,emulate it
            }*/
            
#if DEBUG
            printk("[X: %08x] [Y: %08x] [Z: %08x],[X: %d] [Y: %d] [Z: %d]\n", 
                vec[0], vec[1], vec[2],vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) {
				return -EFAULT;
			}
			break;

		case MXC622X_IOC_READSTATUS:
			printk("mxc622X read status\n");
			//data[0] = MXC622X_REG_DATA;

			if (reg_read_n(MXC622X_REG_DATA, data, 3) < 0){
				return -EFAULT;
			}

			vec[0] = (int)data[0];
			vec[1] = (int)data[1];
			vec[2] = (int)data[2];
#if DEBUG
			printk("[X - %04x] [Y - %04x] [STATUS - %04x]\n", 
				vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) {
				return -EFAULT;
			}
			break;

		case MXC622X_IOC_SETDETECTION:
			printk("mxc622X set detection\n");

			data[0] = MXC622X_REG_CTRL;

			if (copy_from_user(&(data[1]), pa, sizeof(unsigned char))) {
				return -EFAULT;
			}

			if (mxc622x_i2c_tx_data(data, 2) < 0) {
				return -EFAULT;
			}
            /* wait SETDETECTION done */
			msleep(MXC622X_DELAY_SETDETECTION);
			break;
        case MXC622X_IOC_GET_DIR:
            //printk("mma7660 get dir\n");

          #if defined(CONFIG_BOARD_E2)
            data[0] = 4;
          #endif
          #if defined(CONFIG_BOARD_E3)
            data[0] = 3;
          #endif  
          #if defined(CONFIG_BOARD_E4)
            data[0] = 2;
          #endif    
          #if defined(CONFIG_BOARD_E5)
            data[0] = 6;
          #endif             
          #if defined(CONFIG_BOARD_F0)
            data[0] = 0;
          #endif           
          #if defined(CONFIG_BOARD_G0)||defined(CONFIG_BOARD_G0_3G)
            data[0] = 0;
          #endif
          #if defined(CONFIG_BOARD_H0)
            data[0] = 1;
          #endif              
          #if defined(CONFIG_BOARD_I0)
            data[0] = 4;
          #endif
          #if defined(CONFIG_BOARD_J0)
            data[0] = 1;
          #endif         
          #if defined(CONFIG_BOARD_K0)
            data[0] = 6;
          #endif  

			if (copy_to_user(pa, data, 1)) 
			{
				return -EFAULT;
			}
            break;

		default:
			break;
	}

	return 0;
}

static ssize_t mxc622x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "MXC622X");
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(mxc622x, S_IRUGO, mxc622x_show, NULL);

static struct file_operations mxc622x_fops = {
	.owner		= THIS_MODULE,
	.open		= mxc622x_open,
	.release	= mxc622x_release,
	.ioctl		= mxc622x_ioctl,
};

static struct miscdevice mxc622x_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mxc622x",
	.fops = &mxc622x_fops,
};

int mxc622x_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int res = 0;
//---------------------------------------------------------------
	//unsigned char data[16] = {0};				// add by snowwan
//---------------------------------------------------------------

	printk("mxc622x_probe start\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s: functionality check failed\n", __FUNCTION__);
		res = -ENODEV;
		goto out;
	}
	
	this_client = client;

	res = misc_register(&mxc622x_device);
	if (res) {
		pr_err("%s: mxc622x_device register failed\n", __FUNCTION__);
		goto out;
	}

	res = device_create_file(&client->dev, &dev_attr_mxc622x);
	if (res) {
		pr_err("%s: device_create_file failed\n", __FUNCTION__);
		goto out_deregister;
	}

	printk("mxc622x_probe start ok\n");
	return 0;

out_deregister:
	misc_deregister(&mxc622x_device);
out:
	return res;
}

static int mxc622x_remove(struct i2c_client *client)
{
	device_remove_file(&client->dev, &dev_attr_mxc622x);
	misc_deregister(&mxc622x_device);

	return 0;
}

static const struct i2c_device_id mxc622x_id[] = {
	{ MXC622X_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver mxc622x_driver = {
	.probe 		= mxc622x_probe,
	.remove 	= mxc622x_remove,
	.id_table	= mxc622x_id,
	.driver 	= {
		.owner	= THIS_MODULE,
		.name = MXC622X_I2C_NAME,
	},
};

static int sensor_driver_register_iic(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	
	if(i2c_add_driver(&mxc622x_driver)){
		printk("%s i2c_add_driver Error! \n",__func__);
		return -1;
	}
	
	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = IIC_ADDRESS;
	strlcpy(info.type, mxc622x_driver.driver.name, sizeof(info.type));

	adapter = i2c_get_adapter(IIC_BUS);
	if (!adapter) {
		printk("%s : can't get i2c adapter %d\n",__func__,IIC_BUS);
		goto err_driver;
	}
	client = i2c_new_device(adapter, &info);

	i2c_put_adapter(adapter);
	if (!client) {
		printk("%s : can't add i2c device at 0x%x\n",__func__,
			(unsigned int)info.addr);
		goto err_driver;
	}
	
	printk("sensor_driver_register_iic : i2c add %s success! \n",mxc622x_driver.driver.name);
	
	return 0;

err_driver:
	i2c_del_driver(&mxc622x_driver);
	return -ENODEV;
}

static int __init mxc622x_init(void)
{
	printk("mxc622x driver: init\n");
	if(sensor_driver_register_iic()){
		printk("mxc622x_exit : Fail to register iic for sensor device\n");
		return -1;
    }
    return 0;	
	//return i2c_add_driver(&mxc622x_driver);
}

static void __exit mxc622x_exit(void)
{
	printk("mxc622x driver: exit\n");
	i2c_del_driver(&mxc622x_driver);

}

module_init(mxc622x_init);
module_exit(mxc622x_exit);

MODULE_AUTHOR("Robbie Cao<hjcao@memsic.com>");
MODULE_DESCRIPTION("MEMSIC MXC622X (DTOS) Accelerometer Sensor Driver");
MODULE_LICENSE("GPL");

