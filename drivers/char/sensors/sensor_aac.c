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
 * Description: main file of aac media sensor driver
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
#include <linux/i2c.h>

/*
 * XXX: sensor hard ware directly connect to system bus, there is 
 * no use to clock set
 */ 
#include <plat/clock.h>

#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>

#include "common_sensor.h"
#include "sensor_aac.h"

#define DEV_NAME "sensor"


#define IIC_ADDRESS     0x4C
#define IIC_BUS         0x1

/*
 * functions delare
 */
static int sensor_driver_register_device(void);	/* register driver as an char device */
static int sensor_driver_unregister_device(void);
static int sensor_driver_register_iic(void);
static void sensor_driver_unregister_iic(void);

/*
 * this structure include global varaibles
 */
//static wait_queue_head_t wait_sensor;	/* a wait queue for poll system call */
static DECLARE_MUTEX(sensor_access_mutex);
static DECLARE_MUTEX(sensor_data_mutex);

//static struct completion sensor_comp;

struct sensor_device{
	struct delayed_work data_work;
	struct completion data_comp;
	struct i2c_client *client;

    unsigned long default_poll_time;
	int orientation;      //last windows flip value
	int open_cnt;
	int ori_enable_cnt;
	int aac_enable_cnt;
	
	u8 values[MMA_INTSU+1];
	int axis[3];  //x,y,z
};

static struct sensor_device sensor_priv; 


enum{
    ID_ACCELERATION=0,
    //ID_MAGNETIC_FIELD,
    ID_ORIENTATION,
    //ID_TEMPERATURE,
    ID_MAX};
//blow must be same as framework
#define SENSER_NAME_AAC "acceleration"
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
    return msecs_to_jiffies(sensor_poll_time);
}

static spinlock_t irq_lock;

//XEINT5
int sensor_interrupt_init(void)
{
    unsigned long  /*value,*/flags;
    unsigned long rEINTCON_value, rEINTFLTCON0_value;

    spin_lock_init(&irq_lock);
    
    spin_lock_irqsave(&irq_lock,flags);

	rEINTCON_value = __raw_readl(rEINTCON);
	rEINTCON_value &= ~( 0xf<<20 );
	rEINTCON_value |= 0x02<<20;
	__raw_writel(rEINTCON_value, rEINTCON);           //EINTCON

	rEINTFLTCON0_value = __raw_readl(rEINTFLTCON1);
	rEINTFLTCON0_value |= 0xff<<8;
	__raw_writel(rEINTFLTCON0_value, rEINTFLTCON1);        //EINTFILTER

    spin_unlock_irqrestore(&irq_lock,flags);

    return 0;
}
void sensor_interrupt_disable(void)
{
    /*
    unsigned long  value,flags;

    spin_lock_irqsave(&irq_lock,flags);
    
	value = __raw_readl(rEINTG4MASK);
	value |= (0x1<<28);
	__raw_writel(value, rEINTG4MASK);

    spin_unlock_irqrestore(&irq_lock,flags);
    */   
	disable_irq(IRQ_EINT5);
}

void sensor_interrupt_enable(void)
{ /*
    unsigned long  value,flags;

    spin_lock_irqsave(&irq_lock,flags);
    
   
	value = __raw_readl(rEINTG4MASK);
	value &= ~(0x1<<28);
	__raw_writel(value, rEINTG4MASK);

    spin_unlock_irqrestore(&irq_lock,flags);
     */  
	enable_irq(IRQ_EINT5);
}

int sensor_interrupt_clear(void)
{
    /*
    unsigned long  flags;
    int value;

    spin_lock_irqsave(&irq_lock,flags);
    

    spin_unlock_irqrestore(&irq_lock,flags);

    return value&(0x1<<28);*/
	return 1;
}


static int reg_write(struct i2c_client *client,u8 reg,u8 data)
{
	int ret;
    char buf[2];

	
	down(&sensor_access_mutex);
	buf[0]=reg;
	buf[1]=data;
	ret = i2c_master_send(client, buf, 2);
	if (ret < 0){
		printk("%s reg 0x%x data 0x%x failed\n",__func__,reg,data);
		goto error;
	}
	/*
	ret = i2c_master_send(client, &data, 1);
	if (ret < 0){
		printk("%s reg 0x%x data 0x%x failed #2\n",__func__,reg,data);		
	}*/

	printk("%s reg 0x%x : 0x%x\n",__func__,reg,data);
error:	
	up(&sensor_access_mutex);
	return ret;
}

