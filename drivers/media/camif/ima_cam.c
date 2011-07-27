/***************************************************************************** 
 ** ima_cam.c 
 ** 
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 ** 
 ** Description: main file of ima media encode driver
 **
 ** Author:
 **     neville <haixu_fu@infotm.com>
 **      
 ** Revision History: 
 ** ­­­­­­­­­­­­­­­­­ 
 ** 1.1  06/24/2010 neville 
*******************************************************************************/

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
#include <linux/delay.h>
#include <linux/i2c.h>

#include <plat/clock.h>

#include <asm/gpio.h>
#include <mach/imapx_gpio.h>
#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>
#include "ima_cam.h"

#define IIC_ADAPTER 1
#define GPIO_CAMARA_EN IMAPX200_GPB(3)

static struct class *camif_class;
unsigned int frame = 0;
struct clk  *imap_cam_clk;

#define FLAG_SIZE    	4096
unsigned int    flag_phy_addr = 0;
unsigned int *  flag_vir_addr = NULL;

static unsigned int camif_open_count;

static wait_queue_head_t wait_camif;
static volatile int pr_flag;
static volatile int co_flag;
static struct mutex pr_mutex;
static struct mutex co_mutex;
static struct mutex flag_mutex;
struct ima_camif_param_t  *param;
struct timeval time1, time2;

extern struct cam_interface *bf3603_detect(int *invvsync);
extern struct cam_interface *hy511_detect(int *invvsync);
extern struct cam_interface *gc0308_detect(int *invvsync);
extern struct cam_interface *gc0307_detect(int *invvsync);


void *camara_device_table[]={
    hy511_detect,
    gc0308_detect,
    bf3603_detect,
    gc0307_detect,
    NULL,
};

struct cam_interface * device_interface;

void * get_camara_interface(int *invvsync)
{
    struct cam_interface *(*cam_table)(int *)=camara_device_table[0];
    struct cam_interface *dev_interface=NULL;

    int i;

    cam_power_set(CAMARA_POWER_ON);

    for(i=0;;i++){
        cam_table=camara_device_table[i];

        if(!*cam_table)
            break;
            
        dev_interface=(*cam_table)(invvsync);
        if(dev_interface){
            printk("get camara interface\n");
            break;
        }
    }
    
    cam_power_set(CAMARA_POWER_OFF);

    if(!dev_interface)
        printk("unkonw camara interface\n");

    return dev_interface;
}

//----------------------------
//      SENSOR_SET_MODE
//-----------------------------
int  cam_init(int invvsync)
{
    u32 val;
    if(device_interface&&
        device_interface->dev_init){

        //inv vsync
        if(invvsync){
            val=readl(param->ioaddr+IMAP_CIGCTRL);

            if(invvsync)
                val|=(1<<4);
            else
                val&=~(1<<4);
            writel(val,param->ioaddr+IMAP_CIGCTRL);        
        }
        return (*device_interface->dev_init)();
    }    
    
    return -1;
}

int  cam_switch_low_svga(void){
    if(device_interface&&
        device_interface->dev_switch_low_svga){
        return (*device_interface->dev_switch_low_svga)();
    }    
    
    return -1;
}
int  cam_switch_high_svga(void){
    if(device_interface&&
        device_interface->dev_switch_high_svga){
        return (*device_interface->dev_switch_high_svga)();
    }    
    
    return -1;
}
int  cam_switch_high_xuga(void){
    if(device_interface&&
        device_interface->dev_switch_high_xuga){
        return (*device_interface->dev_switch_high_xuga)();
    }    
    
    return -1;
}
int  cam_switch_upmid_xuga(void){
    if(device_interface&&
        device_interface->dev_switch_upmid_xuga){
        return (*device_interface->dev_switch_upmid_xuga)();
    }    
    
    return -1;
}
int  cam_switch_mid_xuga(void){
    if(device_interface&&
        device_interface->dev_switch_mid_xuga){
        return (*device_interface->dev_switch_mid_xuga)();
    }    
    
    return -1;
}
int  cam_switch_low_xuga(void)
{
    if(device_interface&&
        device_interface->dev_switch_low_xuga){
        return (*device_interface->dev_switch_low_xuga)();
    }    
    
    return -1;
}
int  cam_to_high_preview(void)
{
    if(device_interface&&
        device_interface->dev_to_high_preview){
        return (*device_interface->dev_to_high_preview)();
    }    
    
    return -1;
}
int  cam_to_low_preview(void)
{
    if(device_interface&&
        device_interface->dev_to_low_preview){
        return (*device_interface->dev_to_low_preview)();
    }    
    
    return -1;
}

