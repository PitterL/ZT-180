#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/i2c/tsc2007.h>
#include <linux/irq.h>

#include <mach/imapx_gpio.h>
#include <linux/io.h>
#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define Debug
#define	MAX_12BIT	((1<<12)-1)
#define TS_POLL_DELAY	(10 * 1000)	/* ns delay before the first sample */
#define TS_POLL_PERIOD	 10	/* ns delay between samples */

static DECLARE_MUTEX(nas_mutil_mutex);

#define ON_TOUCH_INT	IRQ_EINT4


#define FT5406		1

#define SWAP_XY		1


struct risintech_ts_setup_data {
	int i2c_bus;	
	unsigned short i2c_address;
};
static struct risintech_ts_setup_data risintech_ts_setup = {
	.i2c_address = 0x58,
	.i2c_bus = 0x2,
};


#if FT5406
	#define MAX_X		1791
	#define	MAX_Y		1023
#else
	#define MAX_X		1279
	#define	MAX_Y		767
#endif

#define NOT_X		0
#define NOT_Y		1

//#define ON_TOUCH_INT IRQ_EINT(8)
//#define TOUCH_INT_PIN	S3C64XX_GPN(8)
//#define WAKE_UP_PIN	S3C64XX_GPK(0)
//#define RESET_PIN	S3C64XX_GPK(1)

#define MAX_FINGER	5

static int risintech_ts_open(struct input_dev *dev);
static void risintech_ts_close(struct input_dev *dev);
static irqreturn_t risintech_ts_isr(int irq, void *dev_id);

static struct workqueue_struct *risintech_wq;

unsigned char Filter_Finger;
unsigned char Filter_Conut;

struct nas_ts_point {
	int status;
	int x;
	int y;
	int area;
};	

struct nas_ts_priv {
	struct i2c_client *client;
	struct input_dev *input;
	//struct input_dev *input0;
	struct delayed_work nas_work;
	struct hrtimer timer;
	//struct work_struct  nas_work;
	int reported_finger_count;
	int irq;
	struct early_suspend early_suspend;
	int		(*get_pendown_state)(void);
	void		(*clear_penirq)(void);
	unsigned		pendown;
};

struct nas_ts_point risintech_points[MAX_FINGER];
struct nas_ts_point Old_points[MAX_FINGER];

//2010y 11m 01d
#ifdef CONFIG_HAS_EARLYSUSPEND
static void risintech_ts_early_suspend(struct early_suspend *h);
static void risintech_ts_late_resume(struct early_suspend *h);
#endif


#if 0
static int risintech_i2c_write(struct i2c_client *clinet, u8* msg, u8 uCnt);
static int risintech_i2c_read(struct i2c_client *clinet,u8 reg,u8* buf, u8 uCnt);

static int risintech_i2c_write(struct i2c_client *clinet, u8* msg, u8 uCnt)
{
	int ret;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_i2c_write                 |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	ret = i2c_master_send(clinet, msg, uCnt);
	
	#ifdef Debug
		if(ret<0)
			printk("		risintech_i2c_write : i2c_master_send Error=%d 0x%x\n",ret,ret);
		else		
			printk("		risintech_i2c_write : i2c_master_send OK !\n");
	#endif
	
	return ret;
}
static int risintech_i2c_read(struct i2c_client *clinet,u8 reg,u8* buf, u8 uCnt)
{
	int ret;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_i2c_read                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	ret = i2c_master_send(clinet, reg, 1);
	
	if(ret<0)
	{
		#ifdef Debug
			printk("		risintech_i2c_read : i2c_master_send Error=%d 0x%x\n",ret,ret);
		#endif
		return ret;
	}
	#ifdef Debug
	else		
		printk("		risintech_i2c_read : i2c_master_send OK !\n");
	#endif
	
	ret = i2c_master_recv(clinet, buf, uCnt);
	if(ret<0)
	{
		#ifdef Debug
			printk("		risintech_i2c_read : i2c_master_recv Error=%d 0x%x\n",ret,ret);
		#endif
		return ret;
	}
	#ifdef Debug
	else		
		printk("		risintech_i2c_read : i2c_master_recv OK !\n");
	#endif
	
	return ret;
}
#endif



