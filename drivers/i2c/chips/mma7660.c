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

#include "mma7660.h"

#define DEBUG							0
#define IIC_ADDRESS     MMA7660_I2C_ADDR
#define IIC_BUS         0x1

#define MAX_FAILURE_COUNT				3

#define MMA7660_DELAY_PWRON				300							/* ms, >= 300 ms */
#define MMA7660_DELAY_PWRDN				1							/* ms */
#define MMA7660_DELAY_SETDETECTION		MMA7660_DELAY_PWRON

#define MMA7660_RETRY_COUNT				3

static struct i2c_client *this_client;

static int mma7660_i2c_tx_data(char *buf, int len)
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

	printk("%s reg 0x%x : 0x%x\n",__func__,buf[0],buf[1]);
error:	
	return ret;
}

static int reg_write_1(u8 reg,u8 data)
{
	int ret;
    char buf[2];

	buf[0]=reg;
	buf[1]=data;
	ret = i2c_master_send(this_client, buf, 2);
	if (ret < 0){
		printk("%s reg 0x%x data 0x%x failed\n",__func__,reg,data);
		goto error;
}

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

	



static int mma7660_open(struct inode *inode, struct file *file)
{
	printk("mma7660 open\n");


	return nonseekable_open(inode, file);
}

static int mma7660_release(struct inode *inode, struct file *file)
{

	printk("mma7660 release\n");

	return 0;
}

static int mma7660_ioctl(struct inode *inode, struct file *file,
	unsigned int cmd, unsigned long arg)
{
	void __user *pa = (void __user *)arg;
	unsigned char data[16] = {0};
	int vec[3] = {0};
    u8 val,retry;

    static int sensor_sts=SENSOR_UNKNOW;  //when power on, reset z value to 0,up level will reset cache 

	switch (cmd) 
	{
		case MMA7660_IOC_PWRON:
			printk("mma7660 power on\n");

            if(sensor_sts == SENSOR_UNKNOW){
                reg_write_1(MMA_MODE,0); //Make 7660 enter standby mode to set registers
                
                reg_write_1(MMA_SPCNT,0xff);
                reg_write_1(MMA_INTSU,/*MMA_INT_FB|MMA_INT_PL*/1);
                
                val=0;
                SET_BITS(val,MMA_SR_AMSR,AMSR_RATE_32);
                SET_BITS(val,MMA_SR_AWSR,AWSR_RATE_32);
                SET_BITS(val,MMA_SR_FILT,FILT_DEBOUNCE_3);
                reg_write_1(MMA_SR,val);
                
                val=0;
                SET_BITS(val,MMA_PDET_PDTH,0x3f);
                SET_BITS(val,MMA_PDET_XDA,0);
                SET_BITS(val,MMA_PDET_YDA,0);
                SET_BITS(val,MMA_PDET_ZDA,0);
                reg_write_1(MMA_PDET,val);
                
                reg_write_1(MMA_PD,0xff);
                
                reg_write_1(MMA_MODE,
                                    MMA_MODE_MODE/*|MMA_MODE_AWE*/|  //active
                                    /*MMA_MODE_ASE|MMA_MODE_SCPS|*/  //auto wake/suspend
                                    MMA_MODE_IPP);               //int active low
            }
            sensor_sts = SENSOR_OPENED;
            
            if (reg_read_n(MMA_MODE,&data[0],1) < 0) {
                return -EFAULT;
            }
			data[0]|=MMA_MODE_MODE;
			if (reg_write_1(MMA_MODE,data[0]) <0) {
                return -EFAULT;
			}
/*	wait PWRON done */
			msleep(MMA7660_DELAY_PWRON);
			break;

		case MMA7660_IOC_PWRDN:

			printk("mma7660 power down\n");

            sensor_sts = SENSOR_CLOSED;

            if (reg_read_n(MMA_MODE,&data[0],1) < 0) {
                return -EFAULT;
            }
			data[0]&=~MMA_MODE_MODE;
			if (reg_write_1(MMA_MODE,data[0]) <0)
			{
                return -EFAULT;
			}

/* wait PWRDN done */
			msleep(MMA7660_DELAY_PWRDN);
			break;

		case MMA7660_IOC_READXYZ:
#if DEBUG
            printk("mma7660 read xyz\n");
#endif
            retry = 5;
            do{
    			if (reg_read_n(MMA_XOUT,data, 3) < 0){
    				return -EFAULT;
    			}
    			retry--;
    			if(!retry)
    			    break;
            }while(GET_BITS(data[MMA_XOUT],MMA_xOUT_ALERT)||    //steadable
                    GET_BITS(data[MMA_YOUT],MMA_xOUT_ALERT)||
                    GET_BITS(data[MMA_ZOUT],MMA_xOUT_ALERT)/*||
                    !GET_BITS(data[MMA_XOUT],MMA_xOUT_DATA)*/);
            if(retry){
    			vec[0] =(int)GET_BITS(data[MMA_XOUT],MMA_xOUT_DATA);  //abs value
    			vec[1] =(int)GET_BITS(data[MMA_YOUT],MMA_xOUT_DATA);
    			vec[2] =(int)GET_BITS(data[MMA_ZOUT],MMA_xOUT_DATA);

                if(GET_BITS(data[MMA_XOUT],MMA_xOUT_DIRECT)){   //pola
                    vec[0]-=ADC_MAX;
                }
                if(GET_BITS(data[MMA_YOUT],MMA_xOUT_DIRECT)){
                    vec[1]-=ADC_MAX;
                }
                if(GET_BITS(data[MMA_ZOUT],MMA_xOUT_DIRECT)){
                    vec[2]-=ADC_MAX;
                }
#if 0
    //#if DEBUG
                printk("[X: %08x] [Y: %08x] [Z: %08x],[X: %d] [Y: %d] [Z: %d]\n", 
                    vec[0], vec[1], vec[2],vec[0], vec[1], vec[2]);
#endif            
                vec[0]*=3;  //current count/g = 21.33,so mutiply 3 to make it to interger
                vec[1]*=3;
                if(sensor_sts == SENSOR_OPENED){
                    vec[2]=0;
                    sensor_sts = SENSOR_WORKING;
                }else{
                    vec[2]*=3;
                }
            }else{
                printk("MMA7660_IOC_READXYZ try out\n");
                memset(vec,0,sizeof(vec));
            }
#if DEBUG
            printk("[X: %08x] [Y: %08x] [Z: %08x],[X: %d] [Y: %d] [Z: %d]\n", 
                vec[0], vec[1], vec[2],vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) {
				return -EFAULT;
			}
			break;

		case MMA7660_IOC_READSTATUS:
			printk("mma7660 read status\n");

			if (reg_read_n(MMA_XOUT, data, 4) < 0) {
				return -EFAULT;
			}

			vec[0] = (int)data[0];
			vec[1] = (int)data[1];			
			vec[2] = (unsigned int)data[3];  //status
#if DEBUG
			printk("[X - %04x] [Y - %04x] [STATUS - %04x]\n", 
				vec[0], vec[1], vec[2]);
#endif
			if (copy_to_user(pa, vec, sizeof(vec))) 
			{
				return -EFAULT;
			}
			break;

		case MMA7660_IOC_SETDETECTION:
			printk("mma7660 set detection\n");

			data[0] = MMA_INTSU;

			if (copy_from_user(&(data[1]), pa, sizeof(unsigned char))) 
			{
				return -EFAULT;
			}

			if (mma7660_i2c_tx_data(data, 2) < 0) 
			{
				return -EFAULT;
			}
/* wait SETDETECTION done */
			msleep(MMA7660_DELAY_SETDETECTION);
			break;
        case MMA7660_IOC_GET_DIR:
            //printk("mma7660 get dir\n");

          #if defined(CONFIG_BOARD_E2)
            data[0] = 1;
          #endif
          #if defined(CONFIG_BOARD_E3)
            data[0] = 6;
          #endif  
          #if defined(CONFIG_BOARD_E4)
            data[0] = 7;
          #endif 
          #if defined(CONFIG_BOARD_E5)
            data[0] = 7;
          #endif              
          #if defined(CONFIG_BOARD_F0)
            data[0] = 1;
          #endif           
          #if defined(CONFIG_BOARD_G0)||defined(CONFIG_BOARD_G0_3G)
            data[0] = 3;
          #endif
          #if defined(CONFIG_BOARD_H0)
            data[0] = 4;
          #endif              
          #if defined(CONFIG_BOARD_I0)
            data[0] = 1;
          #endif
          #if defined(CONFIG_BOARD_J0)
            data[0] = 4;
          #endif         
          #if defined(CONFIG_BOARD_K0)
            data[0] = 3;
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

static ssize_t mma7660_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "MMA7660");
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(mma7660, S_IRUGO, mma7660_show, NULL);

static struct file_operations mma7660_fops = {
	.owner		= THIS_MODULE,
	.open		= mma7660_open,
	.release	= mma7660_release,
	.ioctl		= mma7660_ioctl,
};

static struct miscdevice mma7660_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mma7660",
	.fops = &mma7660_fops,
};

int mma7660_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int res = 0;

	printk("mma7660_probe start\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		pr_err("%s: functionality check failed\n", __FUNCTION__);
		res = -ENODEV;
		goto out;
	}
	
	this_client = client;

	res = misc_register(&mma7660_device);
	if (res) {
		pr_err("%s: mma7660_device register failed\n", __FUNCTION__);
		goto out;
	}

	res = device_create_file(&client->dev, &dev_attr_mma7660);
	if (res) {
		pr_err("%s: device_create_file failed\n", __FUNCTION__);
		goto out_deregister;
	}

	printk("mma7660_probe start ok\n");
	return 0;

out_deregister:
	misc_deregister(&mma7660_device);
out:
	return res;
}