//int  cam_switch_low_xuga(void){return -1;}
int  cam_close(void)
{
    if(device_interface&&
        device_interface->dev_close){
        return (*device_interface->dev_close)();
    }    
    
    return -1;
}
//-----------------------------------
//      SENSOR_SET_WHITE_BALANCE
//-----------------------------------
int  cam_set_wb(int cmd_code)
{
    if(device_interface&&
        device_interface->dev_set_wb){
        return (*device_interface->dev_set_wb)(cmd_code);
    }    
    
    return -1;
}
//-----------------------------------
//      SENSOR_SET_COLOR_EFFECT
//-----------------------------------
int  cam_set_effect(int cmd_code)
{
    if(device_interface&&
        device_interface->dev_set_effect){
        return (*device_interface->dev_set_effect)(cmd_code);
    }    
    
    return -1;
}
//-----------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------
int  cam_set_brightness(unsigned char value)
{
    if(device_interface&&
        device_interface->dev_set_brightness){
        return (*device_interface->dev_set_brightness)(value);
    }    
    
    return -1;

}

//-----------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------
int  cam_night_mode_on(void)
{
    if(device_interface&&
        device_interface->dev_night_mode_on){
        return (*device_interface->dev_night_mode_on)();
    }    
    
    return -1;
}                                                    
int  cam_night_mode_off(void)
{
    if(device_interface&&
        device_interface->dev_night_mode_off){
        return (*device_interface->dev_night_mode_off)();
    }    
    
    return -1;
}

int  cam_dump_reg(void)
{
    if(device_interface&&
        device_interface->dev_dump_reg){
        return (*device_interface->dev_dump_reg)();
    }    
    
    return -1;
}



int cam_reg_read(unsigned char addr,unsigned char reg, unsigned char *data, unsigned int size)
{
	struct i2c_adapter *adapter;
	int ret;

	struct i2c_msg msgs[] = { 
		{
			.addr   = addr,
			.flags  = 0,
			.len    = 1,
			.buf    = &reg,

		},
		{
			.addr   = addr,
			.flags  = I2C_M_RD,
			.len    = size,
			.buf    = data,
		}
	};
	adapter = i2c_get_adapter(IIC_ADAPTER);
	if (!adapter){
		return -1; 
	}
 	ret = i2c_transfer(adapter, msgs, 2);
	if ( ret != 2 ){
		return -1; 
	}

	return 0;
}

int cam_reg_write(unsigned char addr, unsigned char * data, int size)
{
	struct i2c_adapter *adapter;
	unsigned int ret;
	
	struct i2c_msg msgs[] = { 
		{
			.addr   = addr,
			.flags  = 0,
			.len            = size,
			.buf            = data,
		}
	};

	adapter = i2c_get_adapter(IIC_ADAPTER);
	if (!adapter){
		//printk(KERN_ERR "[IIC_Write]: can't get i2c adapter\n");
		return -1; 
	}
	ret = i2c_transfer(adapter, msgs, 1) ;
	if (ret!= 1){
		//printk(KERN_ERR "transfer exception %d\n",ret);
		return -1; 
	}

	return 0;
}

static inline u32 ima_cam_readl(struct ima_camif_param_t *param,  int offset)
{
	return readl(param->ioaddr + offset);
}

static  inline void ima_cam_writel(struct ima_camif_param_t *param, int offset, u32 value)
{
	writel(value, param->ioaddr + offset);
}

void cam_power_set(int mode)
{
    gpio_set_value(GPIO_CAMARA_EN,mode);
}