static int imap_get_pendown_state(void)
{
	volatile unsigned int tmp;
	tmp = __raw_readl(rGPGDAT);
	tmp = !(tmp&0x10);
	return tmp;
}

static void imap_clear_penirq(void)
{
	volatile unsigned int tmp;
	tmp = 0xffffffff;
	__raw_writel(tmp, rEINTG4PEND);
}


static int nas_mutil_write(struct nas_ts_priv *clinet, u8 reg, u8 value)
{
	u8 data[2];
	int ret;
	//down(&nas_mutil_mutex);
	data[0] = reg;
	data[1] = value;
	
	ret = i2c_master_send(clinet->client, data, 2);
	if (ret < 0){
		printk("tsc2007_write is error!\r\n");	
	}
	
	//up(&nas_mutil_mutex);
	return ret;
}

static inline int nas_mutil_read(struct nas_ts_priv *clinet, u8 reg, u8 *buf, int len)
{
	int ret;
	u8 data = reg;
	
	//down(&nas_mutil_mutex);
	/* Set address */

	//ret = i2c_master_send(clinet->client, &data, 1);
	//if (ret < 0) 
	//{
	//   printk("		risintech_ts_work: i2c_master_send err: %d !\n", ret);	
	//goto ret_error;
	//}

	ret = i2c_master_recv(clinet->client, buf, len);
	if (ret < 0)
	{	
	   printk("		risintech_ts_work: i2c_master_recv err: ret %d buf=%d !\n",ret,buf);		
	   goto ret_error;
	}
	//buf = i2c_smbus_read_word_data(clinet->client, data);
	//if (buf < 0) 
	//{
	//	printk("		risintech_ts_work: i2c io error: %d !\n", buf);
	//	return buf;
	//}		

	//ret = buf[0];
	//printk("nas_mutil_xfer : buf[0] = 0x%x\r\n",buf[0]);
	
ret_error:
	//up(&nas_mutil_mutex);
	return ret;
}

// config interrupt for GPO2
static void config_gpio(void)
{
	unsigned long rEINTCON_value, rEINTFLTCON0_value;

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

    /*
    if (gpio_request(IMAPX200_GPE(10), "nas_touch")) {
        printk("motor is error\r\n");
        return ;
    }
    // reset
    gpio_direction_output(IMAPX200_GPE(10),1);*/
}


