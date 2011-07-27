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
 
 2010-7-13 ：wifi power control by yqcui
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

//#define DEBUG_PANEL

#if defined(CONFIG_FB_IMAP_LCD800X480)||defined(CONFIG_FB_IMAP_LCD800X480_XY)||defined(CONFIG_FB_IMAP_LCD800X480_XY2)

#if defined(CONFIG_BOARD_E5)
    #define TSC2007_X 4096
    #define TSC2007_Y 4096
    #define TSC2007_X_OFFSET 120
    #define TSC2007_Y_OFFSET 200
    
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1000

    #define TSC2007_X_KEY 3920
#else
    #define TSC2007_X 3950
    #define TSC2007_Y 3750
    #define TSC2007_X_OFFSET 120
    #define TSC2007_Y_OFFSET 160
    
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1000
#endif
    #define POINT_X_JUMP 700
    #define POINT_Y_JUMP 700

#elif defined(CONFIG_FB_IMAP_LCD800X600)
    #define TSC2007_X 3900
    #define TSC2007_Y 3800
    #define TSC2007_X_OFFSET 120
    #define TSC2007_Y_OFFSET 150
    
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1500

    #define POINT_X_JUMP 700
    #define POINT_Y_JUMP 700


#elif defined(CONFIG_FB_IMAP_LCD1024X576)||defined(CONFIG_FB_IMAP_LCD1024X600)||defined(CONFIG_FB_IMAP_LCD1024X600_QM)||defined(CONFIG_FB_IMAP_LCD1024X600_XY)
  #if defined(CONFIG_BOARD_H0)
    #define TSC2007_X 4050
    #define TSC2007_Y 3900
    #define TSC2007_X_OFFSET 120
    #define TSC2007_Y_OFFSET 160    
    
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1500
  #else
    #define TSC2007_X 3985
    #define TSC2007_Y 3850
    #define TSC2007_X_OFFSET 80
    #define TSC2007_Y_OFFSET 130
      
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1000
  #endif
    #define POINT_X_JUMP 700
    #define POINT_Y_JUMP 700  

#elif defined(CONFIG_FB_IMAP_LCD1024X600_7INCH)
    #define TSC2007_X 3950
    #define TSC2007_Y 3750
    #define TSC2007_X_OFFSET 120
    #define TSC2007_Y_OFFSET 160
    
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1000
    
    #define POINT_X_JUMP 700
    #define POINT_Y_JUMP 700
    
#elif defined(CONFIG_FB_IMAP_LCD1024X768_8INCH)
    #define TSC2007_X 4000
    #define TSC2007_Y 3850
    #define TSC2007_X_OFFSET 50
    #define TSC2007_Y_OFFSET 130
    
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1500

    #define POINT_X_JUMP 700
    #define POINT_Y_JUMP 700 
    
#elif defined(CONFIG_FB_IMAP_LCD1024X768_9INCH)
    #define TSC2007_X 4000
    #define TSC2007_Y 3850
    #define TSC2007_X_OFFSET 50
    #define TSC2007_Y_OFFSET 130
        
    #define TOUCHPANEL_SAMPLE_PRESS_INTENSITY           0x1500
    
    #define POINT_X_JUMP 700
    #define POINT_Y_JUMP 700      
#endif

#if defined(CONFIG_BOARD_B0)

#define TOUCH_EINT          IRQ_EINT1
#define TOUCH_EINT_SHIFT    1
#define TOUCH_EINT_FILTER_GRP      rEINTFLTCON0

#else

#define TOUCH_EINT          IRQ_EINT4
#define TOUCH_EINT_SHIFT    4
#define TOUCH_EINT_FILTER_GRP      rEINTFLTCON1

#endif

#define GPIO_USB_WIFI_POWER IMAPX200_GPO(15)	

/*
#define TSC2007_OFFSET 100
#define TSC2007_DELAY 2*/

//#define TS_POLL_DELAY			1 /* ms delay between samples */
#define TS_POLL_PERIOD			20 /* ms delay between samples */