const struct sensor_cmd * get_cam_cmd_list(int op_code,int cmd_code)
{
    const struct sensor_op_list *sensor_op;
    void **cmd_table;
    const struct sensor_cmd *cmd;

    if(op_code >= SENSOR_OP_NUM){
        printk("%s invalid op_code %d\n",__func__,op_code);
        return NULL;
    }

    if(device_interface&&device_interface->dev_get_op_table)
        sensor_op = (*device_interface->dev_get_op_table)(op_code);
    else
        sensor_op=NULL;
    if(!sensor_op){
        printk("%s null sensor op_code %d\n",__func__,op_code);
        return NULL;
    }
    
    if(cmd_code >= sensor_op->num){
        printk("%s invalid cmd %d\n",__func__,cmd_code);
        return NULL;
    }
    cmd_table=sensor_op->cmd_table;
    if(!cmd_table){
        printk("%s null cmd list %d\n",__func__,cmd_code);
        return NULL;
    }
    cmd = (const struct sensor_cmd *)cmd_table[cmd_code];
    
    return cmd;    
}

int excute_cam_cmd_list(const struct sensor_cmd *cmd_list)
{
    unsigned char *buf;

    int ret = 0;

    if(!cmd_list || !device_interface){
        printk("%s cmd list op \n",__func__);
        return -1;
    }

    while(!(cmd_list[0].reg == (unsigned char)MAG_REG&&
        cmd_list[0].val == (unsigned char)MAG_VAL&&
        cmd_list[1].reg == (unsigned char)MAG_REG&&
        cmd_list[1].val == (unsigned char)MAG_VAL)){

        buf = (unsigned char *)&cmd_list[0];
        ret = cam_reg_write(device_interface->info->dev_addr,buf,sizeof(struct sensor_cmd));
        if(ret){
            printk("%s write 0x%x 0x%x\n",__func__,cmd_list->reg,cmd_list->val);
        }
        cmd_list++;
    }    

    return ret;
}


int excute_cam_cmd(int op_code,int cmd_code)
{
    const struct sensor_cmd *cmd_list;

    int ret;

    cmd_list = get_cam_cmd_list(op_code,cmd_code);

    ret = excute_cam_cmd_list(cmd_list);   

    return ret;
}


static int ima_cam_open(struct inode *inode, struct file *file)
{
	camif_open_count++;
	
	printk("%s %d\n",__func__,camif_open_count);
	return CAMIF_RET_OK;
}

static int ima_cam_release(struct inode *inode, struct file *file)
{
	camif_open_count--;
	
	printk("%s %d\n",__func__,camif_open_count);
	return CAMIF_RET_OK;

}

