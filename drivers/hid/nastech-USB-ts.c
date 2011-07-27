#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/usb.h>

MODULE_VERSION("0.6");
MODULE_AUTHOR("nastech by Ming");
MODULE_DESCRIPTION("nastech PC touchscreen Driver ");
MODULE_LICENSE("GPL");

#include "hid-ids.h"

//#define	USB_VENDOR_ID_NAS 0x03eb
//#define USB_DEVICE_ID_NASTECH 0x201c

#define	USB_VENDOR_ID_NAS 0x2101
#define USB_DEVICE_ID_NASTECH 0x1011
#define MY_DEBUG	0

struct nasFinger{
	__u16 x, y;
	__u8 id;
	__u8 rank;
	bool touch, valid;
	//bool valid;		/* valid finger data, or just placeholder? */
	//bool first;		/* is this the first finger in this frame? */
	//bool activity_now;	/* at least one active finger in this frame? */
	//bool activity;		/* at least one active finger previously? */
};

struct nasFinger_data {
	struct nasFinger finger[2];
	__u8 curid, num;
	bool touch, valid1;
	
	__u16 x, y;
	__u8 id;
	bool valid;		/* valid finger data, or just placeholder? */
	bool first;		/* is this the first finger in this frame? */
	bool activity_now;	/* at least one active finger in this frame? */
	bool activity;		/* at least one active finger previously? */
};
static void nas_filter_event(struct nasFinger_data *FingerData, struct input_dev *input);

static int nas_event(struct hid_device *hid, struct hid_field *field,struct hid_usage *usage, __s32 value)
{
	struct nasFinger_data *FingerData = hid_get_drvdata(hid);

	//printk("nas_event : usage->hid = 0x%x, value = 0x%d\r\n",usage->hid,value);

	if (hid->claimed & HID_CLAIMED_INPUT)
	{
		struct input_dev *input = field->hidinput->input;
		
		input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0,255, 0, 0);
		input_set_abs_params(input, ABS_MT_WIDTH_MAJOR, 0,15, 0, 0);
		input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN) ;

		switch (usage->hid)
		{
			case HID_DG_CONTACTMAX:
				//#if MY_DEBUG
				//	printk("|		HID_DG_CONTACTID\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				input_set_abs_params(input, ABS_MT_TRACKING_ID,0, 2, 0, 0);
				break;

			case HID_DG_TIPSWITCH:
				//#if MY_DEBUG
				//	printk("|		HID_DG_TIPSWITCH\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				FingerData->touch = value;
				FingerData->valid = !!value;
				break;
			case HID_DG_INRANGE:
				//#if MY_DEBUG
				//	printk("|		HID_DG_INRANGE\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				break;
			case HID_DG_CONFIDENCE:
				//#if MY_DEBUG
				//	printk("|		HID_DG_CONFIDENCE\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				FingerData->valid1 = value;
				break;
			case HID_DG_CONTACTID:
				//#if MY_DEBUG
				//	printk("|		HID_DG_CONTACTID\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				FingerData->curid = value;
				FingerData->finger[value-1].id = FingerData->curid;
				FingerData->finger[value-1].touch = FingerData->touch;
				FingerData->finger[value-1].valid = 1;
				
				FingerData->id = value;
				break;
			case HID_GD_X:
				//#if MY_DEBUG
				//	printk("|		HID_GD_X\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				if (FingerData->valid1)
					FingerData->finger[FingerData->curid-1].x = value;

				FingerData->x = value;
				break;
			case HID_GD_Y:
				//#if MY_DEBUG
				//	printk("|		HID_GD_Y\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				if (FingerData->valid1)
					FingerData->finger[FingerData->curid-1].y = value;
				FingerData->y = value;
				
				nas_filter_event(FingerData, input);
				break;
			case HID_DG_CONTACTCOUNT:
				//#if MY_DEBUG
				//	printk("|		HID_DG_CONTACTCOUNT\n");
				//	printk("|		value = %d , 0x%x\n",value,value);
				//#endif
				//nas_filter_event(FingerData, input);
				FingerData->first = false;
				FingerData->activity_now = false;
				break;
			/*case HID_DG_INRANGE:
				FingerData->valid = !!value;
				break;
			case HID_GD_X:
				FingerData->x = value;
				break;
			case HID_GD_Y:
				FingerData->y = value;
				nas_filter_event(FingerData, input);
				break;
			case HID_DG_CONTACTID:
				FingerData->id = value;
				break;
			case HID_DG_CONTACTCOUNT:
				// touch emulation: this is the last field in a frame
				FingerData->first = false;
				FingerData->activity_now = false;
				break;
			case HID_DG_CONFIDENCE:
			case HID_DG_TIPSWITCH:
				// avoid interference from generic hidinput handling
				break;
			case 0xff000002:
			*/
			default:
				// fallback to the generic hidinput handling
				return 1;
		}
	}

	/* we have handled the hidinput part, now remains hiddev */
	if (hid->claimed & HID_CLAIMED_HIDDEV && hid->hiddev_hid_event)
		hid->hiddev_hid_event(hid, field, usage, value);

	return 1;
}