#define TSC2007_MEASURE_TEMP0		(0x0 << 4)
#define TSC2007_MEASURE_AUX		(0x2 << 4)
#define TSC2007_MEASURE_TEMP1		(0x4 << 4)
#define TSC2007_ACTIVATE_XN		(0x8 << 4)
#define TSC2007_ACTIVATE_YN		(0x9 << 4)
#define TSC2007_ACTIVATE_YP_XN		(0xa << 4)
#define TSC2007_SETUP			(0xb << 4)
#define TSC2007_MEASURE_X		(0xc << 4)
#define TSC2007_MEASURE_Y		(0xd << 4)
#define TSC2007_MEASURE_Z1		(0xe << 4)
#define TSC2007_MEASURE_Z2		(0xf << 4)

#define TSC2007_POWER_OFF_IRQ_EN	(0x0 << 2)
#define TSC2007_ADC_ON_IRQ_DIS0		(0x1 << 2)
#define TSC2007_ADC_OFF_IRQ_EN		(0x2 << 2)
#define TSC2007_ADC_ON_IRQ_DIS1		(0x3 << 2)

#define TSC2007_12BIT			(0x0 << 1)
#define TSC2007_8BIT			(0x1 << 1)

#define MAX_12BIT_SHIFT     12
#define	MAX_12BIT			((1 << MAX_12BIT_SHIFT) - 1)

#define MAX_PRESSURE    100

#define ADC_ON_12BIT	(TSC2007_12BIT | TSC2007_ADC_ON_IRQ_DIS0)

#define READ_Y		(ADC_ON_12BIT | TSC2007_MEASURE_Y)
#define READ_Z1		(ADC_ON_12BIT | TSC2007_MEASURE_Z1)
#define READ_Z2		(ADC_ON_12BIT | TSC2007_MEASURE_Z2)
#define READ_X		(ADC_ON_12BIT | TSC2007_MEASURE_X)
#define PWRDOWN		(TSC2007_12BIT | TSC2007_POWER_OFF_IRQ_EN)

#define MAX_ADC_VAL     0xffe   //0x1000
#define MIN_ADC_VAL     0x1


static DECLARE_MUTEX(tsc2007_mutex);

struct tsc2007_setup_data {
	int i2c_bus;	
	unsigned short i2c_address;
};
static struct tsc2007_setup_data tsc2007_setup = {
	.i2c_address = 0x90>>1,
	.i2c_bus = 0x2,
};

struct ts_event {
	u16	x;
	u16	y;
	u16	z1, z2;
};

struct tsc2007 {
	struct input_dev	*input;
	char			phys[32];
	struct timer_list timer;
	//struct work_struct work;
	struct delayed_work work;
	struct ts_event		tc;

	struct i2c_client	*client;

	spinlock_t		lock;

	u16			model;
	u16			x_plate_ohms;

	int		    pendown;
	int			irq;

	int			(*get_pendown_state)(void);
	void			(*clear_penirq)(void);
};

struct tsc2007 * tsc2007_battery_i2c;

// 马达震动时间，如果值是5的话，就关掉
static int motor_count = 0;
static struct class *motor_class;
static int g_motor = 0;
static inline void enable_motor(int enable);
static void tsc2007_read_values(struct tsc2007 *tsc, struct ts_event *tc);
static u32 tsc2007_calculate_pressure(struct tsc2007 *tsc, struct ts_event *tc);

extern void virtual_key_isr(int x,int y,int p);


static int tsc2007_write(struct tsc2007 *tsc,u8 reg)
{
	u8 data = reg;
	int ret;
	down(&tsc2007_mutex);
	ret = i2c_master_send(tsc->client, &data, 1);
	if (ret < 0){
		printk("tsc2007_write is error!\r\n");	
	}
	up(&tsc2007_mutex);

	return ret;
}

static inline int tsc2007_xfer(struct tsc2007 *tsc, u8 cmd)
{
	u8 buf[2];
	int ret;
	u8 data = cmd;
	
	down(&tsc2007_mutex);
	/* Set address */
	ret = i2c_master_send(tsc->client, (const char *)&data, 1);
	if (ret < 0) 
		goto ret_error;
	ret = i2c_master_recv(tsc->client, (char *)&buf, 2);
	if (ret < 0)
		goto ret_error;
	ret = ( (buf[0]<<4) | buf[1]>>4 ) & 0xfff;
	//printk("ts : buf[0] = 0x%x,buf[1] = 0x%xret=0x%x\r\n",buf[0],buf[1],ret);
	
ret_error:
	up(&tsc2007_mutex);
	return ret;
}