static int ima_cam_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
{
	int value;
	int ret = CAMIF_RET_OK;
    int invvs;
    //pr_debug("%s cmd =%d \n",__func__,cmd);
	
	switch(cmd)
	{
		case SENSOR_PR_WAIT_OK:
		    //pr_debug("SENSOR_PR_WAIT_OK\n");
			mutex_lock(&pr_mutex);
			pr_flag = 1;
			mutex_unlock(&pr_mutex);
			wait_event_interruptible_timeout(wait_camif, pr_flag == 0, HZ/4);
            //cam_dump_reg();
			break;
		case SENSOR_CO_WAIT_OK:
		    pr_debug("SENSOR_CO_WAIT_OK\n");
			mutex_lock(&co_mutex);
			co_flag = 1;
			mutex_unlock(&co_mutex);
			wait_event_interruptible_timeout(wait_camif, co_flag == 0, HZ/ 4);
			break;
		case SENSOR_SET_MODE:
		    pr_debug("sensor set mode\n");
			if(copy_from_user(&value, (int *)arg, sizeof(int))){
				ret = -EFAULT;
				return ret;
			}

			switch(value){
				case SET_MODE_INIT_SENSOR:
				    pr_debug("SET_MODE_INIT_SENSOR\n");
                    device_interface=get_camara_interface(&invvs);
					cam_init(invvs);
					break;
				case SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA:
				    pr_debug("SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA\n");
					cam_switch_high_svga();
					break;
				case SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA:
				    pr_debug("SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA\n");
					cam_switch_low_svga();
					break;
				case SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA:
				    pr_debug("SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA\n");
					//cam_svga_to_xuga();
					cam_switch_high_xuga();
					//cam_dump_reg();
					break;

				case SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA:
				    pr_debug("SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA\n");
					//cam_svga_to_xuga();
					cam_switch_upmid_xuga();
					break;

				case SET_MODE_SWITCH_SENSOR_TO_MID_XUGA:
				    pr_debug("SET_MODE_SWITCH_SENSOR_TO_MID_XUGA\n");
					//cam_svga_to_xuga();
					cam_switch_mid_xuga();
					break;

				case SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA:
				    pr_debug("SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA\n");
					//cam_svga_to_xuga();
					cam_switch_low_xuga();
					break;
				case SET_MODE_SENSOR_TO_HIGH_PREVIEW:
					pr_debug("SET_MODE_SENSOR_TO_HIGH_PREVIEW\n");
					cam_to_high_preview();
					break;
				case SET_MODE_SENSOR_TO_LOW_PREVIEW:
					pr_debug("SET_MODE_SENSOR_TO_LOW_PREVIEW\n");
					cam_to_low_preview();
					break;					
				case SET_MODE_CLOSE_SENSOR:
				    pr_debug("SET_MODE_CLOSE_SENSOR\n");
					pr_debug(KERN_ERR "SENSOR_CLOSE  KERNAL IOCTL\n");
					cam_close();
					device_interface=NULL;
					break;
				default: 
				    ret = -EFAULT;
					break;

			}
			break;

		case SENSOR_SET_WHITE_BALANCE:
		    pr_debug("sensor set white balance\n");
			if(copy_from_user(&value, (int *)arg, sizeof(int))){
				ret = -EFAULT;
				return ret;
			}

			switch(value){
				case SET_WB_AUTO:
				case SET_WB_CUSTOM:
				case SET_WB_INCANDESCENT:
				case SET_WB_FLUORESCENT:
				case SET_WB_DAYLIGHT:
				case SET_WB_CLOUDY:
				case SET_WB_TWILIGHT:
				case SET_WB_SHADE:
				    pr_debug("SET_WB_%d\n",value);
				    cam_set_wb(value);
					break;			
				default:
				    ret = -EFAULT;
					break;
			}
			break;
		case SENSOR_SET_COLOR_EFFECT:
		    pr_debug("set color efect\n");
			if(copy_from_user(&value, (int *)arg, sizeof(int))){
				ret = -EFAULT;
				return ret;
			}
			switch(value){
				case SET_EFFECT_OFF:
				case SET_EFFECT_MONO:
				case SET_EFFECT_NEGATIVE:
				case SET_EFFECT_SOLARIZE:
				case SET_EFFECT_PASTEL:
				case SET_EFFECT_MOSAIC:
				case SET_EFFECT_RESIZE:
				case SET_EFFECT_SEPIA:
				case SET_EFFECT_POSTERIZE:
				case SET_EFFECT_WHITEBOARD:
				case SET_EFFECT_BLACKBOARD:
				case SET_EFFECT_AQUA:	
				    pr_debug("SET_EFFECT%d\n",value);
				    cam_set_effect(value);
					break;			
				default:
				    ret = -EFAULT;
					break;
			}
			break;
		case SENSOR_SET_ANTIBANDING:
		    pr_debug("set antibanding\n");
			break;
		case SENSOR_SET_BRIGHTNESS:
		    pr_debug("set brightness\n");
		    cam_set_brightness(0);
			break;
		case SENSOR_SET_SCENCE_MODE:
		    pr_debug("set night mode\n");
			if(copy_from_user(&value, (int *)arg, sizeof(int))){
				ret = -EFAULT;
				return ret;
			}
			switch(value){
				case SET_SCENCE_AUTO:
				    pr_debug("SENSOR_SCENCE_AUTO\n");
//					cam_night_mode_off();
					break;
				case SET_SCENCE_NIGHT:
				    pr_debug("SENSOR_SCENCE_NIGHT\n");
//					cam_night_mode_on();
					break;
				default:
				    ret = -EFAULT;
					break;
			}
			break;
        case SENSOR_GET_FLAG_PHY:                                              
            pr_debug(KERN_ERR "SENSOR_GET_FLAG_PHY\n");                      
            if(copy_to_user((unsigned int *)arg, &flag_phy_addr, 4))
            {                                                       
                pr_debug("copy_to_usr error\n");                  
            }                              
            break;                                                    
        case SENSOR_SET_FLAG_DIRTY:                                            
            pr_debug(KERN_ERR "SENSOR_SET_FLAG_DIRTY\n");                      
            mutex_lock(&flag_mutex);                                  
            flag_vir_addr[0] = 0xFFFF;                                
            mutex_unlock(&flag_mutex);                                
            break;
		default:
		    ret = -EFAULT;
			break;

	}

	return ret;
}