static void nas_filter_event(struct nasFinger_data *FingerData, struct input_dev *input)
{
	struct nasFinger* oldFinger = 0;
	bool pressed = false, released = false;
	int i;
	goto	step1;
	
	struct nasFinger* finger1;
	struct nasFinger* finger2;
	finger1=&FingerData[0];
	finger2=&FingerData[1];
	if(finger1->touch)
	{
		input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 255);
		input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 15);
		input_event(input, EV_ABS, ABS_MT_TRACKING_ID, 0);
		input_event(input, EV_ABS, ABS_MT_POSITION_X, finger1->x);
		input_event(input, EV_ABS, ABS_MT_POSITION_Y, finger1->y);
		input_mt_sync(input);
		
		if(finger2->touch)
		{
			input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 255);
			input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 15);
			input_event(input, EV_ABS, ABS_MT_TRACKING_ID, 1);
			input_event(input, EV_ABS, ABS_MT_POSITION_X, finger2->x);
			input_event(input, EV_ABS, ABS_MT_POSITION_Y, finger2->y);
			input_mt_sync(input);
		}
		else
		{
			input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
			input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 0);
			input_event(input, EV_ABS, ABS_MT_TRACKING_ID, 1);
			input_event(input, EV_ABS, ABS_MT_POSITION_X, finger2->x);
			input_event(input, EV_ABS, ABS_MT_POSITION_Y, finger2->y);
			input_mt_sync(input);
		}
		
		input_event(input, EV_KEY, BTN_TOUCH, 1);
		input_event(input, EV_ABS, ABS_X, finger1->x);
		input_event(input, EV_ABS, ABS_Y, finger1->y);
	}
	else
	{
		input_event(input, EV_KEY, BTN_TOUCH, 0);
		input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 0);
	}
	return;
	
		
	/*for (i = 0; i < 2; ++i)
	{
		struct nasFinger* finger = &FingerData->finger[i];
		if (!finger->valid)
		{
			// this finger is just placeholder data, ignore
		}
		else if (finger->touch)
		{
			input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 255);
			input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 15);
			input_event(input, EV_ABS, ABS_MT_TRACKING_ID, i);
			input_event(input, EV_ABS, ABS_MT_POSITION_X, finger->x);
			input_event(input, EV_ABS, ABS_MT_POSITION_Y, finger->y);
			input_mt_sync(input);
			
			if (finger->rank == 0)
			{
				finger->rank = ++(FingerData->num);
				if (finger->rank == 1)
					pressed = true;
			}
			if (finger->rank == 1)
				oldFinger = finger;
		}
		else
		{
			int j;

			for (j = 0; j < 2; ++j)
			{
				struct nasFinger* finger1 = &FingerData->finger[j];
				if (finger1->rank > finger->rank)
				{
					finger1->rank--;
					if (finger1->rank == 1)
						oldFinger = finger1;
				}
			}
			finger->rank = 0;
			--(FingerData->num);
			if (FingerData->num == 0)
				released = true;
		}
		finger->valid=0;
	}
	if (oldFinger)
	{
		if (pressed)
			input_event(input, EV_KEY, BTN_TOUCH, 1);
		input_event(input, EV_ABS, ABS_X, oldFinger->x);
		input_event(input, EV_ABS, ABS_Y, oldFinger->y);
	}
	else if (released)
	{
		input_event(input, EV_KEY, BTN_TOUCH, 0);
		input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
		input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 0);
	}*/