static inline int reg_read(struct i2c_client *client, u8 reg,u8 * data)
{
	int ret;
	
	down(&sensor_access_mutex);
	/* Set address */
	ret = i2c_master_send(client, &reg, 1);
	if (ret < 0){
		printk("%s reg 0x%x failed #1\n",__func__,reg);
		goto error;
	}

	ret = i2c_master_recv(client, data, 1);
	if (ret < 0){
		printk("%s reg 0x%x failed #2\n",__func__,reg);
		goto error;
	}

    //printk("%s reg 0x%x : 0x%x\n",__func__,reg,*data);

error:
	up(&sensor_access_mutex);
	return ret;
}

static inline int reg_read_n(struct i2c_client *client, u8 reg,u8 * data,int n)
{
	int ret;
	
	down(&sensor_access_mutex);
	/* Set address */
	ret = i2c_master_send(client, &reg, 1);
	if (ret < 0){
		printk("%s reg 0x%x failed #1\n",__func__,reg);
		goto error;
	}

	ret = i2c_master_recv(client, data, n);
	if (ret < 0){
		printk("%s reg 0x%x failed #2\n",__func__,reg);
		goto error;
	}

    //printk("%s reg 0x%x : 0x%x\n",__func__,reg,*data);

error:
	up(&sensor_access_mutex);
	return ret;
}

/*
 * open system call just mark file private data as a sensor 
 * instance by default, and you can change it by a ioctl call
 */
static int aac_sensor_open(struct inode *inode, struct file *file)
{
    struct sensor_device *dev;

    dev=&sensor_priv;

    down(&sensor_data_mutex);

    dev->open_cnt++;

    up(&sensor_data_mutex);
	/* dec instance by default, you can change it by ioctl pp instance */
	file->private_data = dev;

    //schedule_delayed_work(&dev->data_work,HZ/100);

	return 0;
}

/*
 * fasync system call be called here
 */
static int aac_sensor_release(struct inode *inode, struct file *file)
{
    struct sensor_device *dev;

    dev=&sensor_priv;

    down(&sensor_data_mutex);

    dev->open_cnt--;

    up(&sensor_data_mutex);

    
	cancel_work_sync(&(sensor_priv.data_work.work));
	return 0;
}

typedef struct {
    union {
        int v[3];
        struct {
            int x;
            int y;
            int z;
        };
    };
} sensors_vec_t;

static int data_filter(u8 *values,int *axis,int event)
{
    int xyfilter,zfilter;

    int x,y,z;
    int ret = -1;

#define XY_AAC_FILTER 1
#define Z_AAC_FILTER 30


#define XY_ORI_FILTER 10//7
#define Z_ORI_FILTER 21//19


    if(event){
        xyfilter = XY_ORI_FILTER;
        zfilter  = Z_ORI_FILTER;    
    }else{
        xyfilter = XY_AAC_FILTER;
        zfilter  = Z_AAC_FILTER;
    }

    x=values[MMA_XOUT];
    if(GET_BITS(x,MMA_xOUT_DIRECT)){
        x=-(BIT_MASK(MMA_xOUT_DATA)-GET_BITS(x,MMA_xOUT_DATA)+1);
    }else{
        x=GET_BITS(x,MMA_xOUT_DATA);
    }
    y=values[MMA_YOUT];
    if(GET_BITS(y,MMA_xOUT_DIRECT)){
        y=-(BIT_MASK(MMA_xOUT_DATA)-GET_BITS(y,MMA_xOUT_DATA)+1);
    }else{
        y=GET_BITS(y,MMA_xOUT_DATA);
    }
    z=values[MMA_ZOUT];
    if(GET_BITS(z,MMA_xOUT_DIRECT)){
        z=-(BIT_MASK(MMA_xOUT_DATA)-GET_BITS(z,MMA_xOUT_DATA)+1);
    }else{
        z=GET_BITS(z,MMA_xOUT_DATA);
    }

    if(z <zfilter || z> -zfilter){
        if((x > xyfilter || x < -xyfilter)||(y > xyfilter || y < -xyfilter)){
            ret = 0;
        }
    }

    if(axis){
        axis[0]=x;
        axis[1]=y;
        axis[2]=z;
    }

    if(ret){
        sensor_debug("bad data x %d y %d z %d\n",x,y,z);
    }

    return ret;
}