static int mma7660_remove(struct i2c_client *client)
{
	device_remove_file(&client->dev, &dev_attr_mma7660);
	misc_deregister(&mma7660_device);

	return 0;
}

static const struct i2c_device_id mma7660_id[] = {
	{ MMA7660_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver mma7660_driver = {
	.probe 		= mma7660_probe,
	.remove 	= mma7660_remove,
	.id_table	= mma7660_id,
	.driver 	= {
		.owner	= THIS_MODULE,
		.name = MMA7660_I2C_NAME,
	},
};

static int sensor_driver_register_iic(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	
	if(i2c_add_driver(&mma7660_driver)){
		printk("%s i2c_add_driver Error! \n",__func__);
		return -1;
	}
	
	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = IIC_ADDRESS;
	strlcpy(info.type, mma7660_driver.driver.name, sizeof(info.type));

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
	
	printk("sensor_driver_register_iic : i2c add %s success! \n",mma7660_driver.driver.name);
	
	return 0;

err_driver:
	i2c_del_driver(&mma7660_driver);
	return -ENODEV;
}

static int __init mma7660_init(void)
{
	printk("mma7660 driver: init\n");
	if(sensor_driver_register_iic()){
		printk("mma7660_exit : Fail to register iic for sensor device\n");
		return -1;
    }
    return 0;	
	//return i2c_add_driver(&mma7660_driver);
}

static void __exit mma7660_exit(void)
{
	printk("mma7660 driver: exit\n");
	i2c_del_driver(&mma7660_driver);

}

module_init(mma7660_init);
module_exit(mma7660_exit);

MODULE_AUTHOR("Robbie Cao<hjcao@memsic.com>");
MODULE_DESCRIPTION("FREESCALE MMA7660 (DTOS) Accelerometer Sensor Driver");
MODULE_LICENSE("GPL");

