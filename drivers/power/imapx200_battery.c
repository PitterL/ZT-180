/***************************************************************************** 
** XXX driver/power/imapx200_battery.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: imapx200 ADC based battery monitor driver.
**
** Author:
**     Warits   <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 04/06/2010 XXX	Warits
*****************************************************************************/

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/ioport.h>
#include <linux/i2c.h>

#include <asm/io.h>
#include <mach/imapx_gpio.h>
#include "imapx200_battery.h"
#include <linux/gpio.h>

//#define DEBUG

struct i2c_client * tsc2007_i2c;
struct tsc2007_setup_data {
	int i2c_bus;	
	unsigned short i2c_address;
};

static int bat_vol = 0;
static struct delayed_work zt_monitor_work;
static struct workqueue_struct *zt_monitor_wqueue;


struct mutex work_lock;
int bat_status = POWER_SUPPLY_STATUS_DISCHARGING;
static void imap_bat_update(struct power_supply *bat_ps);


extern uint32_t tsc2007_get_voltage(void);
static uint32_t GetVoltage(int type)
{
	return  tsc2007_get_voltage();
}


static uint32_t GetPowerAdc(void)
{
	uint32_t powerVot;
	uint32_t powerVotAvg = 0;
	uint32_t j;
	int cnt = 0;

	for (j = 0; j < 2; j++){
		powerVot = GetVoltage(0);
		if (powerVot){
			powerVotAvg += powerVot;
			cnt++;
		}
	}

	if (cnt)
	    powerVotAvg /= cnt;

	return powerVotAvg;
}

static uint32_t check_charge(void)
{
	uint32_t ret;
	
	ret = gpio_get_value(GPIO_PIN_NUM);

	return ret;
}

static uint32_t IsChargingFull(void)
{
    /*
	if( bat_vol > (BATT_FULL_VOL_VALUE-100) )
		return 1;
	else
		return 0;
    */

    return 0;  //not show full when charging
}

static int imap_bat_get_health(union power_supply_propval *val)
{

	val->intval = POWER_SUPPLY_HEALTH_GOOD;
	return 0;
}

static int imap_bat_get_mfr(union power_supply_propval *val)
{

	val->strval = "Rockie";
	return 0;
}

static unsigned int adc_to_voltage(unsigned int adc)
{
    unsigned int voltage,volhigh,vollow;
    unsigned int adchigh,adclow;
    unsigned int volstep,adcstep,adccompen;

    volhigh=BATT_FULL_VOL_VALUE;
    vollow=BATT_EMPTY_VOL_VALUE;

    adchigh = BATT_FULL_ADC_VALUE;
    adclow = BATT_EMPTY_ADC_VALUE;

    volstep=adcstep=adccompen=0;

#if defined(CONFIG_IMAPX_BATTERY_DEEP_DISCHARGE)
    volstep=BATT_DEEP_VOL_STEP_VALUE;
    adcstep=BATT_DEEP_ADC_STEP_VALUE;
#endif
#if defined(CONFIG_IMAPX_BATTERY_BAD_DISCHARGE)
    adccompen=BATT_BAD_DISCHARGE_ADC_COMPENSATION;
#endif


#if !defined(CONFIG_IMAPX_BATTERY_3V)
    adchigh >>= 1;
    adclow >>= 1;
    adc >>= 1;
#endif

    vollow -= volstep;
    adclow -= adcstep;
    adc += adccompen;


    if(adc<=adclow){
        voltage=vollow;
    }else if(adc>=adchigh){
        voltage=volhigh;
    }else{
        voltage=(unsigned int)(adc-adclow)*(volhigh-vollow)/(adchigh-adclow)+vollow;
    }
    return voltage;
}

struct energy_table{
    unsigned int  voltage;
    unsigned int  percent;
};

#if defined(CONFIG_IMAPX_BATTERY_DEEP_DISCHARGE)

const struct energy_table g_engery_table[]={
    {BATT_DEEP_EMPTY_VOL_VALUE,0},
    {3100,3},
    {3200,9},
    {3300,8},
    {3400,12},
    {3500,15},
    {3600,15},
    {3700,15},
    {3800,10},
    {3900,8},
    {4000,5},
    {4100,0},
    {BATT_FULL_VOL_VALUE,0},

    /*{BATT_DEEP_EMPTY_VOL_VALUE,0},
    {3100,0},
    {3200,0},
    {3250,1},
    {3300,2},
    {3400,5},
    {3500,8},
    {3600,21},
    {3700,30},
    {3800,16},
    {3900,12},
    {4000,5},
    {4050,0},
    {BATT_FULL_VOL_VALUE,0},*/

    {(unsigned int)-1,0},  //table end tag
};