step1:
	FingerData->first = !FingerData->first; // touchscreen emulation

	if (!FingerData->valid)
	{
		//
		//  touchscreen emulation: if no finger in this frame is valid
		//  and there previously was finger activity, this is a release
		// 
		if (!FingerData->first && !FingerData->activity_now && FingerData->activity)
		{
			input_event(input, EV_KEY, BTN_TOUCH, 0);
			input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 0);
			input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 0);
			FingerData->activity = false;
		}
		return;
	}

	input_event(input, EV_ABS, ABS_MT_TOUCH_MAJOR, 255);
	input_event(input, EV_ABS, ABS_MT_WIDTH_MAJOR, 15);
	input_event(input, EV_ABS, ABS_MT_TRACKING_ID, FingerData->id);
	input_event(input, EV_ABS, ABS_MT_POSITION_X, FingerData->x);
	input_event(input, EV_ABS, ABS_MT_POSITION_Y, FingerData->y);

	input_mt_sync(input);
	FingerData->valid = false;

	// touchscreen emulation: if first active finger in this frame... 
	if (!FingerData->activity_now)
	{
		// if there was no previous activity, emit touch event 
		if (!FingerData->activity)
		{
			input_event(input, EV_KEY, BTN_TOUCH, 1);
			FingerData->activity = true;
		}
		FingerData->activity_now = true;
		// and in any case this is our preferred finger 
		input_event(input, EV_ABS, ABS_X, FingerData->x);
		input_event(input, EV_ABS, ABS_Y, FingerData->y);
	}
}