#define G_XY 3
static int g_x[G_XY],g_y[G_XY];
static int g_calc_count = 0;


uint32_t tsc2007_get_voltage(void)
{
	return  (uint32_t)tsc2007_xfer(tsc2007_battery_i2c,0x18);
}
EXPORT_SYMBOL(tsc2007_get_voltage);

//static int g_a = 0;
static void tsc2007_read_values(struct tsc2007 *tsc, struct ts_event *tc)
{

	/* y- still on; turn on only y+ (and ADC) */
	tsc2007_write(tsc,0x90);
	tc->y = tsc2007_xfer(tsc, 0xd8);

	/* turn y- off, x+ on, then leave in lowpower */
	tsc2007_write(tsc,0x80);
	tc->x = tsc2007_xfer(tsc, 0xc8);

	/* turn y+ off, x- on; we'll use formula #1 */
	tc->z1 = tsc2007_xfer(tsc, 0xe8);
	tc->z2 = tsc2007_xfer(tsc, 0xf8);

	/* Prepare for next touch reading - power down ADC, enable PENIRQ */
	//tsc2007_xfer(tsc, PWRDOWN);
}
/*
static void tsc2007_timer(unsigned long data)
{
	struct tsc2007 *ts = (struct tsc2007 *) data;
	unsigned long flags;
}*/

static u32 tsc2007_calculate_pressure(struct tsc2007 *tsc, struct ts_event *tc)
{
	u32 rt = (u32)-1;
    u32 delta;

    /*printk("point(%4d,%4d), z(%d %d)\n",
               tc->x, tc->y, tc->z1,tc->z2);*/
		
    if(tc->x >= MAX_ADC_VAL || tc->x <= MIN_ADC_VAL)
        return (u32)-1;
		
    if(tc->y >= MAX_ADC_VAL || tc->y <= MIN_ADC_VAL)
        return (u32)-1;

    if(!tc->z1)
        return (u32)-1;

	//rt=(u32)tc->x*(tc->z2/tc->z1-1);
    if(tc->z2 > tc->z1)
        delta = tc->z2 - tc->z1;
    else
        delta = tc->z1 - tc->z2;

    if(delta){
        delta = delta*tc->x/tc->z1;
    }
    rt = delta;
    /*printk("point(%4d,%4d), z(%d %d) rt 0x%x\n",
               tc->x, tc->y, tc->z1,tc->z2,rt);*/

	return rt;
}


static void tsc2007_send_up_event(struct tsc2007 *tsc)
{
	struct input_dev *input = tsc->input;

	// printk("UP\n");

	input_report_key(input, BTN_TOUCH, 0);
	input_report_abs(input, ABS_PRESSURE, 0);
	input_sync(input);
}

#define TOUCH_NONE  0
#define TOUCH_PEN   (1<<1)
#define TOUCH_KEY   (1<<2)

#define PENDOWN_STABLE   (1<<8)