static void risintech_ts_work(struct work_struct *work)
{

	unsigned short xpos0, ypos0;	
	unsigned short xpos1, ypos1;	
	unsigned short xpos2, ypos2;	
	unsigned short xpos3, ypos3;	
	unsigned short xpos4, ypos4;

		
	//unsigned char event;
	unsigned char Finger;
	unsigned char buf[8];
	//struct i2c_msg msg[2];
	int ret;
	//uint8_t start_reg;
	
	struct nas_ts_priv *nas_priv = container_of(work,struct nas_ts_priv,nas_work);
	
	
	if (nas_priv->get_pendown_state) {
       if (unlikely(!nas_priv->get_pendown_state())) {
	       nas_priv->pendown = false;	   
	       goto out;
	   }
   }else{
        nas_priv->pendown = false;
        goto out;
   }
	
	
	#ifdef Debug
	printk("+-----------------------------------------+\n");
	printk("|	risintech_ts_work!                    |\n");
	printk("+-----------------------------------------+\n");
	#endif
	
	memset(buf, 0, sizeof(buf));
		

        ret = nas_mutil_read(nas_priv,0xF9,buf,8);

	
	#ifdef Debug
		printk("		risintech_ts_work: i2c_transfer ret = 0x%x %d\n" , ret,ret);
	#endif
	
	if(ret<0)
	{
		#ifdef Debug
		//printk("		risintech_ts_work: i2c_transfer ret = 0x%x %d\n" , ret,ret);
		printk("		risintech_ts_work: i2c_transfer Error !\n");
		#endif
		goto out;
	}
	
	else
	{
		#ifdef Debug
		//printk("		risintech_ts_work: i2c_transfer ret = 0x%x %d\n" , ret,ret);
		printk("		risintech_ts_work: i2c_transfer OK !\n");
		#endif
	}
	     

	#ifdef Debug
		printk("		risintech_ts_work: buf[0] = 0x%x\n" ,buf[0]);
		printk("		risintech_ts_work: buf[1] = 0x%x\n" ,buf[1]);
		printk("		risintech_ts_work: buf[2] = 0x%x\n" ,buf[2]);
		printk("		risintech_ts_work: buf[3] = 0x%x\n" ,buf[3]);
		printk("		risintech_ts_work: buf[4] = 0x%x\n" ,buf[4]);
		printk("		risintech_ts_work: buf[5] = 0x%x\n" ,buf[5]);
		printk("		risintech_ts_work: buf[6] = 0x%x\n" ,buf[6]);
		printk("		risintech_ts_work: buf[7] = 0x%x\n" ,buf[7]);		
		printk("\n");
	#endif

    
	//initialize points
	{	
		int i;
		
		xpos0=0;	ypos0=0;
		xpos1=0;	ypos1=0;
		xpos2=0;	ypos2=0;
		xpos3=0;	ypos3=0;
		xpos4=0;	ypos4=0;

		for(i=0;i<MAX_FINGER;i++)
		{
			risintech_points[i].x=0;
			risintech_points[i].y=0;
			risintech_points[i].status=0;
		}
	}
			
	unsigned char i;
	unsigned char index;
	unsigned short xpos, ypos;


	xpos = ((int)buf[1])+ ((int)(buf[2]<<7));
        ypos = ((int)buf[3])+ ((int)(buf[4]<<7));

	//check limitation
	if(xpos > MAX_12BIT) xpos = MAX_12BIT;
	if(ypos > MAX_12BIT) ypos = MAX_12BIT;
	if(xpos<=0)xpos=0;
	if(ypos<=0)ypos=0;
	   
    risintech_points[0].status = buf[0]>>6; //left
	risintech_points[0].x = xpos;
	risintech_points[0].y = ypos;

	#ifdef Debug			
	printk("		risintech_ts_work: X0 = %d , Y0 = %d\n",risintech_points[0].x,risintech_points[0].y);					
	printk("		risintech_ts_work: Status 0 = %d \n",risintech_points[0].status);
	printk("\n");	
	#endif
		
    if(!nas_priv->pendown) {
        nas_priv->pendown = true;
        input_report_key(nas_priv->input, BTN_TOUCH, 1);
    }
    /*
    if (!nas_priv->get_pendown_state && nas_priv->pendown) {	  
        nas_priv->pendown = false;
    }*/

    printk("DOWN\n");
    input_report_abs(nas_priv->input, ABS_X, risintech_points[0].x);
	input_report_abs(nas_priv->input, ABS_Y, risintech_points[0].y);
    input_report_abs(nas_priv->input, ABS_PRESSURE, 100);	
    input_sync(nas_priv->input);

    /*
	if(nas_priv->pendown){
	    printk("DOWN\n");
            input_report_key(nas_priv->input, BTN_TOUCH, 1);
            input_report_abs(nas_priv->input, ABS_PRESSURE, 100);	
	}
	else
        {
            printk("UP\n");
            input_report_key(nas_priv->input, BTN_TOUCH, 0);
            input_report_abs(nas_priv->input, ABS_PRESSURE, 0);		
        }
	input_sync(nas_priv->input);
    */
out:
    if(!nas_priv->pendown){
        printk("UP\n");
        input_report_key(nas_priv->input, BTN_TOUCH, 0);
        input_report_abs(nas_priv->input, ABS_PRESSURE, 0);
        input_sync(nas_priv->input);
    }else{
	   schedule_delayed_work(&nas_priv->nas_work,msecs_to_jiffies(TS_POLL_PERIOD));
    }
    return;
	

}