static int nas_input_mapped(struct hid_device *hdev, struct hid_input *hi,struct hid_field *field, struct hid_usage *usage,unsigned long **bit, int *max)
{
	printk("nas_input_mapping , usage->type = 0x%x\r\n",usage->type);
	if (usage->type == EV_KEY || usage->type == EV_ABS)
		clear_bit(usage->code, *bit);
	return 0;
}
static int nas_input_mapping(struct hid_device *hdev, struct hid_input *hi,struct hid_field *field, struct hid_usage *usage,unsigned long **bit, int *max)
{
	printk("usage->hid & HID_USAGE_PAGE = 0x%x\r\n",(usage->hid & HID_USAGE_PAGE));
	printk("usage->hid = 0x%x\r\n",usage->hid);
	
	switch (usage->hid & HID_USAGE_PAGE)
	{
		case HID_UP_GENDESK:
			printk("nas_input_mapping , usage->hid = 0x%x , min = %d, max = %d\r\n"
				,usage->hid,field->logical_minimum,field->logical_maximum);
			switch (usage->hid)
			{
				case HID_GD_X:
					hid_map_usage(hi, usage, bit, max,EV_ABS, ABS_MT_POSITION_X);
					// touchscreen emulation
					input_set_abs_params(hi->input, ABS_X,field->logical_minimum,field->logical_maximum, 0, 0);
					//input_set_abs_params(hi->input, ABS_MT_POSITION_X,field->logical_minimum,field->logical_maximum, 0, 0);
					//input_set_abs_params(nas_input, ABS_HAT0X,field->logical_minimum,field->logical_maximum, 0, 0);
					input_set_abs_params(hi->input, ABS_MT_TOUCH_MAJOR, 0,255, 0, 0);
					input_set_abs_params(hi->input, ABS_MT_WIDTH_MAJOR, 0,15, 0, 0);
					return 1;
				case HID_GD_Y:
					hid_map_usage(hi, usage, bit, max,EV_ABS, ABS_MT_POSITION_Y);
					// touchscreen emulation
					input_set_abs_params(hi->input, ABS_Y,field->logical_minimum,field->logical_maximum, 0, 0);
					//input_set_abs_params(hi->input, ABS_MT_POSITION_Y,field->logical_minimum,field->logical_maximum, 0, 0);
					//input_set_abs_params(nas_input, ABS_HAT0Y,field->logical_minimum,field->logical_maximum, 0, 0);
					return 1;
			}
			return 0;
	
		case HID_UP_DIGITIZER:
			printk("nas_input_mapping , usage->hid = 0x%x, max = %d\r\n",usage->hid,*max);
			switch (usage->hid)
			{
				case HID_DG_CONFIDENCE:
				case HID_DG_TIPSWITCH:
				case HID_DG_INRANGE:
					/* touchscreen emulation */
					hid_map_usage(hi, usage, bit, max, EV_KEY, BTN_TOUCH);
					hi->input->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS) | BIT_MASK(EV_SYN) ;
					return 1;
				case HID_DG_CONTACTID:
					hid_map_usage(hi, usage, bit, max,EV_ABS, ABS_MT_TRACKING_ID);
					//input_set_abs_params(hi->input, ABS_MT_TRACKING_ID,0, 2, 0, 0);
					return 1;

				//case HID_DG_INPUTMODE:
				//case HID_DG_DEVICEINDEX:
				//case HID_DG_CONTACTCOUNT:
				//case HID_DG_CONTACTMAX:
				//case HID_DG_TIPPRESSURE:
				//case HID_DG_WIDTH:
				//	hid_map_usage(hi, usage, bit, max,EV_ABS, ABS_MT_TOUCH_MAJOR);
				//	input_set_abs_params(hi->input, ABS_MT_TOUCH_MAJOR, 0,255, 0, 0);
				//	input_set_abs_params(hi->input, ABS_MT_WIDTH_MAJOR, 0,15, 0, 0);
				//	return 1;
				//case HID_DG_HEIGHT:
				//	hid_map_usage(hi, usage, bit, max,EV_ABS, ABS_MT_TOUCH_MINOR);
				//	input_set_abs_params(hi->input, ABS_MT_ORIENTATION,1, 1, 0, 0);
				//	return 1;
			}
			return 0;
	
		case 0xff000000:
			/* ignore vendor-specific features */
			return -1;
	}

	return 0;
}
static int nas_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;
	struct nasFinger_data *FingerData;
	
	printk("nas_probe\r\n");
	
	FingerData = kmalloc(sizeof(struct nasFinger_data), GFP_KERNEL);
	if (!FingerData)
	{
		dev_err(&hdev->dev, "cannot allocate nastech Touch data\n");
		return -ENOMEM;
	}
	
	FingerData->valid = false;
	FingerData->activity = false;
	FingerData->activity_now = false;
	FingerData->first = false;
	
	FingerData->finger[0].rank=0;
	FingerData->finger[1].rank=0;
	FingerData->num=0;
	
	hid_set_drvdata(hdev, FingerData);

	ret = hid_parse(hdev);
	printk("nas_probe : ret = 0x%x\r\n",ret);
	if (!ret)
		ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT|HID_CONNECT_HIDINPUT_FORCE);

	if (ret)
		kfree(FingerData);
	printk("nas_probe is OK\r\n");
	return ret;
}
static void nas_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
	kfree(hid_get_drvdata(hdev));
}

static const struct hid_device_id nas_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_NAS, USB_DEVICE_ID_NASTECH) },
	{ }
};
MODULE_DEVICE_TABLE(hid, nas_devices);

static const struct hid_usage_id nas_grabbed_usages[] = {
	{ HID_ANY_ID, HID_ANY_ID, HID_ANY_ID },
	{ HID_ANY_ID - 1, HID_ANY_ID - 1, HID_ANY_ID - 1}
};

static struct hid_driver nas_driver = {
	.name = "nastech-usb-ts",
	.id_table = nas_devices,
	.probe = nas_probe,
	.remove = nas_remove,
	.input_mapping = nas_input_mapping,
	.input_mapped = nas_input_mapped,
	.usage_table = nas_grabbed_usages,
	.event = nas_event,
};


static int __init nas_init(void)
{
	printk("nas_init\r\n");
	return hid_register_driver(&nas_driver);
}

static void __exit nas_exit(void)
{
	hid_unregister_driver(&nas_driver);
}

module_init(nas_init);
module_exit(nas_exit);
MODULE_LICENSE("GPL");