static void tsc2007_ts_poscheck(struct work_struct *work)
{
    struct tsc2007 *ts =
       container_of(to_delayed_work(work), struct tsc2007, work);
    struct ts_event tc;
    u32 rt;
	int dx,dy,dx1,dy1,dx2,dy2;
	int pressure;

    if (ts->get_pendown_state()) { 

        tsc2007_read_values(ts, &tc);
        rt = tsc2007_calculate_pressure(ts, &tc);   
        
        dx = (int)tc.x;
        dy = (int)tc.y;

#if defined(DEBUG_PANEL)
       printk("point(%4d,%4d), pressure (%x) 0x%x %d\n",
           tc.x,tc.y, rt ,ts->pendown,ts->get_pendown_state());
#endif 

#if defined(CONFIG_TOUCHSCREEN_TSC2007_KEY)
        if(dx > TSC2007_X_KEY)
#else
        if(0)
#endif
        {
            if(ts->pendown & TOUCH_PEN){
                if(ts->pendown & PENDOWN_STABLE)
                    tsc2007_send_up_event(ts);
                ts->pendown &= ~(PENDOWN_STABLE|TOUCH_PEN);
            }
            ts->pendown |= TOUCH_KEY;
        }else{
            if(ts->pendown & TOUCH_KEY){
                if(ts->pendown & PENDOWN_STABLE)
                    virtual_key_isr(0,0,0);
                ts->pendown &= ~(PENDOWN_STABLE|TOUCH_KEY);
            }        
            ts->pendown |= TOUCH_PEN;
        }
    }else{
        if(ts->pendown & PENDOWN_STABLE){
            if(ts->pendown & TOUCH_PEN){
                tsc2007_send_up_event(ts);
            }else if(ts->pendown & TOUCH_KEY){
                virtual_key_isr(0,0,0);
            } 
        }
        ts->pendown = TOUCH_NONE;
        goto out;

    }

    if(ts->pendown & TOUCH_PEN){
        
        if(rt < TOUCHPANEL_SAMPLE_PRESS_INTENSITY ){	
    		// 第一个点不做处理
    		if( g_calc_count > 1 ) {
    			
    			dy1 = abs(dy - g_y[0]);
    			dy2 = abs(g_y[0] - g_y[1]);

    			dx1 = abs(dx - g_x[0]);
    			dx2 = abs(g_x[0] - g_x[1]);
    			
    			if( (( abs( dy2 - dy1 ) ) < POINT_Y_JUMP ) && (( abs( dx2 - dx1 ) ) < POINT_X_JUMP ))	{
    				
    				g_y[2] = dy - g_y[0];
    				g_x[2] = dx - g_x[0];	
    				
    				//printk("tc.x = %d , tc.y = %d, g_x[2] = %d, g_y[2] = %d, g_x[0] = %d, g_y[0] = %d\r\n",tc.x,tc.y,g_x[2],g_y[2],g_x[0],g_y[0]);
    			} else {
    				
    				// 处理错误点，补点
    				dx = g_x[0] + g_x[2];
    				dy = g_y[0] + g_y[2];
    				if( dx < TSC2007_X_OFFSET ) { 
    					dx = TSC2007_X_OFFSET;
    					g_calc_count = 0;
    				}
    				if( dy < TSC2007_Y_OFFSET ) {
    					dy = TSC2007_Y_OFFSET;
    					g_calc_count = 0;
    				}
    				if( dx > TSC2007_X ) { 
    					dx = TSC2007_X;
    					g_calc_count = 0;
    				}
    				if( dy > TSC2007_Y ) {
    					dy = TSC2007_Y;
    					g_calc_count = 0;
    				}				
    					
    				//printk("error : tc.x = %d , tc.y = %d, g_x[0] = %d, g_y[0] = %d\r\n",tc.x,tc.y,g_x[0],g_y[0]);
    			}
    		}
    	}

        if (rt < TOUCHPANEL_SAMPLE_PRESS_INTENSITY) {
            struct input_dev *input = ts->input;

            if (!(ts->pendown & PENDOWN_STABLE)) {
                input_report_key(input, BTN_TOUCH, 1);
                ts->pendown |=PENDOWN_STABLE;
            } 
  
            //can be negative value

            g_y[1] = g_y[0];
    		g_x[1] = g_x[0];
    		
    		// 保存前一个点
    		g_y[0] = dy;
    		g_x[0] = dx;
    		
    		g_calc_count++;

          //map oritentation
          #if defined(CONFIG_FB_IMAP_LCD800X480)||defined(CONFIG_FB_IMAP_LCD800X480_XY)||defined(CONFIG_FB_IMAP_LCD800X480_XY2)
            //opposite point y
            dy = TSC2007_Y - dy + TSC2007_Y_OFFSET;
          #endif
          #if defined(CONFIG_FB_IMAP_LCD1024X600_7INCH)
            //opposite point x
            dx = TSC2007_X - dx + TSC2007_X_OFFSET;
          #endif
          
          #if defined(CONFIG_FB_IMAP_LCD800X600)||defined(CONFIG_FB_IMAP_LCD1024X768_8INCH)
            //opposite point xy
            dx = TSC2007_X - dx + TSC2007_X_OFFSET;
            dy = TSC2007_Y - dy + TSC2007_Y_OFFSET;
          #endif

          #if defined(CONFIG_FB_IMAP_LCD1024X600_XY)
            #if defined(CONFIG_BOARD_K0)
            dx = TSC2007_X - dx + TSC2007_X_OFFSET;
            dy = TSC2007_Y - dy + TSC2007_Y_OFFSET;
            #endif
          #endif
            //pressue is 0~100
            pressure = ((TOUCHPANEL_SAMPLE_PRESS_INTENSITY-rt)*MAX_PRESSURE)/TOUCHPANEL_SAMPLE_PRESS_INTENSITY;

            dx = dx<TSC2007_X_OFFSET? TSC2007_X_OFFSET:dx;
            dy = dy<TSC2007_Y_OFFSET? TSC2007_Y_OFFSET:dy;

            dx = ((dx - TSC2007_X_OFFSET)<<MAX_12BIT_SHIFT)/(TSC2007_X-TSC2007_X_OFFSET);
            dy = ((dy - TSC2007_Y_OFFSET)<<MAX_12BIT_SHIFT)/(TSC2007_Y-TSC2007_Y_OFFSET);

          #if defined(DEBUG_PANEL)
             printk("(%4d,%4d %4d)\n",
                 dx, dy ,pressure);
          #endif

    	   input_report_abs(input, ABS_X, dx);
    	   input_report_abs(input, ABS_Y, dy);
    	   input_report_abs(input, ABS_PRESSURE,pressure);

    	   input_sync(input);

       }
    }else{
        if (rt < TOUCHPANEL_SAMPLE_PRESS_INTENSITY){
            if (!(ts->pendown & PENDOWN_STABLE)) {
                ts->pendown |= PENDOWN_STABLE;
                virtual_key_isr(dx,dy,1);
            }
        }
    }
out:
   if (ts->pendown)
	   schedule_delayed_work(&ts->work,
					 msecs_to_jiffies(TS_POLL_PERIOD));
   else{
#if defined(DEBUG_PANEL)
          printk("pen up\n");
#endif
	   enable_irq(ts->irq);				  	   
   }
}