static enum hrtimer_restart risintech_ts_timer(struct hrtimer *timer)
{
	struct nas_ts_priv *nas_priv = container_of(timer, struct nas_ts_priv, timer);
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_timer!                   |\n");
		printk("+-----------------------------------------+\n");
	#endif
	queue_work(risintech_wq, &nas_priv->nas_work);
	
	hrtimer_start(&nas_priv->timer, ktime_set(0, TS_POLL_PERIOD), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static int risintech_ts_probe(struct i2c_client *client,const struct i2c_device_id *idp)
{
	struct nas_ts_priv *nas_priv;
	struct input_dev *nas_input;
	//struct input_dev *nas_input0;
	int error;

	#ifdef Debug
	printk("+-----------------------------------------+\n");
	printk("|	risintech_ts_probe!                   |\n");
	printk("+-----------------------------------------+\n");
	#endif
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		#ifdef Debug
		printk("		risintech_ts_probe: need I2C_FUNC_I2C\n");
		#endif
		return -ENODEV;
	}
	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_READ_WORD_DATA))
	{
        #ifdef Debug
		printk("		risintech_ts_probe: need I2C_FUNC_SMBUS_READ_WORD_DATA\n");
	    #endif
		return -EIO;
	}
	else
	{
	#ifdef Debug	
		printk("		risintech_ts_probe: i2c Check OK!\n");
		printk("		risintech_ts_probe: i2c_client name : %s\n",client->name);
		printk("		risintech_ts_probe: i2c_client addr : 0x%x\n",client->addr);
	#endif
	}
	
	
	nas_priv = kzalloc(sizeof(*nas_priv), GFP_KERNEL);
	if (!nas_priv)
	{
		#ifdef Debug
		printk("		risintech_ts_probe: kzalloc Error!\n");
		#endif
		error=-ENODEV;
		goto	err0;
		//return -ENODEV;
	}
	
	else
	{
		#ifdef Debug
		printk("		risintech_ts_probe: kzalloc OK!\n");
	    #endif	
	}
	
	
	dev_set_drvdata(&client->dev, nas_priv);
	nas_priv->client = client;
	i2c_set_clientdata(client, nas_priv);
	
	nas_input = input_allocate_device();
	if (!nas_input)
	{
		#ifdef Debug
		printk("		risintech_ts_probe: input_allocate_device Error\n");
		#endif
		error=-ENODEV;
		goto	err1_1;
		//return -ENODEV;
	}
	
	else
	{
		#ifdef Debug
		printk("		risintech_ts_probe: input_allocate_device OK\n");
	        #endif
	}
	
	


	nas_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);    // | BIT_MASK(EV_SYN) ;
	nas_input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH); // | BIT_MASK(BTN_2);
	
	//__set_bit(EV_ABS, nas_input->evbit);
	//__set_bit(EV_KEY, nas_input->evbit);
	//__set_bit(BTN_TOUCH, nas_input->keybit);
	

	input_set_abs_params(nas_input, ABS_X, 0,MAX_12BIT, 0, 0);//設定絕對座標的最大值與最小值
	input_set_abs_params(nas_input, ABS_Y, 0, MAX_12BIT, 0, 0);//設定絕對座標的最大值與最小值	
	input_set_abs_params(nas_input, ABS_PRESSURE, 0, MAX_12BIT, 0, 0);
	
	nas_input->name = client->name;
	nas_input->id.bustype = BUS_I2C;
	nas_input->dev.parent = &client->dev;
	//nas_input->open = risintech_ts_open;
	//nas_input->close = risintech_ts_close;
	input_set_drvdata(nas_input, nas_priv);
	
	nas_priv->client = client;
	nas_priv->input = nas_input;
	

	
        nas_priv->get_pendown_state = imap_get_pendown_state;
	nas_priv->clear_penirq      = imap_clear_penirq;


	//INIT_WORK(&nas_priv->nas_work, risintech_ts_work);
	INIT_DELAYED_WORK(&nas_priv->nas_work, risintech_ts_work);
	
	//hrtimer_init(&nas_priv->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	//nas_priv->timer.function = risintech_ts_timer;
	
	nas_priv->irq=ON_TOUCH_INT;
	#ifdef Debug
		printk("		risintech_ts_probe: nas_priv->irq : %d\n",nas_priv->irq);
	#endif
	
	error = input_register_device(nas_input);
	if(error)
	{
		#ifdef Debug
			printk("		risintech_ts_probe: input_register_device input Error!\n");
		#endif
		
		error=-ENODEV;
		goto	err1;
	}
	#ifdef Debug
	else
	{
		
		printk("		risintech_ts_probe: input_register_device input OK!\n");
		
	}
	#endif
	
	#ifdef Debug
		printk("		risintech_ts_probe: input_id: %d\n",&nas_input->id);
	#endif

	config_gpio();


	//WAKE_UP_PIN
	
	/*error = gpio_request(WAKE_UP_PIN,"GPK");
	if(error)
	{
		#ifdef Debug
			printk("		risintech_ts_probe: WAKE_UP_PIN gpio_request Error!\n");
		#endif
	}
	else
	{
		#ifdef Debug
			printk("		risintech_ts_probe: WAKE_UP_PIN gpio_request OK!\n");
		#endif
		gpio_direction_output(WAKE_UP_PIN, 1);//GPIO Output Hight
	}
	mdelay(10);
	*/
	//RESET_PIN
	/*error = gpio_request(RESET_PIN,"GPK");
	if(error)
	{
		#ifdef Debug
			printk("		risintech_ts_probe: RESET_PIN gpio_request Error!\n");
		#endif
	}
	else
	{
		#ifdef Debug
			printk("		risintech_ts_probe: RESET_PIN gpio_request OK!\n");
		#endif
		gpio_direction_output(RESET_PIN, 1);//GPIO Output Hight
		mdelay(10);
		gpio_direction_output(RESET_PIN, 0);//GPIO Output Hight
		udelay(10);
		gpio_direction_output(RESET_PIN, 1);//GPIO Output Hight
		mdelay(20);
	}
	*/
	//TOUCH_INT_PIN
	/*error = gpio_request(TOUCH_INT_PIN,"GPN");
	if(error)
	{
		#ifdef Debug
			printk("		risintech_ts_probe: TOUCH_INT_PIN gpio_request Error!\n");
		#endif
	}
	else
	{
		#ifdef Debug
			printk("		risintech_ts_probe: TOUCH_INT_PIN gpio_request OK!\n");
		#endif
		s3c_gpio_cfgpin(TOUCH_INT_PIN, S3C_GPIO_SFN(0x0F));
		s3c_gpio_setpull(TOUCH_INT_PIN, S3C_GPIO_PULL_NONE);
		
		//gpio_direction_output(TOUCH_INT_PIN, 1);//GPIO Output Hight
	}*/
	//申請中斷
	error = request_irq(/*ON_TOUCH_INT*/nas_priv->irq, risintech_ts_isr,IRQF_DISABLED|IRQF_TRIGGER_FALLING, client->name, nas_priv);
	if (error)
	{
		#ifdef Debug
		printk("		risintech_ts_probe: request_irq Error!\n");
		#endif
		error=-ENODEV;
		goto err2;
	}	
	else
	{
		#ifdef Debug
		printk("		risintech_ts_probe: request_irq OK!\n");
		#endif
	
	}
	