static struct file_operations ima_cam_fops = 
{
		owner:		THIS_MODULE,
		open:		ima_cam_open,
		release:	ima_cam_release,
		ioctl:		ima_cam_ioctl,
};

static irqreturn_t ima_cam_irq_handle(int irq ,void *dev_id)
{
	u32 intmask;
	unsigned int clear_int = 0;
	irqreturn_t ret;
	intmask = readl(param->ioaddr+IMAP_CICPTSTATUS);
	////printk(KERN_ERR "get irq 0x%08x\n",intmask);
	
	if(!intmask || intmask == 0xFFFFFFFF)
	{
		ret = IRQ_NONE;
		return ret;
	}
	if(intmask & CAMIF_OVERFLOW)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= (intmask & CAMIF_OVERFLOW);
	}
	if(intmask & CAMIF_UNDERFLOW)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= (intmask & CAMIF_UNDERFLOW);
	}

	if(intmask & PRFIFO_DIRTY)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= PRFIFO_DIRTY;
	}
	if(intmask & COFIFO_DIRTY)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= COFIFO_DIRTY;
	}
	if(intmask & CERR656)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= CERR656;
	}

	if(intmask & PR_LEISURE)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= PR_LEISURE;
	}

	if(intmask & CO_LEISURE)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= CO_LEISURE;
	}
	
	if(intmask & PR_DMA_SUCCESS)
	{
		mutex_lock(&pr_mutex);
		/*	
		if(frame == 0)
		{
			do_gettimeofday(&time1);
		}
		if(frame == 29)
		{
			do_gettimeofday(&time2);
			//printk(KERN_ERR "30frame cost %ds,%ld us\n",(time2.tv_sec - time1.tv_sec) , (time2.tv_usec - time1.tv_usec));
			time1.tv_sec = time2.tv_sec = time2.tv_usec = time1.tv_usec = 0;
		}
		if( frame!=29)
			frame++;
		else frame = 0;
		*/
		if(pr_flag == 1)
		{
			wake_up_interruptible(&wait_camif);
		}
		pr_flag = 0;
		mutex_unlock(&pr_mutex);
		clear_int |= PR_DMA_SUCCESS;
	}

	if(intmask & CO_DMA_SUCCESS)
	{
		mutex_lock(&co_mutex);
		if(co_flag == 1)
		{
			wake_up_interruptible(&wait_camif);
		}
		co_flag = 0;
		mutex_unlock(&co_mutex);
		clear_int |= CO_DMA_SUCCESS;
	}

	//ima_cam_writel(param, IMAP_CICPTSTATUS, (intmask | clear_int));
	writel(intmask, param->ioaddr+IMAP_CICPTSTATUS);
	
	return IRQ_HANDLED;
}