static int query_one_data(struct sensor_device *dev,u8 *val,int len)
{
    struct i2c_client *client;
    int read_len;

    char alert=0;
    int ret = 0;


    if(len<MMA_TILT+1){
        printk("%s bad reg len %d\n",__func__,len);
        return -1;
    }

    client=dev->client;

    do{
        read_len=reg_read_n(client,MMA_XOUT,val,len);

        if(read_len!=len){
            ret = -1;
            break;  //bad read
        }
        
        if(!(GET_BITS(val[MMA_XOUT],MMA_xOUT_ALERT))&&
            !(GET_BITS(val[MMA_YOUT],MMA_xOUT_ALERT))&&
            !(GET_BITS(val[MMA_ZOUT],MMA_xOUT_ALERT))){
            ret = 0;
            break;
        }
        
        alert++;
        if(alert==10)
            printk("%s data alert x=0x%x y=0x%x z=0x%x tilt=0x%x alert=%d\n",__func__,
                val[MMA_XOUT],val[MMA_YOUT],val[MMA_ZOUT],val[MMA_TILT],alert);
    }while(read_len==read_len && alert <= 10);

    return ret;
}

#define MAX_MATCH_TIME 3

static int aac_sensor_query(struct sensor_device *dev,int event)
{
	u8 values[MMA_INTSU+1];

    int i,retry;

    int ret = -1;

    if(event){
        msleep(500);
    }

    if(event)
        retry = 5;
    else
        retry = 1;

    for(i=0;i<retry;i++){
        if(query_one_data(dev,values,MMA_INTSU+1)!=0){
            sensor_debug("event %d query_one_data failed\n\n",event);
            break;
        }

        if(data_filter(values,dev->axis,event)==0){
            break;  //valid data
        }
    }

    if(i<retry){
        down(&sensor_data_mutex);
        if(memcmp(values,dev->values,sizeof(dev->values))){
            memcpy(dev->values,values,sizeof(dev->values));
            ret =0;
        }

        up(&sensor_data_mutex);
    }

    return ret;
}