//2010y 11m 01d	
#ifdef CONFIG_HAS_EARLYSUSPEND
	nas_priv->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	nas_priv->early_suspend.suspend = risintech_ts_early_suspend;
	nas_priv->early_suspend.resume = risintech_ts_late_resume;
	register_early_suspend(&nas_priv->early_suspend);
#endif

	Filter_Finger=0xFF;
	Filter_Conut=0;
	{
		int i;
		for(i=0;i<MAX_FINGER;i++)
		{
			Old_points[i].x=0xFFFF;
			Old_points[i].y=0xFFFF;
		}
	}
    
    printk("		risintech_ts_probe: request_irq OK!\n");
	
	return 0;
err2:
	input_unregister_device(nas_input);
	//nas_input = NULL; /* so we dont try to free it below */
err1:
	//input_free_device(nas_input0);
	hrtimer_cancel(&nas_priv->timer);
err1_1:
	input_free_device(nas_input);
	kfree(nas_priv);

err0:
	dev_set_drvdata(&client->dev, NULL);
	return error;
}
static int risintech_ts_remove(struct i2c_client *client)
{
	struct nas_ts_priv *nas_priv = dev_get_drvdata(&client->dev);
	unregister_early_suspend(&nas_priv->early_suspend);
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_remove !               |\n");
		printk("+-----------------------------------------+\n");
	#endif

	nas_priv->irq=ON_TOUCH_INT;
	free_irq(nas_priv->irq, nas_priv);//釋放中斷
	input_unregister_device(nas_priv->input);
        
        hrtimer_cancel(&nas_priv->timer);
	
	kfree(nas_priv);

	dev_set_drvdata(&client->dev, NULL);

	return 0;
}
static int risintech_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	//u8	msg[4];
	//int	error;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_suspend                |\n");
		printk("+-----------------------------------------+\n");
	#endif
	/*
	msg[0]=0xFC;
	msg[1]=0x3A;
	msg[2]=0x03;
	msg[3]=0xC5;
	
	error=risintech_i2c_write(client,msg,4);
	
	#ifdef Debug	
		if(error<0)
		{
				printk("|	risintech_ts_suspend: Command Error\n");
				printk("|	risintech_ts_suspend: error = 0x%x\n",error);
		}
		else
		{
				printk("|	risintech_ts_suspend: Command OK\n");
				printk("|	risintech_ts_suspend: error = 0x%x\n",error);
		}
	#endif
	*/
	
	return 0;
}
static int risintech_ts_resume(struct i2c_client *client)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_resume                |\n");
		printk("+-----------------------------------------+\n");
	#endif
	//gpio_direction_output(WAKE_UP_PIN, 0);
	//mdelay(5);
	//gpio_direction_output(WAKE_UP_PIN, 1);
	return 0;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