static irqreturn_t tsc2007_irq(int irq, void *handle)
{
	struct tsc2007 *ts = handle;
	
	// 强制关闭motor
	// gpio_set_value(IMAPX200_GPB(4),0);	
	enable_motor(0);
	motor_count = 0;

	if (!ts->get_pendown_state || likely(ts->get_pendown_state())) {
		disable_irq_nosync(ts->irq);
		
		// motor
		// gpio_set_value(IMAPX200_GPB(4),1);
		enable_motor(1);
		motor_count = 0;
		g_calc_count = 0;
		
		schedule_delayed_work(&ts->work, msecs_to_jiffies(TS_POLL_PERIOD));
	} 
	
	return IRQ_HANDLED;	
}

// config interrupt EINT1 -> EINT2 -> EINT4
// 
static void config_gpio(void)
{
	unsigned long value,shift;

#ifndef CONFIG_TOUCHSCREEN_NAS
    //EINTCON
    shift = TOUCH_EINT_SHIFT;
    shift <<= 2;
	value = __raw_readl(rEINTCON);
	value &= ~(0xF<<shift);
	value |= 0x02<<shift;
	__raw_writel(value, rEINTCON);

    //EINTFILTER
    shift = TOUCH_EINT_SHIFT;
    if(shift>=4)
        shift-=4;
    shift <<= 3;
	value = __raw_readl(TOUCH_EINT_FILTER_GRP);
	value |= 0xff<<shift;
	__raw_writel(value, TOUCH_EINT_FILTER_GRP);        
#endif
	
	// wifi power
	if (gpio_request(GPIO_USB_WIFI_POWER, "wifi_power")) {
		printk("motor is error\r\n");
		return ;
	}
	gpio_direction_output(GPIO_USB_WIFI_POWER,1);
}

static int imap_get_pendown_state(void)
{
	volatile unsigned int tmp;
	tmp = __raw_readl(rGPGDAT);
	tmp = !(tmp&(1<<TOUCH_EINT_SHIFT));
	return tmp;
}

static void imap_clear_penirq(void)
{
	volatile unsigned int tmp;
	tmp = 0xffffffff;
	//__raw_writel(tmp, rEINTG4PEND);
}

static int tsc2007_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct tsc2007 *ts;
	struct tsc2007_platform_data *pdata = pdata = client->dev.platform_data;
#ifndef CONFIG_TOUCHSCREEN_NAS	
	struct input_dev *input_dev=NULL;