static int aac_sensor_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct sensor_device *dev;
    //sensors_vec_t vec[ID_MAX];
    int x=0,y=0,z=0,temp;
	char data[255],*curr;
    char bit;

    int event;

    int result;
    
    dev=&sensor_priv;

	event=wait_for_completion_timeout(&dev->data_comp,get_poll_time());
    result = aac_sensor_query(dev,event);  //get new data

    down(&sensor_data_mutex);

    curr=data;

    if(result == 0){
        if(event){
            bit=GET_BITS(dev->values[MMA_TILT],MMA_TILT_ALERT);
            if(!bit){
                bit=GET_BITS(dev->values[3],MMA_TILT_POLA);

               // board e2
              #if defined(CONFIG_BOARD_E2)  
                if(bit==POLA_LEFT){           
                    x=180;
                }else if(bit==POLA_RIGHT){
                    x=0;
                }else if(bit==POLA_UP){
                    x=270;
                }else if(bit==POLA_DOWN){
                    x=90;
                }else{
                    event=0;
                }
              #endif
                //board 3e
              #if defined(CONFIG_BOARD_E3)  
                if(bit==POLA_LEFT){          
                    x=0;
                }else if(bit==POLA_RIGHT){
                    x=180;
                }else if(bit==POLA_UP){
                    x=270;
                }else if(bit==POLA_DOWN){
                    x=90;
                }else{
                    event=0;
                }
              #endif               
                //board g0
              #if defined(CONFIG_BOARD_G0)||defined(CONFIG_BOARD_G0_3G)   
                if(bit==POLA_LEFT){           
                    x=90;
                }else if(bit==POLA_RIGHT){
                    x=270;
                }else if(bit==POLA_UP){
                    x=180;
                }else if(bit==POLA_DOWN){
                    x=0;
                }else{
                    event=0;
                }          
              #endif
                //board h0
              #if defined(CONFIG_BOARD_H0)    
                if(bit==POLA_LEFT){           
                    x=180;
                }else if(bit==POLA_RIGHT){
                    x=0;
                }else if(bit==POLA_UP){
                    x=90;
                }else if(bit==POLA_DOWN){
                    x=270;
                }else{
                    event=0;
                }          
              #endif              
                //board i0
              #if defined(CONFIG_BOARD_I0)||defined(CONFIG_BOARD_K0)
                if(bit==POLA_LEFT){           
                    x=270;
                }else if(bit==POLA_RIGHT){
                    x=90;
                }else if(bit==POLA_UP){
                    x=0;
                }else if(bit==POLA_DOWN){
                    x=180;
                }else{
                    event=0;
                }          
              #endif
                //board j0
              #if defined(CONFIG_BOARD_J0)||defined(CONFIG_BOARD_L0)
                if(bit==POLA_LEFT){           
                    x=270;
                }else if(bit==POLA_RIGHT){
                    x=90;
                }else if(bit==POLA_UP){
                    x=180;
                }else if(bit==POLA_DOWN){
                    x=0;
                }else{
                    event=0;
                }          
              #endif              
                if(x==dev->orientation){
                    event=0;
                }else{
                    dev->orientation=x;
                }
            }else{
                event=0;
            }
        }

        //ID_ACCELERATION
        if(event==0){
            x=dev->axis[0];
            y=dev->axis[1];
            z=dev->axis[2];

            //board e2
          #if defined(CONFIG_BOARD_E2)    
            temp=x;
            x=y;y=temp;z=-z;
          #endif

            //board e3
          #if defined(CONFIG_BOARD_E3)    
            temp=x;
            x=x;y=y;z=-z;
          #endif
            // board g0
          #if defined(CONFIG_BOARD_G0)||defined(CONFIG_BOARD_G0_3G)
            temp=x;
            x=y;y=temp;z=-z;
          #endif
            // board h0
          #if defined(CONFIG_BOARD_H0)
            temp=x;
            x=y;y=temp;z=-z;
          #endif            
          #if defined(CONFIG_BOARD_I0)||defined(CONFIG_BOARD_K0)
            temp=x;
            x=y;y=temp;z=-z;
          #endif 
          #if defined(CONFIG_BOARD_J0)||defined(CONFIG_BOARD_L0)
            temp=x;
            x=y;y=-temp;z=-z;
          #endif           
        	sprintf(curr,"%s:%d:%d:%d ",
        	    SENSER_NAME_AAC,
        	    x,y,z);
            curr+=strlen(curr);
        //ID_ORIENTATION    
        }else {
            y=z=0;
            sprintf(curr,"%s:%d:%d:%d ",
                SENSER_NAME_ORI,
                x,y,z);
                
            curr+=strlen(curr);
        }
        if(event){
            sensor_debug("event %d x=%d y=%d z=%d\n",event,x,y,z);
            sensor_debug("read data: %s (%d) x %d y %d z %d  filt 0x%x  status 0x%x\n",data,count,
                dev->axis[0],dev->axis[1],dev->axis[2],
                dev->values[MMA_TILT],
                dev->values[MMA_INTSU]);
            
        }
    }
    //time
	sprintf(curr,"sync:%lld",
	    time_to_ns());
  
	count=strlen(data);

	up(&sensor_data_mutex);
	
	if (copy_to_user(buf, data, count)) {
		return -EFAULT;
	}
	
	return count;
}