static void risintech_ts_early_suspend(struct early_suspend *h)
{
	struct nas_ts_priv *ts;
	ts = container_of(h, struct nas_ts_priv, early_suspend);
	risintech_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void risintech_ts_late_resume(struct early_suspend *h)
{
	struct nas_ts_priv *ts;
	ts = container_of(h, struct nas_ts_priv, early_suspend);
	risintech_ts_resume(ts->client);
}
#endif

static int risintech_ts_open(struct input_dev *dev)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_open!                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	return 0;
}
static void risintech_ts_close(struct input_dev *dev)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_close!                 |\n");
		printk("+-----------------------------------------+\n");
	#endif
}
static irqreturn_t risintech_ts_isr(int irq, void *dev_id)
{
	struct nas_ts_priv *nas_priv = dev_id;
	int ret;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_isr!                     |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
		
	
	printk("|	risintech_ts_isr---0                   |\n");

	//disable_irq(ON_TOUCH_INT);//關避中斷
	//disable_irq_nosync(ON_TOUCH_INT);
	//imap_clear_penirq();
  
	printk("|	risintech_ts_isr---1                   |\n");

	
	
	if (!nas_priv->get_pendown_state || likely(nas_priv->get_pendown_state())) 
	{
             printk("|	risintech_ts_isr---schedule_delayed_work                   |\n");
	    
	    //hrtimer_start(&nas_priv->timer, ktime_set(0, TS_POLL_DELAY),HRTIMER_MODE_REL);
	    //ret=queue_work(risintech_wq, &nas_priv->nas_work);
	    //ret=schedule_work(&nas_priv->nas_work);
	    //ret=schedule_delayed_work(&nas_priv->nas_work, msecs_to_jiffies(TS_POLL_PERIOD));
      
            ret=schedule_delayed_work(&nas_priv->nas_work, msecs_to_jiffies(TS_POLL_PERIOD));
            
            if(ret)
	    {
		     #ifdef Debug
		     printk("		risintech_ts_isr: queue_work non-zero otherwise!\n");
		     #endif
	    }
	    else
	    {
	             #ifdef Debug
		      printk("		risintech_ts_isr: queue_work work was already on a queue!\n");
		      #endif
	    }
        }
	else
	   printk("|	risintech_ts_isr---ts->get_pendown_state                  |\n");



	
	
	return IRQ_HANDLED;
}