#endif
	int err;
	
	printk("tsc2007_probe .................................... \r\n");

	ts = kzalloc(sizeof(struct tsc2007), GFP_KERNEL);
	if (!ts) {
		err = -ENOMEM;
		goto err_free_mem;
	}

	ts->client = client;
	i2c_set_clientdata(client, ts);
	
	config_gpio();
	tsc2007_write(ts,/*0xb3*/0xb1);
	tsc2007_write(ts,0x30);

	tsc2007_battery_i2c = ts;	

#ifndef CONFIG_TOUCHSCREEN_NAS

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		goto err_free_mem;
	}
	ts->input = input_dev;

	//INIT_WORK(&ts->work, tsc2007_ts_poscheck);
	INIT_DELAYED_WORK(&ts->work, tsc2007_ts_poscheck);

	spin_lock_init(&ts->lock);


	ts->model             = 7843;
	ts->x_plate_ohms      = 180;
	ts->get_pendown_state = imap_get_pendown_state;
	ts->clear_penirq      = imap_clear_penirq;

	snprintf(ts->phys, sizeof(ts->phys),
		 "%s/input0", dev_name(&client->dev));

	input_dev->name = "TSC2007 Touchscreen";
	input_dev->phys = ts->phys;
	input_dev->id.bustype = BUS_I2C;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	//input_set_abs_params(input_dev, ABS_X, 0, MAX_12BIT, 0, 0);
	//input_set_abs_params(input_dev, ABS_Y, 0, MAX_12BIT, 0, 0);
	//input_set_abs_params(input_dev, ABS_PRESSURE, 0, MAX_12BIT, 0, 0);

#ifdef DEF_1024_576
	input_set_abs_params(input_dev, ABS_X, 0, 1024, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, 576, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, MAX_12BIT, 0, 0);
#else
	input_set_abs_params(input_dev, ABS_X, 0, MAX_12BIT/*TSC2007_X+100*/, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, MAX_12BIT/*TSC2007_Y+100*/, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, /*MAX_12BIT*/MAX_PRESSURE, 0, 0);
#endif

	// ts->irq = client->irq;
	ts->irq = TOUCH_EINT;
	
	err = request_irq(ts->irq, tsc2007_irq, IRQF_DISABLED,
			client->dev.driver->name, ts);
	if (err < 0) {
		dev_err(&client->dev, "irq %d busy?\n", ts->irq);
		goto err_free_mem;
	}
	printk("tsc2007_probe : request_irq , ts = 0x%p\r\n",ts);
	/* Prepare for next touch reading - power down ADC, enable PENIRQ */
	
	err = input_register_device(input_dev);
	if (err)
		goto err_free_irq;

	dev_info(&client->dev, "registered with irq (%d)\n", ts->irq);
	tsc2007_xfer(ts, PWRDOWN);
#else
	tsc2007_xfer(ts, TSC2007_12BIT|TSC2007_ADC_ON_IRQ_DIS0);
#endif
	
	return 0;
#ifndef CONFIG_TOUCHSCREEN_NAS
err_free_irq:
	free_irq(ts->irq, ts);
	hrtimer_cancel(&ts->timer);
#endif

err_free_mem:
#ifndef CONFIG_TOUCHSCREEN_NAS	
	input_free_device(input_dev);
#endif
	kfree(ts);
	return err;
}

static int tsc2007_remove(struct i2c_client *client)
{
	struct tsc2007	*ts = i2c_get_clientdata(client);
	struct tsc2007_platform_data *pdata;

	pdata = client->dev.platform_data;
	pdata->exit_platform_hw();

	free_irq(ts->irq, ts);
	hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input);
	kfree(ts);

	return 0;
}

static struct i2c_device_id tsc2007_idtable[] = {
	{ "tsc2007_ts", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, tsc2007_idtable);

static struct i2c_driver tsc2007_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "tsc2007_ts"
	},
	.id_table	= tsc2007_idtable,
	.probe		= tsc2007_probe,
	.remove		= tsc2007_remove,
};

static inline void enable_motor(int enable){
	// printk("enable_motor : g_motor = 0x%x\r\n",g_motor);
	if(g_motor) {
		//gpio_set_value(IMAPX200_GPB(4),enable);
	}
	return;
}

