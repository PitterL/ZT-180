/*
 * * Fake Battery driver for android
 * *
 * * Copyright © 2009 Rockie Cheng <aokikyon@gmail.com>
 * *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 as
 * * published by the Free Software Foundation.
 * */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/jiffies.h>
#include <linux/sched.h>

#define BAT_STAT_PRESENT 0x01
#define BAT_STAT_FULL   0x02
#define BAT_STAT_LOW   0x04
#define BAT_STAT_DESTROY 0x08
#define BAT_STAT_AC   0x10
#define BAT_STAT_CHARGING 0x20
#define BAT_STAT_DISCHARGING 0x40

#define BAT_ERR_INFOFAIL 0x02
#define BAT_ERR_OVERVOLTAGE 0x04
#define BAT_ERR_OVERTEMP 0x05
#define BAT_ERR_GAUGESTOP 0x06
#define BAT_ERR_OUT_OF_CONTROL 0x07
#define BAT_ERR_ID_FAIL   0x09
#define BAT_ERR_ACR_FAIL 0x10

#define BAT_ADDR_MFR_TYPE 0x5F

static int android_ac_get_prop(struct power_supply *psy,
   enum power_supply_property psp,
   union power_supply_propval *val)
{

	switch (psp)
	{
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = BAT_STAT_AC;
			break;
		default:
			break;
	}
	return 0;
}

static enum power_supply_property android_ac_props[] =
{
	POWER_SUPPLY_PROP_ONLINE,
};

static struct power_supply android_ac =
{
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.properties = android_ac_props,
	.num_properties = ARRAY_SIZE(android_ac_props),
	.get_property = android_ac_get_prop,
};

static int android_bat_get_status(union power_supply_propval *val)
{

	val->intval = POWER_SUPPLY_STATUS_FULL;
	return 0;
}

static int android_bat_get_health(union power_supply_propval *val)
{

	val->intval = POWER_SUPPLY_HEALTH_GOOD;
	return 0;
}

static int android_bat_get_mfr(union power_supply_propval *val)
{

	val->strval = "Rockie";
	return 0;
}

static int android_bat_get_tech(union power_supply_propval *val)
{
	val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
	return 0;
}

static int android_bat_get_property(struct power_supply *psy,
   enum power_supply_property psp,
   union power_supply_propval *val)
{
	int ret = 0;

	switch (psp)
	{
		case POWER_SUPPLY_PROP_STATUS:
			ret = android_bat_get_status(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = BAT_STAT_PRESENT;
			break;

		case POWER_SUPPLY_PROP_HEALTH:
			ret = android_bat_get_health(val);
			break;
		case POWER_SUPPLY_PROP_MANUFACTURER:
			ret = android_bat_get_mfr(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			ret = android_bat_get_tech(val);
			if (ret)
			  return ret;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_AVG:
			val->intval = 3;
			break;
		case POWER_SUPPLY_PROP_CURRENT_AVG:
			val->intval = 3;
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = 100;
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

static enum power_supply_property android_bat_props[] = {
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

/*********************************************************************
 * *   Initialisation
 * *********************************************************************/

static struct platform_device *bat_pdev;

static struct power_supply android_bat =
{
	.properties = android_bat_props,
	.num_properties = ARRAY_SIZE(android_bat_props),
	.get_property = android_bat_get_property,
	.use_for_apm = 1,
};

static int __init android_bat_init(void)
{
	int ret = 0;

	bat_pdev = platform_device_register_simple("battery", 0, NULL, 0);

	ret = power_supply_register(&bat_pdev->dev, &android_ac);
	if (ret)
	  goto ac_failed;

	android_bat.name = bat_pdev->name;

	ret = power_supply_register(&bat_pdev->dev, &android_bat);
	if (ret)
	  goto battery_failed;

	goto success;

	power_supply_unregister(&android_bat);
battery_failed:
	power_supply_unregister(&android_ac);
ac_failed:
	platform_device_unregister(bat_pdev);
success:
	return ret;
}

static void __exit android_bat_exit(void)
{
	power_supply_unregister(&android_bat);
	power_supply_unregister(&android_ac);
	platform_device_unregister(bat_pdev);
}

module_init(android_bat_init);
module_exit(android_bat_exit);

MODULE_AUTHOR("Rockie Cheng <aokikyon@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fake Battery driver for android");
