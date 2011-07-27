/************************************************************
**   driver/video/backlight/imapx200_bl.c
**  
**   Copyright (c) 2009~2014 ShangHai Infotm .Ltd all rights reserved.
**
**   Use of infoTM's code  is governed by terms and conditions
**   stated in the accompanying licensing statment.
**   
**   Description: backlight control driver for imapx200 SOC
** 
**
**   AUTHOR:
**   Haixu Fu	 <haixu_fu@infotm.com>
**   warits		 <warits.wang@infotm.com>
**
**   
**   Revision History:
**  ---------------------------------
**   1.1  03/06/2010   Haixu Fu
**   1.2  04/01/2010   warits (add android features)
*******************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/sysdev.h>
#include <linux/gpio.h>

#define	IMAP_MAX_INTENSITY (0xff)
#define	IMAP_DEFAULT_INTENSITY IMAP_MAX_INTENSITY

static int imapbl_suspended = 0;
static int imapbl_cur_brightness = IMAP_DEFAULT_INTENSITY;

struct imapbl_data {
	int current_intensity;
	int suspend;
};

static DEFINE_MUTEX(bl_mutex);

extern int imap_timer_setup(int channel,unsigned long g_tcnt,unsigned long gtcmp);
extern int imap_pwm_suspend(struct sys_device *pdev, pm_message_t pm);
extern int imap_pwm_resume(struct sys_device *pdev);

#define GPIO_BK_PWM      IMAPX200_GPF(8)


#if defined(CONFIG_BOARD_B0)
#define GPIO_BK_POWER_EN IMAPX200_GPL(3)
#define GPIO_LCD_PWR_EN  IMAPX200_GPO(11)
#define GPIO_PWM_ON_VAL  1
#endif

#if (defined(CONFIG_BOARD_E3)||defined(CONFIG_BOARD_E4))
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  1
#endif

#if (defined(CONFIG_BOARD_E5))
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  0
#endif


#if defined(CONFIG_BOARD_F0)
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  0
#endif

#if defined(CONFIG_BOARD_G0)||defined(CONFIG_BOARD_G0_3G)
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  1
#endif

#if defined(CONFIG_BOARD_H0)
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  1
#endif

#if defined(CONFIG_BOARD_I0)||defined(CONFIG_BOARD_J0)
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  0
#endif

#if defined(CONFIG_BOARD_K0)
#define GPIO_BK_POWER_EN IMAPX200_GPA(7)
#define GPIO_LCD_PWR_EN  IMAPX200_GPE(6)
#define GPIO_PWM_ON_VAL  1
#endif

static void bk_power_init(void)
{
    if (gpio_request(GPIO_BK_PWM, "lcd-pwm")) {
        printk("bk power init failed\n");
        return;
    }

    if (gpio_request(GPIO_BK_POWER_EN, "bk-pwr")) {
        printk("bk power init failed\n");
        return;
    }

    if (gpio_request(GPIO_LCD_PWR_EN, "lcd-pwr")) {
        printk("bk power init failed\n");
        return;
    }

    //gpio_direction_output(GPIO_BK_PWM,0);

    /*gpio_direction_output(GPIO_BK_POWER_EN,0);
    msleep(100);
    gpio_direction_output(GPIO_LCD_PWR_EN,0);*/
}


void bk_power_ctrl(bool on)
{
    static int last_state = 2;

    printk("bk_power_ctrl = %d\n",last_state);

    if(last_state < 2){  //bk and lcd all set power on,will power on
        if(on != !!last_state){
            if(on){
                last_state = 1;
                gpio_set_value(GPIO_LCD_PWR_EN,1);
                msleep(150);
                gpio_set_value(GPIO_BK_POWER_EN,1);
                imapx200_gpio_cfgpin(GPIO_BK_PWM,2);  //to function mode
            }else{
                last_state = 0;
                gpio_direction_output(GPIO_BK_PWM,!GPIO_PWM_ON_VAL); //to gpio mode
                //gpio_set_value(GPIO_BK_POWER_EN,0);   //why: for flame?
                //imapx200_gpio_cfgpin(GPIO_BK_PWM,1);  //not need
                msleep(20);
                gpio_set_value(GPIO_LCD_PWR_EN,0);
            }
        }
    }else if(last_state == 2){    
        last_state =0;
        gpio_direction_output(GPIO_BK_PWM,0);  //to gpio mode
        gpio_set_value(GPIO_BK_POWER_EN,0);
        //imapx200_gpio_cfgpin(GPIO_BK_PWM,1);
        msleep(20);
        gpio_set_value(GPIO_LCD_PWR_EN,0);
    }else{
        last_state --;
    }
}

static void imapbl_write_intensity(int intensity,int update)
{
	int g_tcmp;

    //printk("set intensity %d ",intensity);

    if(update){
        bk_power_ctrl(!!intensity);
    }
	
	g_tcmp = intensity*100/255;
	if (g_tcmp > 100)
		g_tcmp = 100;
	
	else if (g_tcmp <5){
	}
		//g_tcmp -= 5;
		
	else if(g_tcmp < 20) {
		g_tcmp -= 5;
	}
	else {
	}

#if (defined(CONFIG_BOARD_E5)||     \
        defined(CONFIG_BOARD_F0)||defined(CONFIG_BOARD_I0)||defined(CONFIG_BOARD_J0))
    g_tcmp = 100 - g_tcmp;
#endif
#ifdef CONFIG_IMAP_PRODUCTION
	//printk("now, adjust the backlight, g_tcmp1 is %d!\n",g_tcmp);
	imap_timer_setup(2,100*500,g_tcmp*500);
