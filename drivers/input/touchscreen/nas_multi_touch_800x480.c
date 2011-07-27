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


static DECLARE_MUTEX(nas_mutil_mutex);

#define ON_TOUCH_INT	IRQ_EINT4


#define FT5406		1

#define SWAP_XY		1

//mg add 0415
unsigned short gamma600[]={
0 ,   0 ,   0 ,   0 ,
 0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   1 ,   1 ,
 1 ,   1 ,   2 ,   2 ,   2 ,   2 ,   3 ,   3 ,
 3 ,   3 ,   4 ,   4 ,   4 ,   5 ,   5 ,   6 ,
 6 ,   6 ,   7 ,   7 ,   8 ,   8 ,   8 ,   9 ,
 9 ,   10 ,   10 ,   11 ,   11 ,   12 ,   12 ,   13 ,
 13 ,   14 ,   14 ,   15 ,   15 ,   16 ,   16 ,   17 ,
 18 ,   18 ,   19 ,   19 ,   20 ,   20 ,   21 ,   22 ,
 22 ,   23 ,   24 ,   24 ,   25 ,   26 ,   26 ,   27 ,
 28 ,   28 ,   29 ,   30 ,   30 ,   31 ,   32 ,   33 ,
 33 ,   34 ,   35 ,   36 ,   36 ,   37 ,   38 ,   39 ,
 40 ,   40 ,   41 ,   42 ,   43 ,   44 ,   44 ,   45 ,
 46 ,   47 ,   48 ,   49 ,   50 ,   50 ,   51 ,   52 ,
 53 ,   54 ,   55 ,   56 ,   57 ,   58 ,   59 ,   59 ,
 60 ,   61 ,   62 ,   63 ,   64 ,   65 ,   66 ,   67 ,
 68 ,   69 ,   70 ,   71 ,   72 ,   73 ,   74 ,   75 ,
 76 ,   77 ,   78 ,   79 ,   80 ,   81 ,   82 ,   84 ,
 85 ,   86 ,   87 ,   88 ,   89 ,   90 ,   91 ,   92 ,
 93 ,   94 ,   96 ,   97 ,   98 ,   99 ,   100 ,   101 ,
 102 ,   104 ,   105 ,   106 ,   107 ,   108 ,   110 ,   111 ,
 112 ,   113 ,   114 ,   116 ,   117 ,   118 ,   119 ,   120 ,
 122 ,   123 ,   124 ,   125 ,   127 ,   128 ,   129 ,   131 ,
 132 ,   133 ,   134 ,   136 ,   137 ,   138 ,   140 ,   141 ,
 142 ,   144 ,   145 ,   146 ,   148 ,   149 ,   150 ,   152 ,
 153 ,   154 ,   156 ,   157 ,   158 ,   160 ,   161 ,   163 ,
 164 ,   165 ,   167 ,   168 ,   170 ,   171 ,   172 ,   174 ,
 175 ,   177 ,   178 ,   180 ,   181 ,   183 ,   184 ,   185 ,
 187 ,   188 ,   190 ,   191 ,   193 ,   194 ,   196 ,   197 ,
 199 ,   200 ,   202 ,   203 ,   205 ,   206 ,   208 ,   210 ,
 211 ,   213 ,   214 ,   216 ,   217 ,   219 ,   220 ,   222 ,
 224 ,   225 ,   227 ,   228 ,   230 ,   232 ,   233 ,   235 ,
 236 ,   238 ,   240 ,   241 ,   243 ,   245 ,   246 ,   248 ,
 250 ,   251 ,   253 ,   255 ,   256 ,   258 ,   260 ,   261
};


struct nastech_ts_setup_data {
	int i2c_bus;	
	unsigned short i2c_address;
};
static struct nastech_ts_setup_data nastech_ts_setup = {
	.i2c_address = 0x38,
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

static int nastech_ts_open(struct input_dev *dev);
static void nastech_ts_close(struct input_dev *dev);
static irqreturn_t nastech_ts_isr(int irq, void *dev_id);

static struct workqueue_struct *nastech_wq;

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
	//struct delayed_work work;
	struct hrtimer timer;
	struct work_struct  nas_work;
	int reported_finger_count;
	int irq;
	struct early_suspend early_suspend;
};