static const struct i2c_device_id risintech_ts_id[] = {
	{ "risintech-ts", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, risintech_ts_id);

static struct i2c_driver risintech_ts_driver = {
	.driver = {
		.name = "risintech-ts",
	},
	.probe = risintech_ts_probe,
	.remove = risintech_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND	
	.suspend	= risintech_ts_suspend,
	.resume		= risintech_ts_resume,
#endif
	.id_table = risintech_ts_id,
};

static char banner[] __initdata = KERN_INFO "risintech Touchscreen driver, (c) 2010 nas Technologies Corp. \n";
static int __init risintech_ts_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int ret;

	#ifdef Debug	
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_init!                  |\n");
		printk("+-----------------------------------------+\n");
		printk(banner);
	#endif
	
	printk(banner);
	
	risintech_wq = create_singlethread_workqueue("risintech_wq");
	if (!risintech_wq)
	{
		#ifdef Debug
		printk("		risintech_ts_init: create_singlethread_workqueue Error!\n");
		#endif
		return -ENOMEM;
	}
	#ifdef Debug
	else
	{
		printk("		risintech_ts_init: create_singlethread_workqueue OK!\n");
	}
	#endif
	
	ret=i2c_add_driver(&risintech_ts_driver);//註冊 i2c_Driver
	if(ret)
	{
		#ifdef Debug
		printk("		risintech_ts_init: i2c_add_driver Error! \n");
		#endif
		//return ret;
	}
	else
	{
		printk("		risintech_ts_init: i2c_add_driver OK! \n");
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = risintech_ts_setup.i2c_address;
	strlcpy(info.type, "risintech-ts", I2C_NAME_SIZE);

	adapter = i2c_get_adapter(risintech_ts_setup.i2c_bus);
	if (!adapter) {
		printk("risintech_ts_init : can't get i2c adapter %d\n",
			risintech_ts_setup.i2c_bus);
		goto err_driver;
	}
	client = i2c_new_device(adapter, &info);

	i2c_put_adapter(adapter);
	if (!client) {
		printk("risintech_ts_init : can't add i2c device at 0x%x\n",
			(unsigned int)info.addr);
		goto err_driver;
	}

	printk("...... success\r\n");
	return ret;
err_driver:
	i2c_del_driver(&risintech_ts_driver);
	return -ENODEV;
}

static void __exit risintech_ts_exit(void)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	risintech_ts_exit!                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	i2c_del_driver(&risintech_ts_driver);//註銷 i2c_Driver
	if (risintech_wq)
		destroy_workqueue(risintech_wq);
}

module_init(risintech_ts_init);
module_exit(risintech_ts_exit);

MODULE_AUTHOR("Risintech");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Risintech Touchscreen Driver");