#else
	imap_timer_setup(2,100,g_tcmp);
#endif
	return;
}
	
static void imapbl_send_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);

	if(bd->props.power != FB_BLANK_UNBLANK)
		intensity = 0;
	if(bd->props.fb_blank != FB_BLANK_UNBLANK)
		intensity = 0;
	if(imapbl_suspended)
		intensity = 0;
	mutex_lock(&bl_mutex);
	imapbl_write_intensity(intensity,
          devdata->current_intensity!=intensity && (!devdata->current_intensity || !intensity));
	mutex_unlock(&bl_mutex);
	devdata->current_intensity = intensity ;
}

static int imapbl_set_intensity(struct backlight_device *bd)
{
	imapbl_send_intensity(bd);
	//printk(KERN_INFO "BRIGHT SET OK\n");
	return 0;
}


/* set and get funcs for ANDROID */
static void imapbl_leds_set_brightness(struct led_classdev *led_cdev,
   int brightness)
{

//	printk(KERN_INFO "Calling with brightness %d\n", brightness);
	mutex_lock(&bl_mutex);
	imapbl_write_intensity(brightness,imapbl_cur_brightness!=brightness && (!imapbl_cur_brightness || !brightness));
	imapbl_cur_brightness = brightness;
	mutex_unlock(&bl_mutex);
}   

static int imapbl_leds_get_brightness(struct led_classdev *led_cdev)
{
	return imapbl_cur_brightness;
}

static int imapbl_get_intensity(struct backlight_device *bd)
{
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);
	return devdata->current_intensity;
}

static struct backlight_ops imapbl_ops = {
	.get_brightness = imapbl_get_intensity,
	.update_status 	= imapbl_set_intensity,
};

#ifdef CONFIG_PM

static int imapbl_suspend(struct platform_device *pdev,pm_message_t state)
{
	/* XXX ANDROID XXX */
#if 0 
	struct backlight_device *bd = platform_get_drvdata(pdev);
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);

	devdata->suspend = 1;
	imapbl_send_intensity(bd);
#endif
	imap_pwm_suspend(NULL, state);
	return 0;
}

static int imapbl_resume(struct platform_device *pdev)
{
	/* XXX ANDROID XXX */
#if 0 
	struct	backlight_device *bd = platform_get_drvdata(pdev);
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);

	devdata->suspend = 0;
	imapbl_send_intensity(bd);
#endif
	imap_pwm_resume(NULL);
	return 0;
}

#else

#define  imapbl_suspend NULL
#define	 imapbl_resume  NULL

#endif
static struct led_classdev imap_bl_cdev = {
	.name = "lcd-backlight",
	.brightness_set = imapbl_leds_set_brightness,
	.brightness_get = imapbl_leds_get_brightness,
};

static  int imapbl_probe(struct platform_device *pdev)
{
	int error;
	struct backlight_device *bd;
	struct imapbl_data *devdata;
	//uint32_t gpf8;

	devdata = kzalloc(sizeof(struct imapbl_data),GFP_KERNEL);
	if(!devdata)
		return -ENOMEM;

	/* Config GPF8 */
	/*
	gpf8 = readl(rGPFCON);
#if defined(CONFIG_LCD_FOR_PRODUCT)
	gpf8 &= ~(0x3 << 12);
	gpf8 |= (0x2 << 12);
#else
	gpf8 &= ~(0x3 << 16);
	gpf8 |= (0x2 << 16);               
#endif        
	writel(gpf8, rGPFCON);*/


	bd = backlight_device_register ("lcd-backlight",&pdev->dev,devdata,
			&imapbl_ops);

	if(IS_ERR(bd))
		return PTR_ERR(bd);

	/* Register LEDS for ANDROID GUI backlight control */
	/**                                                                                           
	 * led_classdev_register - register a new object of led_classdev class.                       
	 * @parent: The device to register.                                                           
	 * @led_cdev: the led_classdev structure for this device.                                     
	 */                                                                                           
	error = led_classdev_register(&pdev->dev, &imap_bl_cdev);
	if(error)
	{
		printk(KERN_INFO "Registe leds lcd-backlight failed.\n");
	}

    bk_power_init();

	platform_set_drvdata(pdev,bd);
	bd->props.max_brightness = IMAP_MAX_INTENSITY;
	bd->props.brightness 	 = IMAP_DEFAULT_INTENSITY;
	imapbl_send_intensity(bd);
	
	return 0;
}

static int imapbl_remove(struct platform_device *pdev)
{
	struct backlight_device *bd = platform_get_drvdata(pdev);
	struct imapbl_data *devdata = dev_get_drvdata(&bd->dev);
	
	kfree(devdata);
	bd->props.brightness = 0;
	bd->props.power	= 0;
	imapbl_send_intensity(bd);
	backlight_device_unregister(bd);

	return 0;
}

static struct platform_driver imapbl_driver = {
	.probe		= imapbl_probe,
	.remove		= imapbl_remove,
	.suspend	= imapbl_suspend,
	.resume		= imapbl_resume,
	.driver		= {
		.name	= "imapx200-lcd-backlights",
		.owner  = THIS_MODULE,
	},
};

static int __init imapbl_init(void)
{
	int ret;
	ret = platform_driver_register(&imapbl_driver);
	if(ret)
		return ret;
	return 0;
}

static void __exit imapbl_exit(void)
{
	platform_driver_unregister(&imapbl_driver);
}



module_init(imapbl_init);
module_exit(imapbl_exit);

MODULE_AUTHOR("ronaldo, warits");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IMAPX200 Backlight Driver");