#else

const struct energy_table g_engery_table[]={
    {BATT_EMPTY_VOL_VALUE,0},
    {3500,8},
    {3600,19},
    {3700,33},
    {3800,18},
    {3900,14},
    {4000,8},
    {4050,0},
    {BATT_FULL_VOL_VALUE,0},
    {(unsigned int)-1,0},  //table end tag
};

#endif

static int calculate_capacity(unsigned int vol)
{
    const struct energy_table *et;
    unsigned int percent;
    unsigned int step;

    unsigned int dw;
    
    et=g_engery_table;

    percent=0;
    dw=0;
    while(vol>et[dw].voltage){
        percent+=et[dw].percent;
        /*printk("%d %d>%d current step %d percent %d\n",
            dw,vol,et[dw].voltage,et[dw].percent,percent);*/
        dw++;
    }

    if(dw>0){        
        step=((((vol-et[dw-1].voltage)*
                    et[dw-1].percent))<<8)/
                    (et[dw].voltage-et[dw-1].voltage);
        step>>=8;            
        percent-=et[dw-1].percent;
        percent+=step;

        /*
        printk("dw %d step=%d full=%d percent=%d\n",
            dw,
            step,
            et[dw-1].percent,
            percent);*/
    }

    return (int)percent;
}


static int imap_bat_get_capacity(void)
{
	uint32_t adc;
    unsigned int vol;
    int percent;

    adc = GetPowerAdc();

    vol = adc_to_voltage(adc);

	percent = calculate_capacity(vol);

#if !defined(CONFIG_IMAPX_BATTERY_3V)
    vol <<=1;
#endif
    bat_vol = vol;

#if defined(DEBUG)    
    printk("adc %d vol %d percent %d\n",adc,vol,percent);
#endif
    return percent;
}

static int imap_bat_get_tech(union power_supply_propval *val)
{
	val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
	return 0;
}

static int imap_bat_get_prop(struct power_supply *psy,
   enum power_supply_property psp,
   union power_supply_propval *val)
{
	int ret = 0;