struct nas_ts_point nastech_points[MAX_FINGER];
struct nas_ts_point Old_points[MAX_FINGER];

//2010y 11m 01d
#ifdef CONFIG_HAS_EARLYSUSPEND
static void nastech_ts_early_suspend(struct early_suspend *h);
static void nastech_ts_late_resume(struct early_suspend *h);
#endif


#if 0
static int nastech_i2c_write(struct i2c_client *clinet, u8* msg, u8 uCnt);
static int nastech_i2c_read(struct i2c_client *clinet,u8 reg,u8* buf, u8 uCnt);

static int nastech_i2c_write(struct i2c_client *clinet, u8* msg, u8 uCnt)
{
	int ret;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_i2c_write                 |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	ret = i2c_master_send(clinet, msg, uCnt);
	
	#ifdef Debug
		if(ret<0)
			printk("		nastech_i2c_write : i2c_master_send Error=%d 0x%x\n",ret,ret);
		else		
			printk("		nastech_i2c_write : i2c_master_send OK !\n");
	#endif
	
	return ret;
}
static int nastech_i2c_read(struct i2c_client *clinet,u8 reg,u8* buf, u8 uCnt)
{
	int ret;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_i2c_read                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	ret = i2c_master_send(clinet, reg, 1);
	
	if(ret<0)
	{
		#ifdef Debug
			printk("		nastech_i2c_read : i2c_master_send Error=%d 0x%x\n",ret,ret);
		#endif
		return ret;
	}
	#ifdef Debug
	else		
		printk("		nastech_i2c_read : i2c_master_send OK !\n");
	#endif
	
	ret = i2c_master_recv(clinet, buf, uCnt);
	if(ret<0)
	{
		#ifdef Debug
			printk("		nastech_i2c_read : i2c_master_recv Error=%d 0x%x\n",ret,ret);
		#endif
		return ret;
	}
	#ifdef Debug
	else		
		printk("		nastech_i2c_read : i2c_master_recv OK !\n");
	#endif
	
	return ret;
}
#endif



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
	ret = i2c_master_send(clinet->client, &data, 1);
	if (ret < 0) 
		goto ret_error;
	ret = i2c_master_recv(clinet->client, buf, len);
	if (ret < 0)
		goto ret_error;
		
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