static int aac_sensor_write(struct file *filp,const char __user *buf, size_t count, loff_t *f_pos)
{
    struct sensor_device *dev;
    char    command[128];    
    int     params;

    size_t len;

    dev=&sensor_priv;

    len=min(sizeof(command)-1,count);
    if(copy_from_user(command,buf,len)){
        return -EFAULT;
    }

    /* read the next event */
    //sensor_debug("count=%d len=%d\n",count,len);

    command[len] = 0;
    sensor_debug("%s aac %d ori %d poll %ld %ld\n",command,
        dev->aac_enable_cnt,dev->ori_enable_cnt,
        dev->default_poll_time,
        get_poll_time());

    /* "set:%s:%d" corresponds to control__activate() */
    if(sscanf(command, "set:" SENSER_NAME_AAC ":%d",&params) == 1){
        sensor_debug("%s enable=%d\n",SENSER_NAME_AAC,params);
        if(params == 1){
            dev->aac_enable_cnt++;
        }else if(params == 0){
            if(dev->aac_enable_cnt>0)
                dev->aac_enable_cnt--;
        }else{
            sensor_debug("%s unknow cmd para params=%d\n",__func__,params);
        }
        if(dev->aac_enable_cnt<=0){
            set_poll_time(dev->default_poll_time);
        }
            
    }else if (sscanf(command, "set:" SENSER_NAME_ORI ":%d",&params) == 1) {
        sensor_debug("%s enable=%d\n",SENSER_NAME_ORI,params);
        if(params == 1){
            sensor_interrupt_enable();
            dev->ori_enable_cnt++;
            if(dev->ori_enable_cnt==1)
                set_poll_time(dev->default_poll_time);
        }else if(params == 0){
            sensor_interrupt_disable();
            if(dev->ori_enable_cnt>0)
                dev->ori_enable_cnt--;
            if(dev->ori_enable_cnt==0)
                set_poll_time(MAX_SCHEDULE_TIMEOUT);
        }else{
            sensor_debug("%s unknow cmd para params=%d\n",__func__,params);
        }

        if(dev->ori_enable_cnt==0){
            set_poll_time(MAX_SCHEDULE_TIMEOUT);
        }else if(dev->ori_enable_cnt==1){
            set_poll_time(dev->default_poll_time);
        }
    /* "set-delay:%d" corresponds to control__set_delay() */        
    }else if (sscanf(command, "set-delay:%d", &params) == 1) {
        //sensor_debug("%s set delay %d\n",__func__,params);
        set_poll_time((unsigned long)params);
        if(!dev->default_poll_time){
            dev->default_poll_time=(unsigned long)params;   //set first poll time as default poll time
        }
        complete(&dev->data_comp);
    /* "set-delay:%d" corresponds to control__wake() */
    }else if (!strcmp((const char*)command, "wake")) {
        //sensor_debug("%s set wake\n",__func__);
        set_poll_time(/*dev->default_poll_time*/MAX_SCHEDULE_TIMEOUT);        
    }else{
        sensor_debug("%s unknow command %s\n",__func__,command);
        len=0;
    }

    return len;
}

static int is_open_gsensor = 0;
static int aac_sensor_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("%s cmd = %d, arg = %ld \n",__func__,cmd,arg);
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

static struct file_operations aac_sensor_fops = 
{
	owner:		THIS_MODULE,
	open:		aac_sensor_open,
	read:		aac_sensor_read,
	write:		aac_sensor_write,
	release:	aac_sensor_release,
	ioctl: 		aac_sensor_ioctl,
};

static void aac_sensor_work(struct work_struct *work)
{
    struct delayed_work *d_work;
    struct sensor_device *dev;

    d_work=container_of(work,struct delayed_work,work);
    dev=container_of(d_work,struct sensor_device,data_work);

    complete(&dev->data_comp);

}

static irqreturn_t aac_sensor_irq(int irq, void *dev_id)
{
    struct sensor_device *dev;
	volatile unsigned int tmp;
	tmp = __raw_readl(rGPGDAT);    

    dev = (struct sensor_device *)dev_id;
	
	//printk("aac_sensor_irq : tmp = 0x%x\r\n",tmp);
	// 0x17
    if(( tmp & ( 1 << 5 )) == 0 ){
        schedule_delayed_work(&dev->data_work,HZ/100);
    }

	return IRQ_HANDLED;
}



/*
 * platform operation relate functions
 */
static int aac_sensor_probe(struct i2c_client *client,const struct i2c_device_id *idp)
{
    struct sensor_device *dev;
	u8  val;

    dev=&sensor_priv;
	/* initualize wait queue */
	init_completion(&dev->data_comp);
    INIT_DELAYED_WORK(&dev->data_work,aac_sensor_work);

    if(sensor_interrupt_init()){
        sensor_error("Fail to config gpio port\n");
        return -1;
    }


	if(request_irq(IRQ_EINT5, aac_sensor_irq,IRQF_DISABLED,"aac_sensor",dev)){
        sensor_error("Fail to request gpio interrupt\n");
        return -1;
	}
	sensor_interrupt_disable();
	sensor_interrupt_clear();
	
	
    reg_write(client,MMA_MODE,0); //Make 7660 enter standby mode to set registers

    reg_write(client,MMA_SPCNT,0xff);
    reg_write(client,MMA_INTSU,MMA_INT_FB|MMA_INT_PL/*|MMA_INT_PD*/);

    val=0;
    SET_BITS(val,MMA_SR_AMSR,AMSR_RATE_128);
    SET_BITS(val,MMA_SR_AWSR,AWSR_RATE_32);
    SET_BITS(val,MMA_SR_FILT,FILT_DISABLE);
    reg_write(client,MMA_SR,val);

    val=0;
    SET_BITS(val,MMA_PDET_PDTH,2);
    SET_BITS(val,MMA_PDET_XDA,1);
    SET_BITS(val,MMA_PDET_YDA,1);
    SET_BITS(val,MMA_PDET_ZDA,1);
    reg_write(client,MMA_PDET,val);
    
    reg_write(client,MMA_PD,0x1f);

    reg_write(client,MMA_MODE,
                        MMA_MODE_MODE|MMA_MODE_AWE|  //active
                        MMA_MODE_ASE/*|MMA_MODE_SCPS*/|  //auto wake/suspend
                        MMA_MODE_IPP);               //int active low

    sensor_priv.client=client;

	//i2c_set_clientdata(client, nas_priv);
	return 0;
}