	switch (psp)
	{
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = bat_status;
			return 0;
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			//val->intval = imap_bat_get_present();
			val->intval = bat_vol;
			break;

		case POWER_SUPPLY_PROP_HEALTH:
			ret = imap_bat_get_health(val);
			break;
		case POWER_SUPPLY_PROP_MANUFACTURER:
			ret = imap_bat_get_mfr(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			ret = imap_bat_get_tech(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_AVG:
			// val->intval = GetPowerAdc();
			val->intval = bat_vol;
			break;
		case POWER_SUPPLY_PROP_CURRENT_AVG:
			val->intval = bat_vol;
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = imap_bat_get_capacity();
			
			if(bat_status == POWER_SUPPLY_STATUS_CHARGING){
                if(val->intval<5)
                    val->intval = 5;
            }
#if defined(DEBUG)                
			val->intval = 100;
#endif
			break;
		case POWER_SUPPLY_PROP_TEMP:
			val->intval = 50;
			break;
		case POWER_SUPPLY_PROP_TEMP_AMBIENT:
			val->intval = 50;
			break;
		case POWER_SUPPLY_PROP_CHARGE_COUNTER:
			val->intval = 10;
			break;
		case POWER_SUPPLY_PROP_SERIAL_NUMBER:
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}

static int imap_ac_get_prop(struct power_supply *psy,
   enum power_supply_property psp,
   union power_supply_propval *val)
{

	switch (psp)
	{
		case POWER_SUPPLY_PROP_ONLINE:
			if (check_charge())
			{
				val->intval = POWER_SUPPLY_STATUS_CHARGING;
			}
			else{
				val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
			}
			//if (check_charge())
			//val->intval = imap_bat_get_type();
			break;
		default:
			break;
	}
	return 0;
}


static enum power_supply_property imap_bat_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_AVG,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_AMBIENT,
	POWER_SUPPLY_PROP_MANUFACTURER,
	POWER_SUPPLY_PROP_SERIAL_NUMBER,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
};
static void imap_bat_external_power_changed(struct power_supply *bat_ps)
{
	//schedule_work(&bat_work);
	cancel_delayed_work(&zt_monitor_work);
	queue_delayed_work(zt_monitor_wqueue, &zt_monitor_work, HZ/10);
}
/*********************************************************************
 * *   Initialisation
 * *********************************************************************/

static struct platform_device *bat_pdev;

static struct power_supply imap_bat =
{
	.name			= "battery",  // used in android "com_android_server_BatteryService.cpp"
	.type			= POWER_SUPPLY_TYPE_BATTERY,
	.properties = imap_bat_props,
	.num_properties = ARRAY_SIZE(imap_bat_props),
	.get_property = imap_bat_get_prop,
	.external_power_changed = imap_bat_external_power_changed,
	.use_for_apm = 1,
};

static enum power_supply_property imap_ac_props[] =
{
	POWER_SUPPLY_PROP_ONLINE,
};

static struct power_supply imap_ac =
{
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = imap_ac_props,
	.num_properties = ARRAY_SIZE(imap_ac_props),
	.get_property = imap_ac_get_prop,
};

static const struct i2c_device_id tsc2007_i2c_id[] = {
	{ "tsc2007", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tsc2007_i2c_id);


static void imap_bat_update(struct power_supply *bat_ps)
{
	mutex_lock(&work_lock);
	
	// high -> charging
	if( check_charge() ) {
		bat_status = POWER_SUPPLY_STATUS_CHARGING;
		if (IsChargingFull()){
			bat_status = POWER_SUPPLY_STATUS_FULL;	
		}
	} else {
		bat_status = POWER_SUPPLY_STATUS_NOT_CHARGING;	
	}
	//printk("imap_bat_update : bat_status = %d\r\n",bat_status);
	power_supply_changed(bat_ps);

	mutex_unlock(&work_lock);
}

static void zt_battery_work(struct work_struct *work)
{
	const int interval = HZ * 15; //15

	imap_bat_update(&imap_bat);
	queue_delayed_work(zt_monitor_wqueue, &zt_monitor_work, interval);
}


static int __devinit imap_bat_probe(struct platform_device *dev)
{
	int ret = 0;

	mutex_init(&work_lock);

	if (gpio_request(GPIO_PIN_NUM, "panel power")) {
		printk("panel power is error\r\n");
		return -1;
	}

	ret = power_supply_register(&dev->dev, &imap_ac);
	if (ret)
	  goto ac_failed;

	imap_bat.name = dev->name;
	
	//INIT_WORK(&bat_work, imap_bat_work);
	ret = power_supply_register(&dev->dev, &imap_bat);
	if (ret)
	  goto battery_failed;	

	
	INIT_DELAYED_WORK(&zt_monitor_work, zt_battery_work);
	zt_monitor_wqueue = create_singlethread_workqueue(dev_name(&dev->dev));
	if (!zt_monitor_wqueue) {
		ret = -ESRCH;
		goto workqueue_failed;
	}
	queue_delayed_work(zt_monitor_wqueue, &zt_monitor_work, HZ * 1);	

	goto success;

workqueue_failed:
	power_supply_unregister(&imap_bat);
battery_failed:
	power_supply_unregister(&imap_ac);
ac_failed:
	platform_device_unregister(dev);
success:
	return ret;
}
static int __devexit imap_bat_remove(struct platform_device *dev)
{
	cancel_rearming_delayed_workqueue(zt_monitor_wqueue,
					  &zt_monitor_work);
	destroy_workqueue(zt_monitor_wqueue);

	power_supply_unregister(&imap_bat);
	power_supply_unregister(&imap_ac);
	return 0;
}
#ifdef CONFIG_PM
static int imap_bat_suspend(struct platform_device *dev, pm_message_t state)
{
	flush_scheduled_work();
	return 0;
}

static int imap_bat_resume(struct platform_device *dev)
{
	cancel_delayed_work(&zt_monitor_work);
	queue_delayed_work(zt_monitor_wqueue, &zt_monitor_work, HZ);	
	return 0;
}
#else
#define imap_bat_suspend NULL
#define imap_bat_resume NULL
#endif
static struct platform_driver imap_bat_driver = {
	.driver.name	= "battery",
	.driver.owner	= THIS_MODULE,
	.probe		= imap_bat_probe,
	.remove		= __devexit_p(imap_bat_remove),
	.suspend	= imap_bat_suspend,
	.resume		= imap_bat_resume,
};

static int __init imap_bat_init(void)
{
	return platform_driver_register(&imap_bat_driver);
}


static void __exit imap_bat_exit(void)
{		
	platform_device_unregister(bat_pdev);
}

module_init(imap_bat_init);
module_exit(imap_bat_exit);

MODULE_AUTHOR("yqcui@zinithink.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Battery driver for zenithink");