static void nastech_ts_work(struct work_struct *work)
{
	//unsigned short xpos, ypos;
	//unsigned short xpos0, ypos0;
	
	unsigned short xpos0, ypos0;
	unsigned short xpos1, ypos1;
	unsigned short xpos2, ypos2;
	unsigned short xpos3, ypos3;
	unsigned short xpos4, ypos4;
	
	
	
	//unsigned char event;
	unsigned char Finger;
	unsigned char buf[27];
	//struct i2c_msg msg[2];
	int ret;
	//uint8_t start_reg;
	
	struct nas_ts_priv *nas_priv = container_of(work,struct nas_ts_priv,nas_work);
	
	#ifdef Debug
	printk("+-----------------------------------------+\n");
	printk("|	nastech_ts_work!                  |\n");
	printk("+-----------------------------------------+\n");
	#endif
	
	memset(buf, 0xFF, sizeof(buf));
	
	//buf[0]=0xF9;
	/*start_reg = 0xF9;
	msg[0].addr = 0x38;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &start_reg;

	msg[1].addr = 0x38;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 26;
	msg[1].buf = buf;
	
	ret = i2c_transfer(nas_priv->client->adapter, msg, 2);*/
    ret = nas_mutil_read(nas_priv,0xF9,buf,26);
	
	#ifdef Debug
		printk("		nastech_ts_work: i2c_transfer ret = 0x%x %d\n" , ret,ret);
	#endif
	
	if(ret<0)
	{
		#ifdef Debug
		//printk("		nastech_ts_work: i2c_transfer ret = 0x%x %d\n" , ret,ret);
		printk("		nastech_ts_work: i2c_transfer Error !\n");
		#endif
		goto out;
	}
	#ifdef Debug
	else
	{
		
		//printk("		nastech_ts_work: i2c_transfer ret = 0x%x %d\n" , ret,ret);
		printk("		nastech_ts_work: i2c_transfer OK !\n");
		
	}
	#endif
	/*{
		printk("		nastech_ts_work: buf[0] = 0x%x\n" ,buf[0]);
		printk("		nastech_ts_work: buf[1] = 0x%x\n" ,buf[1]);
		printk("		nastech_ts_work: buf[2] = 0x%x\n" ,buf[2]);
		printk("		nastech_ts_work: buf[3] = 0x%x\n" ,buf[3]);
		printk("		nastech_ts_work: buf[4] = 0x%x\n" ,buf[4]);
		printk("		nastech_ts_work: buf[5] = 0x%x\n" ,buf[5]);
		printk("		nastech_ts_work: buf[6] = 0x%x\n" ,buf[6]);
		printk("		nastech_ts_work: buf[7] = 0x%x\n" ,buf[7]);
		printk("		nastech_ts_work: buf[8] = 0x%x\n" ,buf[8]);
		printk("		nastech_ts_work: buf[9] = 0x%x\n" ,buf[9]);
		printk("		nastech_ts_work: buf[10] = 0x%x\n" ,buf[10]);
		printk("		nastech_ts_work: buf[11] = 0x%x\n" ,buf[11]);
		printk("		nastech_ts_work: buf[12] = 0x%x\n" ,buf[12]);
		printk("		nastech_ts_work: buf[13] = 0x%x\n" ,buf[13]);
		printk("		nastech_ts_work: buf[14] = 0x%x\n" ,buf[14]);
		printk("		nastech_ts_work: buf[15] = 0x%x\n" ,buf[15]);
		printk("		nastech_ts_work: buf[16] = 0x%x\n" ,buf[16]);
		printk("		nastech_ts_work: buf[17] = 0x%x\n" ,buf[17]);
		printk("		nastech_ts_work: buf[18] = 0x%x\n" ,buf[18]);
		printk("		nastech_ts_work: buf[19] = 0x%x\n" ,buf[19]);
		printk("		nastech_ts_work: buf[20] = 0x%x\n" ,buf[20]);
		printk("		nastech_ts_work: buf[21] = 0x%x\n" ,buf[21]);
		printk("		nastech_ts_work: buf[22] = 0x%x\n" ,buf[22]);
		printk("		nastech_ts_work: buf[23] = 0x%x\n" ,buf[23]);
		printk("		nastech_ts_work: buf[24] = 0x%x\n" ,buf[24]);
		printk("		nastech_ts_work: buf[25] = 0x%x\n" ,buf[25]);
		printk("\n");
	}*/
	Finger=buf[3];
	if(Finger==0xFF)
		Finger=0;

	#ifdef Debug
		printk("		nastech_ts_work: Finger : %d\n",Finger);
	#endif
	
	//goto out;
	
	{	
		int i;
		
		xpos0=0xFFFF;	ypos0=0xFFFF;
		xpos1=0xFFFF;	ypos1=0xFFFF;
		xpos2=0xFFFF;	ypos2=0xFFFF;
		xpos3=0xFFFF;	ypos3=0xFFFF;
		xpos4=0xFFFF;	ypos4=0xFFFF;

		for(i=0;i<MAX_FINGER;i++)
		{
			nastech_points[i].x=0xFFFF;
			nastech_points[i].y=0xFFFF;
			nastech_points[i].status=0;
		}
	}
	
	#if FT5406
		{
			unsigned char i;
			unsigned char id;
			unsigned char index;
			unsigned short xpos, ypos;
			unsigned char event;
			
			if(Finger!=0xFF && Finger>0)
			{
				//for(i=0;i<5;i++)
				//for(i=0;i<Finger;i++)
				for(i=0;i<MAX_FINGER;i++)
				{
					index=(i*4)+7;
					id=(buf[index]&0xF0)>>4;
					//if(id>=Finger)
					if(id==0x0F)
						continue;

					index=(i*4)+5;
					xpos=(unsigned short)(buf[index]*0x100);
					xpos=xpos|buf[index+1];
					if(xpos!=0xFFFF)
						xpos=xpos&0x0FFF;
						
					event=(buf[index]&0xC0)>>6;
					
					index=(i*4)+7;				
					ypos=(unsigned short)(buf[index]*0x100);
					ypos=ypos|buf[index+1];
					if(ypos!=0xFFFF)
						ypos=ypos&0x0FFF;
	
					#if SWAP_XY
					{
						unsigned short tmp;
						tmp=xpos;
						xpos=ypos;
						ypos=tmp;
					}
					#endif
					
					#if NOT_X
						xpos=MAX_X-xpos;
					#endif
					
					#if NOT_Y
						ypos=MAX_Y-ypos;
					#endif
					
					nastech_points[id].x=xpos;
					nastech_points[id].y=ypos;
					if(event==0x01 || event==0x03)
						nastech_points[id].status=0;
					else
						nastech_points[id].status=1;
					
					
					if(id==0)
					{
						xpos0=xpos;
						ypos0=ypos;
					}
					else if(id==1)
					{
						xpos1=xpos;
						ypos1=ypos;
					}
					else if(id==2)
					{
						xpos2=xpos;
						ypos2=ypos;
					}
					else if(id==3)
					{
						xpos3=xpos;
						ypos3=ypos;
					}
					else //if(id==4)
					{
						xpos4=xpos;
						ypos4=ypos;
					}
				}
			}
		}
	#else
		{
			unsigned char i;
			//unsigned char id;
			unsigned char index;
			unsigned short xpos, ypos;
			
			for(i=0;i<Finger;i++)
			{
				index=(i*4)+5;
				xpos=(unsigned short)(buf[index]*0x100);
				xpos=xpos|buf[index+1];
				
				index=(i*4)+7;
				ypos=(unsigned short)(buf[index]*0x100);
				ypos=ypos|buf[index+1];
			//}
				#if SWAP_XY
				{
					unsigned short tmp;
					tmp=xpos;
					xpos=ypos;
					ypos=tmp;
				}
				#endif
				
				#if NOT_X
					xpos=MAX_X-xpos;
				#endif			
				
				#if NOT_Y
					ypos=MAX_Y-ypos;
				#endif
					
				if(i==0)
				{
					xpos0=xpos;
					ypos0=ypos;
				}
				else if(i==1)
				{
					xpos1=xpos;
					ypos1=ypos;
				}
				else if(i==2)
				{
					xpos2=xpos;
					ypos2=ypos;
				}
				else if(i==3)
				{
					xpos3=xpos;
					ypos3=ypos;
				}
				else //if(i==4)
				{
					xpos4=xpos;
					ypos4=ypos;
				}
			}
		}
	#endif

	//xpos=(unsigned short)(buf[7]*0x100);
	//xpos=xpos|buf[8];
	
	//ypos=(unsigned short)(buf[5]*0x100);
	//ypos=ypos|buf[6];

	//if(buf[11]!=0xFF)
	//	buf[11]&=0x0F;
	//xpos0=(unsigned short)(buf[11]*0x100);
	//xpos0=xpos0|buf[12];
	//ypos0=(unsigned short)(buf[9]*0x100);
	//ypos0=ypos0|buf[10];


	#ifdef Debug	
	
		if(Filter_Finger!=Finger)
		{
			Filter_Finger=Finger;
			//printk("		nastech_ts_work: X = 0x%x , Y = 0x%x\n",xpos,ypos);
			//printk("		nastech_ts_work: X = %d , Y = %d\n",xpos,ypos);		
			//printk("		nastech_ts_work: X0 = 0x%x , Y0 = 0x%x\n",xpos0,ypos0);		
			//printk("		nastech_ts_work: X0 = %d , Y0 = %d\n",xpos0,ypos0);
			
			printk("		nastech_ts_work: Finger : %d\n",Finger);
			//printk("		nastech_ts_work: X0 = 0x%x , Y0 = 0x%x\n",xpos0,ypos0);
			//printk("		nastech_ts_work: X1 = 0x%x , Y1 = 0x%x\n",xpos1,ypos1);
			//printk("		nastech_ts_work: X2 = 0x%x , Y2 = 0x%x\n",xpos2,ypos2);
			//printk("		nastech_ts_work: X3 = 0x%x , Y3 = 0x%x\n",xpos3,ypos3);
			//printk("		nastech_ts_work: X4 = 0x%x , Y4 = 0x%x\n",xpos4,ypos4);
			
			//printk("		nastech_ts_work: X0 = %d , Y0 = %d\n",xpos0,ypos0);
			//printk("		nastech_ts_work: X1 = %d , Y1 = %d\n",xpos1,ypos1);
			//printk("		nastech_ts_work: X2 = %d , Y2 = %d\n",xpos2,ypos2);
			//printk("		nastech_ts_work: X3 = %d , Y3 = %d\n",xpos3,ypos3);
			//printk("		nastech_ts_work: X4 = %d , Y4 = %d\n",xpos4,ypos4);
			
			printk("		nastech_ts_work: X0 = %d , Y0 = %d\n",nastech_points[0].x,nastech_points[0].y);
			printk("		nastech_ts_work: X1 = %d , Y1 = %d\n",nastech_points[1].x,nastech_points[1].y);
			printk("		nastech_ts_work: X2 = %d , Y2 = %d\n",nastech_points[2].x,nastech_points[2].y);
			printk("		nastech_ts_work: X3 = %d , Y3 = %d\n",nastech_points[3].x,nastech_points[3].y);
			printk("		nastech_ts_work: X4 = %d , Y4 = %d\n",nastech_points[4].x,nastech_points[4].y);
			
			printk("		nastech_ts_work: Status 0 = %d \n",nastech_points[0].status);
			printk("		nastech_ts_work: Status 1 = %d \n",nastech_points[1].status);
			printk("		nastech_ts_work: Status 2 = %d \n",nastech_points[2].status);
			printk("		nastech_ts_work: Status 3 = %d \n",nastech_points[3].status);
			printk("		nastech_ts_work: Status 4 = %d \n",nastech_points[4].status);
			printk("\n");
		}
		
	#endif
		
	{
		int i;
		int fingers=0;
		int ratio; //mg add 0415
		int xx,yy; //mg add 0415
		
		
		for(i=0;i<MAX_FINGER;i++)
		//for(i=0;i<3;i++)
		{
			if(!nastech_points[i].status)
			{
				input_report_abs(nas_priv->input, ABS_MT_TOUCH_MAJOR, 0);
			}
			else
			{
				input_report_abs(nas_priv->input, ABS_MT_TOUCH_MAJOR, 255);
				input_report_abs(nas_priv->input, ABS_MT_POSITION_X, nastech_points[i].x);
				
				yy=nastech_points[i].y;
				 if (yy<260)
	                         {
	                                ratio=yy;

	                                yy=  gamma600[ratio];


	                         }
				input_report_abs(nas_priv->input, ABS_MT_POSITION_Y, yy);
				fingers++;
			}
			input_mt_sync(nas_priv->input);
		}
		input_report_key(nas_priv->input, BTN_TOUCH, fingers > 0);
		//if(nastech_points[0].status)
		//{
		//	input_report_abs(nas_priv->input, ABS_X, nastech_points[0].x);
		//	input_report_abs(nas_priv->input, ABS_Y, nastech_points[0].y);
		//}
		input_sync(nas_priv->input);
		
		//mdelay(5);
	}
	

out:

    ;
	//enable_irq(ON_TOUCH_INT);
}
static int nastech_ts_probe(struct i2c_client *client,const struct i2c_device_id *idp)
{
	struct nas_ts_priv *nas_priv;
	struct input_dev *nas_input;
	//struct input_dev *nas_input0;
	int error;

	#ifdef Debug
	printk("+-----------------------------------------+\n");
	printk("|	nastech_ts_probe!                 |\n");
	printk("+-----------------------------------------+\n");
	#endif
	
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
	{
		#ifdef Debug
		printk("		nastech_ts_probe: need I2C_FUNC_I2C\n");
		#endif
		return -ENODEV;
	}
	#ifdef Debug
	else
	{
		printk("		nastech_ts_probe: i2c Check OK!\n");
		printk("		nastech_ts_probe: i2c_client name : %s\n",client->name);
		printk("		nastech_ts_probe: i2c_client addr : 0x%x\n",client->addr);
	}
	#endif
	
	nas_priv = kzalloc(sizeof(*nas_priv), GFP_KERNEL);
	if (!nas_priv)
	{
		#ifdef Debug
		printk("		nastech_ts_probe: kzalloc Error!\n");
		#endif
		error=-ENODEV;
		goto	err0;
		//return -ENODEV;
	}
	#ifdef Debug
	else
	{
		
		printk("		nastech_ts_probe: kzalloc OK!\n");
		
	}
	#endif
	
	dev_set_drvdata(&client->dev, nas_priv);
	nas_priv->client = client;
	i2c_set_clientdata(client, nas_priv);
	
	nas_input = input_allocate_device();
	if (!nas_input)
	{
		#ifdef Debug
		printk("		nastech_ts_probe: input_allocate_device Error\n");
		#endif
		error=-ENODEV;
		goto	err1_1;
		//return -ENODEV;
	}
	#ifdef Debug
	else
	{
		printk("		nastech_ts_probe: input_allocate_device OK\n");
	}
	#endif
	
	//nas_input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN) ;
	//nas_input->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH) | BIT_MASK(BTN_2);
	
	__set_bit(EV_ABS, nas_input->evbit);
	__set_bit(EV_KEY, nas_input->evbit);
	__set_bit(BTN_TOUCH, nas_input->keybit);
	
	//#if SWAP_XY
	//	input_set_abs_params(nas_input, ABS_X, 0,MAX_Y, 0, 0);//設定絕對座標的最大值與最小值
	//	input_set_abs_params(nas_input, ABS_Y, 0, MAX_X, 0, 0);//設定絕對座標的最大值與最小值
	//#else
		input_set_abs_params(nas_input, ABS_X, 0,MAX_X, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_Y, 0, MAX_Y, 0, 0);//設定絕對座標的最大值與最小值
	//#endif
		//input_set_abs_params(nas_input, ABS_HAT0X, 0,MAX_X, 0, 0);//設定絕對座標的最大值與最小值
		//input_set_abs_params(nas_input, ABS_HAT0Y, 0, MAX_Y, 0, 0);//設定絕對座標的最大值與最小值
	
	input_set_abs_params(nas_input, ABS_MT_TOUCH_MAJOR, 0,255, 0, 0);
	//#if SWAP_XY
	//	input_set_abs_params(nas_input, ABS_MT_POSITION_X, 0,MAX_Y, 0, 0);//設定絕對座標的最大值與最小值
	//	input_set_abs_params(nas_input, ABS_MT_POSITION_Y, 0, MAX_X, 0, 0);//設定絕對座標的最大值與最小值
	//#else
		input_set_abs_params(nas_input, ABS_MT_POSITION_X, 0,MAX_X, 0, 0);//設定絕對座標的最大值與最小值
		input_set_abs_params(nas_input, ABS_MT_POSITION_Y, 0, MAX_Y, 0, 0);//設定絕對座標的最大值與最小值
	//#endif
	//input_set_abs_params(nas_input, ABS_MT_TOUCH_MAJOR, 0,255, 0, 0);
	//input_set_abs_params(nas_input, ABS_MT_WIDTH_MAJOR, 0,15, 0, 0);
	//input_set_abs_params(nas_input, ABS_MT_TOUCH_MAJOR, 0,1, 0, 0);
	//input_set_abs_params(nas_input, ABS_MT_WIDTH_MAJOR, 0,1, 0, 0);
	
	nas_input->name = client->name;
	nas_input->id.bustype = BUS_I2C;
	nas_input->dev.parent = &client->dev;
	//nas_input->open = nastech_ts_open;
	//nas_input->close = nastech_ts_close;
	input_set_drvdata(nas_input, nas_priv);
	
	nas_priv->client = client;
	nas_priv->input = nas_input;

	//INIT_DELAYED_WORK(&priv->work, migor_ts_poscheck);
	INIT_WORK(&nas_priv->nas_work, nastech_ts_work);
	
	nas_priv->irq=ON_TOUCH_INT;
	#ifdef Debug
		printk("		nastech_ts_probe: nas_priv->irq : %d\n",nas_priv->irq);
	#endif
	
	error = input_register_device(nas_input);
	if(error)
	{
		#ifdef Debug
			printk("		nastech_ts_probe: input_register_device input Error!\n");
		#endif
		
		error=-ENODEV;
		goto	err1;
	}
	#ifdef Debug
	else
	{
		
		printk("		nastech_ts_probe: input_register_device input OK!\n");
		
	}
	#endif
	
	#ifdef Debug
		printk("		nastech_ts_probe: input_id: %d\n",&nas_input->id);
	#endif
	config_gpio();

	//WAKE_UP_PIN
	
	/*error = gpio_request(WAKE_UP_PIN,"GPK");
	if(error)
	{
		#ifdef Debug
			printk("		nastech_ts_probe: WAKE_UP_PIN gpio_request Error!\n");
		#endif
	}
	else
	{
		#ifdef Debug
			printk("		nastech_ts_probe: WAKE_UP_PIN gpio_request OK!\n");
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
			printk("		nastech_ts_probe: RESET_PIN gpio_request Error!\n");
		#endif
	}
	else
	{
		#ifdef Debug
			printk("		nastech_ts_probe: RESET_PIN gpio_request OK!\n");
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
			printk("		nastech_ts_probe: TOUCH_INT_PIN gpio_request Error!\n");
		#endif
	}
	else
	{
		#ifdef Debug
			printk("		nastech_ts_probe: TOUCH_INT_PIN gpio_request OK!\n");
		#endif
		s3c_gpio_cfgpin(TOUCH_INT_PIN, S3C_GPIO_SFN(0x0F));
		s3c_gpio_setpull(TOUCH_INT_PIN, S3C_GPIO_PULL_NONE);
		
		//gpio_direction_output(TOUCH_INT_PIN, 1);//GPIO Output Hight
	}*/
	//申請中斷
	error = request_irq(/*ON_TOUCH_INT*/nas_priv->irq, nastech_ts_isr,IRQF_DISABLED|IRQF_TRIGGER_FALLING, client->name, nas_priv);
	if (error)
	{
		#ifdef Debug
			printk("		nastech_ts_probe: request_irq Error!\n");
		#endif
		error=-ENODEV;
		goto err2;
	}
	#ifdef Debug
	else
	{
		
		printk("		nastech_ts_probe: request_irq OK!\n");
		
	}
	#endif
	
//2010y 11m 01d	
#ifdef CONFIG_HAS_EARLYSUSPEND
	nas_priv->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	nas_priv->early_suspend.suspend = nastech_ts_early_suspend;
	nas_priv->early_suspend.resume = nastech_ts_late_resume;
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
	unregister_early_suspend(&nas_priv->early_suspend);
	#ifdef Debug
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
static int nastech_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	//u8	msg[4];
	//int	error;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_suspend                |\n");
		printk("+-----------------------------------------+\n");
	#endif
	/*
	msg[0]=0xFC;
	msg[1]=0x3A;
	msg[2]=0x03;
	msg[3]=0xC5;
	
	error=nastech_i2c_write(client,msg,4);
	
	#ifdef Debug	
		if(error<0)
		{
				printk("|	nastech_ts_suspend: Command Error\n");
				printk("|	nastech_ts_suspend: error = 0x%x\n",error);
		}
		else
		{
				printk("|	nastech_ts_suspend: Command OK\n");
				printk("|	nastech_ts_suspend: error = 0x%x\n",error);
		}
	#endif
	*/
	
	return 0;
}
static int nastech_ts_resume(struct i2c_client *client)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_resume                |\n");
		printk("+-----------------------------------------+\n");
	#endif
	//gpio_direction_output(WAKE_UP_PIN, 0);
	//mdelay(5);
	//gpio_direction_output(WAKE_UP_PIN, 1);
	return 0;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