//init camif ,config sensor
static int  cam_gpio_init(void)
{
	u32 tmp;

	/*for p1011*/
		
	tmp = readl(rDIV_CFG1);
	tmp &= ~((3 << 16) | (0x1f << 18));
	tmp |= ((2<<16) | (19<<18));
	writel(tmp, rDIV_CFG1);
	
	tmp = readl(rWP_MASK);
	tmp &= ~(0x1<15);
	writel(tmp, rWP_MASK);

	writel(0x2AAAAAA, rGPLCON);

    if(gpio_request(GPIO_CAMARA_EN, "GPIO_CAMARA_EN")){
        printk("gpio_request %d failed",GPIO_CAMARA_EN);
        return -1;
    }
    gpio_direction_output(GPIO_CAMARA_EN,CAMARA_POWER_OFF);
    
    imapx200_gpio_setpull(GPIO_CAMARA_EN, 0);

    return 0;
}

int cam_reset(struct ima_camif_param_t *param)
{
	u32 tmp;

    printk(KERN_ERR "cam_reset\n");

	writel(0x0, param->ioaddr + IMAP_CIGCTRL);

	tmp = readl(param->ioaddr+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, param->ioaddr+IMAP_CIGCTRL);
	mdelay(100);

	tmp = readl(param->ioaddr+IMAP_CIGCTRL);
	tmp &= ~(0x1 << 1);
	writel(tmp, param->ioaddr+IMAP_CIGCTRL);
	mdelay(100);

	tmp = readl(param->ioaddr+IMAP_CIGCTRL);
	tmp |= (0x1 << 1);
	writel(tmp, param->ioaddr+IMAP_CIGCTRL);

	//inserved vsync 
	/*tmp = readl(param->ioaddr+IMAP_CIGCTRL);
    tmp |= (0x1 << 4);
    tmp = readl(param->ioaddr+IMAP_CIGCTRL);*/
	return 0;
}

static int ima_cam_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;

	imap_cam_clk = NULL;
	camif_open_count = 0;

	/*initualize wait queue*/
	init_waitqueue_head(&wait_camif);
	pr_flag = co_flag = 0;
	//printk(KERN_ERR "iMAP camera interface driver probe\n");
	camif_debug("iMAP camera driver probe\n");

	param = kzalloc(sizeof(struct ima_camif_param_t), GFP_KERNEL);
	if(!param) {
		camif_error("alloc buffer failed!\n");
		return  -ENOMEM;
	}

	ret = register_chrdev(CAMIF_DEFAULT_MAJOR, "ima_camif", &ima_cam_fops);
	if(ret) {
		camif_error("imapx register chardev error!\n");
		goto out;
	}

	camif_class = class_create(THIS_MODULE, "ima_camif");
	param->dev_id = MKDEV(CAMIF_DEFAULT_MAJOR, CAMIF_DEFAULT_MINOR);
	device_create(camif_class , NULL,  param->dev_id, NULL, "imapx200-camif");
	
	param->dev = &pdev->dev;
	
	/*get clock srouce*/
	param->hclk = clk_get(&pdev->dev, "camif");
	if( IS_ERR (param->hclk)) {
		camif_error("failed to get clock\n");
		ret = -ENOENT;
		goto err_io_noclk;
	}
	clk_enable(param->hclk);

	/*get irq number*/
	param->irq = platform_get_irq(pdev,0);
	if(param->irq < 0) {
		camif_error("no irq specified\n");
		ret = param->irq;
		goto err_io_clk;
	}

	ret = request_irq(param->irq, ima_cam_irq_handle, IRQF_DISABLED,
			dev_name(&pdev->dev),param);
	if(ret)
		goto err_io_clk;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) {
		camif_error("no memory specified\n");
		ret = -ENOENT;
		goto err_io_clk;
	}

	param->phy_start = res->start;
	param->phy_size = resource_size(res);
	param->res = request_mem_region(res->start, resource_size(res),
			pdev->name);

	if(!param->res) {
		camif_error("cannot request IO\n");
		ret = -ENXIO;
		goto err_io_clk;
	}

	//printk(KERN_ERR " start = %08x,size = %d\n",res->start,resource_size(res));
	param->ioaddr = ioremap_nocache(res->start, resource_size(res));
	if(!param->ioaddr) {
		camif_error("cannot map IO\n");
		ret = -ENXIO;
		goto err_io_mem;
	}

    ret = cam_gpio_init();
    if(ret){
        camif_error("cam_gpio_init failed\n");
        goto err_io_mem;
    }
    
    ret = cam_reset(param);
    
    if(ret){
        camif_error("cam_reset failed\n");
        goto err_io_mem;
    }
    
    /* Initialize mutex */
    memset(&pr_mutex, 0x00, sizeof(struct mutex));
    memset(&co_mutex, 0x00, sizeof(struct mutex));
    memset(&flag_mutex, 0x00, sizeof(struct mutex));
    mutex_init(&pr_mutex);
    mutex_init(&co_mutex);
    mutex_init(&flag_mutex);
    //tmp = readl(param->ioaddr+IMAP_CIGCTRL);
    flag_vir_addr = (unsigned int  *)kmalloc(FLAG_SIZE, GFP_KERNEL);
    if(flag_vir_addr == NULL){                                              
        camif_error("kmalloc buffer failed\n");
        ret = - -ENOMEM;                       
        goto err_io_mem;                       
    }                                              
    memset(flag_vir_addr, 0x00, FLAG_SIZE);
    flag_phy_addr = (unsigned int)virt_to_phys((unsigned long *)flag_vir_addr);
    printk("[camif]-:flag_phy_addr = %0x\n",flag_phy_addr);     

	return CAMIF_RET_OK;		