static int motor_open(struct inode *inode, struct file *file)
{
	printk("motor_open\r\n");
	return 0;	
}
static int motor_release(struct inode *inode, struct file *file)
{
	printk("motor_release\r\n");
	return 0;	
}

extern void zt_lock_screen(void);
extern void zt_shut_down();

static int motor_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
//	int param;
	printk("motor_ioctl , cmd = %d\r\n",cmd);
	switch(cmd)
	{
	    //motor
		case 0 :
			g_motor	= arg;
			printk("motor_ioctl , g_motor = %d\r\n",g_motor);
			break;
			
		// wifi power on
		case 10:		    
#if !(defined(CONFIG_BOARD_G0_3G))
			gpio_set_value(GPIO_USB_WIFI_POWER,1);
#endif
			break;
			
		// wifi power down
		case 11:
#if !(defined(CONFIG_BOARD_G0_3G))
			gpio_set_value(GPIO_USB_WIFI_POWER,0);
#endif
			break;
		//IOCTL_GPS_HW_INIT
		case 12:
            /*
            if (gpio_request(IMAPX200_GPE(7), "gps_power")) {
                printk("motor is error\r\n");
            }
            gpio_direction_output(IMAPX200_GPE(7),0);
            imapx200_gpio_setpull(IMAPX200_GPE(7),0);*/
			break;
		//IOCTL_GPS_HW_CONTROL
		case 13:
            //gpio_set_value(IMAPX200_GPE(7),!!arg);
            break;
		case 15:
			zt_lock_screen();
			break;
		// factory reset
		case 16:
			writel('EDat' ,rINFO0);
			break;
        // image update
		case 17:
			zt_shut_down();
			break;		
        // image update
		case 18:
		    writel('NewL' ,rINFO0);
			break;
        case 22:
            //gpio_set_value(IMAPX200_GPE(0),!!arg);
            break;
		default:
			g_motor = 0;
	}
	return 0;	
}
static struct file_operations motor_fops = 
{
	.owner		= THIS_MODULE,
	.open		= motor_open,
	.release	= motor_release,
	.ioctl		= motor_ioctl,
};

static int __init tsc2007_init(void)
{

	struct i2c_board_info info;
	struct i2c_adapter *adapter;
	struct i2c_client *client;
	int ret;
	
	//X900_IICInit();
	/* create motor dev */
	if (register_chrdev(MOTOR_MAJOR, "motor", &motor_fops) < 0)
	{
		printk("Register char device for memalloc error\n");
		return -EFAULT;
	}

	motor_class = class_create(THIS_MODULE, "motor");
	device_create(motor_class, NULL, MKDEV(MOTOR_MAJOR, 0), NULL, "motor");	

	printk("touch init");
	ret = i2c_add_driver(&tsc2007_driver);
	if (ret != 0) {
		printk("tsc2007_init : can't add i2c driver\n");
		return ret;
	}

	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = tsc2007_setup.i2c_address;
	strlcpy(info.type, "tsc2007_ts", I2C_NAME_SIZE);

//#ifndef CONFIG_TOUCHSCREEN_NAS
	adapter = i2c_get_adapter(tsc2007_setup.i2c_bus);
	if (!adapter) {
		printk("tsc2007_init : can't get i2c adapter %d\n",
			tsc2007_setup.i2c_bus);
		goto err_driver;
	}
/*#else
	adapter = i2c_get_adapter(0x1);
	if (!adapter) {
		printk("tsc2007_init : can't get i2c adapter %d\n",
			tsc2007_setup.i2c_bus);
		goto err_driver;
	}
#endif*/

	client = i2c_new_device(adapter, &info);

	i2c_put_adapter(adapter);
	if (!client) {
		printk("tsc2007_init : can't add i2c device at 0x%x\n",
			(unsigned int)info.addr);
		goto err_driver;
	}

	return 0;

err_driver:
	i2c_del_driver(&tsc2007_driver);
	return -ENODEV;
}

static void __exit tsc2007_exit(void)
{
	i2c_del_driver(&tsc2007_driver);
}

module_init(tsc2007_init);
module_exit(tsc2007_exit);

MODULE_AUTHOR("Kwangwoo Lee <kwlee@mtekvision.com>");
MODULE_DESCRIPTION("TSC2007 TouchScreen Driver");
MODULE_LICENSE("GPL");