static void nastech_ts_early_suspend(struct early_suspend *h)
{
	struct nas_ts_priv *ts;
	ts = container_of(h, struct nas_ts_priv, early_suspend);
	nastech_ts_suspend(ts->client, PMSG_SUSPEND);
}

static void nastech_ts_late_resume(struct early_suspend *h)
{
	struct nas_ts_priv *ts;
	ts = container_of(h, struct nas_ts_priv, early_suspend);
	nastech_ts_resume(ts->client);
}
#endif

static int nastech_ts_open(struct input_dev *dev)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_open!                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	return 0;
}
static void nastech_ts_close(struct input_dev *dev)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_close!                 |\n");
		printk("+-----------------------------------------+\n");
	#endif
}
static irqreturn_t nastech_ts_isr(int irq, void *dev_id)
{
	struct nas_ts_priv *nas_priv = dev_id;
	int ret;
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_isr!                   |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	
	//disable_irq_nosync(ON_TOUCH_INT);//關避中斷
	//printk("|	nastech_ts_isr---0                   |\n");

	//disable_irq(ON_TOUCH_INT);//關避中斷
	//disable_irq_nosync(ON_TOUCH_INT);

	//printk("|	nastech_ts_isr---1                   |\n");
	ret=queue_work(nastech_wq, &nas_priv->nas_work);
	//printk("|	nastech_ts_isr---2                   |\n");

	//ret=schedule_work(&nas_priv->nas_work);
	if(ret)
	{
		#ifdef Debug
		printk("		nastech_ts_isr: queue_work non-zero otherwise!\n");
		#endif
	}
	#ifdef Debug
	else
	{
		printk("		nastech_ts_isr: queue_work work was already on a queue!\n");
	}
	#endif
	return IRQ_HANDLED;
}
static enum hrtimer_restart nastech_ts_timer(struct hrtimer *timer)
{
	struct nas_ts_priv *nas_priv = container_of(timer, struct nas_ts_priv, timer);
	
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_timer!                 |\n");
		printk("+-----------------------------------------+\n");
	#endif
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
#ifndef CONFIG_HAS_EARLYSUSPEND	
	.suspend	= nastech_ts_suspend,
	.resume		= nastech_ts_resume,
#endif
	.id_table = nastech_ts_id,
};