err_io_mem:
	release_resource(param->res);
err_io_clk:
	clk_disable(param->hclk);
err_io_noclk:
	kfree(param);
out:
	camif_error("driver probe failed with err %d\n",ret);
	return ret;
}


static int ima_cam_remove(struct platform_device *pdev) 
{
	mutex_destroy(&pr_mutex);
	mutex_destroy(&co_mutex);
    mutex_destroy(&flag_mutex);

	iounmap((void *)(param->ioaddr));		
	release_mem_region(param->phy_start, param->phy_size);
	if(param->res)
	{
		release_resource(param->res);
		kfree(param->res);
		param->res = NULL;
	}

	free_irq(param->irq, pdev);
	device_destroy(camif_class, param->dev_id);
	class_destroy(camif_class);
	unregister_chrdev(CAMIF_DEFAULT_MAJOR, "ima-camif");
	clk_disable(param->hclk);


	return CAMIF_RET_OK;
}

#ifdef CONFIG_PM
static int ima_cam_suspend(struct platform_device *pdev, pm_message_t state)
{
	//cam_close();
	printk(KERN_INFO "imapx_cam_suspend -- \n");
    //need power off here
	gpio_set_value(GPIO_CAMARA_EN,CAMARA_POWER_OFF);
	
	return CAMIF_RET_OK;
}

static int ima_cam_resume(struct platform_device *pdev)
{
	printk(KERN_INFO "imapx_cam_resume ++\n");

    //need resume power here
    gpio_set_value(GPIO_CAMARA_EN,CAMARA_POWER_ON);

	cam_reset(param);
	return CAMIF_RET_OK;
}
#else 

#define	ima_cam_suspend NULL
#define	ima_cam_resume  NULL
#endif


static struct platform_driver ima_cam_driver =
{
	.probe			= ima_cam_probe,
	.remove			= ima_cam_remove,
#ifdef CONFIG_PM
	.suspend		= ima_cam_suspend,
	.resume			= ima_cam_resume,
#endif	
	.driver			=
	{
		.owner		= THIS_MODULE,
		.name 		= "ima_camif",
	},
};



/*
 * init && exit
 */
static int __init ima_cam_init(void)
{
	if(platform_driver_register(&ima_cam_driver)){
		camif_error("Failed to register IMA CAMIF driver\n");
		return -EPERM;
	}
	camif_debug("IMA CAMIF driver register OK!\n");
	
	return CAMIF_RET_OK;
}


static void __exit ima_cam_exit(void)
{
	platform_driver_unregister(&ima_cam_driver);
	camif_debug("IMA CAMIF driver unregister OK!\n");
}



module_init(ima_cam_init);
module_exit(ima_cam_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("neville of ZT");
MODULE_DESCRIPTION("IMA CAMIF DRIVER");