static int aac_sensor_remove(struct i2c_client *client)
{
	/* release irq */	
	sensor_interrupt_disable();
	sensor_interrupt_clear();
	free_irq(IRQ_EINT5, NULL);
	
	return 0;
}


static const struct i2c_device_id aac_sensor_id[] = {
	{ "sensor_acc", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, aac_sensor_id);

static struct i2c_driver aac_sensor_driver = {
	.driver = {
	    .owner		= THIS_MODULE,
		.name = "sensor_acc",
	},
	.probe = aac_sensor_probe,
	.remove = aac_sensor_remove,
	.id_table = aac_sensor_id,
};


/*
 * init and exit
 */
static int __init aac_sensor_init(void)
{
	/* call probe */
	//enum sensor_state sensor_state_ori;
	/*
	if(platform_driver_register(&aac_sensor_driver)){
		sensor_error("Fail to register platform driver for sensor driver\n");
		return -EPERM;
	}*/

    if(sensor_driver_register_iic()){
		sensor_error("Fail to register iic for sensor device\n");
		return -1;
    }

	/* register char device driver */
	if(sensor_driver_register_device()){
		sensor_error("Fail to register char device for sensor device\n");
		return -1;
	}

	return 0;
}

static void __exit aac_sensor_exit(void)
{
	/* call remove */
	//platform_driver_unregister(&aac_sensor_driver);
	sensor_driver_unregister_device();
	sensor_driver_unregister_iic();
}

module_init(aac_sensor_init);
module_exit(aac_sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZT");
MODULE_DESCRIPTION("aac sensor driver");


/*
 * this function do driver register to regist a node under /dev
 */
static struct class *sensor_class;

int sensor_driver_register_device(void)
{
	if(register_chrdev(SENSOR_DEFAULT_MAJOR, "aac-sensor", &aac_sensor_fops)){
		sensor_error("register char deivce error\n");
		return -1;
	}

	sensor_class = class_create(THIS_MODULE, "aac-sensor");
	device_create(sensor_class, NULL, MKDEV(SENSOR_DEFAULT_MAJOR, SENSOR_DEFAULT_MINOR), NULL, DEV_NAME);

	return 0;
}

int sensor_driver_unregister_device(void)
{
	device_destroy(sensor_class, MKDEV(SENSOR_DEFAULT_MAJOR, SENSOR_DEFAULT_MINOR));
	class_destroy(sensor_class);
	unregister_chrdev(SENSOR_DEFAULT_MAJOR, "aac-sensor");

	return 0;
}


//static char banner[] __initdata = KERN_INFO "nastech Touchscreen driver, (c) 2010 nas Technologies Corp. \n";
static int sensor_driver_register_iic(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	
	if(i2c_add_driver(&aac_sensor_driver)){
		printk("%s i2c_add_driver Error! \n",__func__);
		return -1;
	}
	
	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = IIC_ADDRESS;
	strlcpy(info.type, aac_sensor_driver.driver.name, sizeof(info.type));

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
	
	printk("%s i2c add %s success! \n",__func__,aac_sensor_driver.driver.name);
	
	return 0;

err_driver:
	i2c_del_driver(&aac_sensor_driver);
	return -ENODEV;
}

static void sensor_driver_unregister_iic(void)
{
	printk("%s\n",__func__);

	i2c_del_driver(&aac_sensor_driver);
}