static char banner[] __initdata = KERN_INFO "nastech Touchscreen driver, (c) 2010 nas Technologies Corp. \n";
static int __init nastech_ts_init(void)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int ret;

	#ifdef Debug	
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_init!                  |\n");
		printk("+-----------------------------------------+\n");
		printk(banner);
	#endif
	
	printk(banner);
	
	nastech_wq = create_singlethread_workqueue("nastech_wq");
	if (!nastech_wq)
	{
		#ifdef Debug
		printk("		nastech_ts_init: create_singlethread_workqueue Error!\n");
		#endif
		return -ENOMEM;
	}
	#ifdef Debug
	else
	{
		printk("		nastech_ts_init: create_singlethread_workqueue OK!\n");
	}
	#endif
	
	ret=i2c_add_driver(&nastech_ts_driver);//註冊 i2c_Driver
	if(ret)
	{
		#ifdef Debug
		printk("		nastech_ts_init: i2c_add_driver Error! \n");
		#endif
		//return ret;
	}
	else
	{
		printk("		nastech_ts_init: i2c_add_driver OK! \n");
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
	return ret;
err_driver:
	i2c_del_driver(&nastech_ts_driver);
	return -ENODEV;
}

static void __exit nastech_ts_exit(void)
{
	#ifdef Debug
		printk("+-----------------------------------------+\n");
		printk("|	nastech_ts_exit!                  |\n");
		printk("+-----------------------------------------+\n");
	#endif
	
	i2c_del_driver(&nastech_ts_driver);//註銷 i2c_Driver
	if (nastech_wq)
		destroy_workqueue(nastech_wq);
}

module_init(nastech_ts_init);
module_exit(nastech_ts_exit);

MODULE_AUTHOR("nastech Ming");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("nastech Touchscreen Driver");
