static int  cam_switch_low_svga(void)
{
	int i, ret;	

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
		
	for(i = 0; i < ((sizeof(ov2655_svga_low_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_svga_low_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to low svga :i=%d\n",i);
		}
	}
	mdelay(30);

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int  cam_switch_high_svga(void)
{
	int i, ret;
	
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_svga_high_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_svga_high_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to high svga :i=%d\n",i);
		}
	}
	mdelay(30);
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int  cam_switch_high_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_high_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_high_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to high xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int cam_switch_upmid_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_upmid_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_upmid_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to mid xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int  cam_switch_mid_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_mid_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_mid_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to mid xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int cam_svga_to_xuga(void){
/********************************************\
				STOP PREVIEW
\********************************************/
	int i, ret;
	char gain = 0;

	char reg0x3013;
	char reg0x3002, reg0x3003;
	char reg0x3000;
#ifdef D50HZ
	char reg0x3070, reg0x3071;
#else
	char reg0x3072, reg0x3073;
#endif

	uint32_t shutter;
	uint32_t extra_lines;
	uint32_t preview_exposure;
	uint32_t preview_gain16;
	uint32_t preview_dummy_pixel;
	uint32_t capture_dummy_pixel;
	uint32_t capture_dummy_line;
	uint32_t preview_pclk_frequency;
	uint32_t capture_pclk_frequency;
	uint32_t capture_max_gain;
	uint32_t capture_max_gain16;
	uint32_t preview_line_width;
	uint32_t capture_line_width;
	uint32_t capture_maximum_shutter;
	uint32_t capture_exposure;
	uint32_t preview_banding_filter;
	uint32_t capture_banding_filter = 0; 
	uint32_t gain_exposure;
	uint32_t capture_gain16;
	uint32_t capture_gain;

	struct ov2655_regval_list ov2655_stop_preview[] = {
		{0x30, 0x13, 0x00},
	};
	struct ov2655_regval_list ov2655_3002[] = {
		{0x30, 0x02, 0x00},
	};
	struct ov2655_regval_list ov2655_3003[] = {
		{0x30, 0x03, 0x00},
	};
	struct ov2655_regval_list ov2655_3000[] = {
		{0x30, 0x00, 0x00},
	};

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);

	// stop AE/AG
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3013), &reg0x3013, 1);
	reg0x3013 = reg0x3013 & 0xfa;
        ov2655_stop_preview->value	= reg0x3013;	

	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_stop_preview), SENSOR_OV2655);
	if(ret)
	{
		camif_error("Failed to transfer data to i2c\n");
		return -1;
	}
	else{
		//printk("stop AE/AG\n");
	}

	// read back preview shutter
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3002), &reg0x3002, 1);
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3003), &reg0x3003, 1);
	shutter = (((uint32_t)reg0x3002)<<8) + (uint32_t)reg0x3003;
	//printk(" shutter %x\n", shutter);

	// preview exposure
	preview_exposure = shutter;
	//printk(" preview_exposure %x\n", preview_exposure);

	// read back gain for preview
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3000), &reg0x3000, 1);
	preview_gain16 = (uint32_t)(16 + (reg0x3000 & 0x0f));
	preview_gain16 = (preview_gain16 * (((reg0x3000 & 0x80)>>7)+1) * (((reg0x3000 & 0x40)>>6)+1) * (((reg0x3000 & 0x20)>>5)+1) * (((reg0x3000 & 0x10)>>4)+1));
	//printk(" preview_gain16 %x\n", preview_gain16);

/********************************************\
		CALCULATE CAPTURE EXPOSURE	
\********************************************/
	// dummy pixel and dummy line could be insert for capture
	preview_dummy_pixel = 0;
	capture_dummy_pixel = 0;
	capture_dummy_line = 0;
	preview_pclk_frequency = 540;  	// 	*2
	capture_pclk_frequency = 324;	//	*1
//	preview_pclk_frequency = 1;
//	capture_pclk_frequency = 1;
	
	capture_max_gain   = 1;		// from 1x to 32x
	capture_max_gain16 = capture_max_gain * 16;
	preview_line_width = 1940 + preview_dummy_pixel;
	capture_line_width = 1940 + capture_dummy_pixel;
	capture_maximum_shutter = 1236 + capture_dummy_line;
	capture_exposure = preview_exposure * capture_pclk_frequency * preview_line_width;
	//printk(" capture_exposure %x\n", capture_exposure);
	capture_exposure = capture_exposure / (preview_pclk_frequency * capture_line_width);
	//printk(" capture_exposure %x\n", capture_exposure);

	// calculate banding filter
#ifdef D50HZ
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3070), &reg0x3070, 1);
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3071), &reg0x3071, 1);
	preview_banding_filter = (((uint32_t)reg0x3071)<<8) + (uint32_t)reg0x3070;
	//printk(" preview_banding_filter %x\n", preview_banding_filter);
#else
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3072), &reg0x3072, 1);
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3073), &reg0x3073, 1);
	preview_banding_filter = (((uint32_t)reg0x3073)<<8) + (uint32_t)reg0x3072;
	//printk(" preview_banding_filter %x\n", preview_banding_filter);
#endif

//	capture_banding_filter = preview_banding_filter * capture_pclk_frequency;
//  	capture_banding_filter = capture_banding_filter * preview_line_width;
//	capture_banding_filter = capture_banding_filter / preview_pclk_frequency;
//	capture_banding_filter = capture_banding_filter / capture_line_width;
	capture_banding_filter = 84;
	//printk(" capture_banding_filter %x\n", capture_banding_filter);

	// redistribute gain and exposure
	gain_exposure = preview_gain16 * capture_exposure;
	//printk(" gain_exposure %x\n", gain_exposure);
	if( gain_exposure < capture_banding_filter * 16 ) {
		capture_exposure = gain_exposure / 16;
		capture_gain16 = (gain_exposure*2 + 1)/capture_exposure;
		capture_gain16 = capture_gain16 >> 1;
		//printk(" -------------------1\n");
	}
	else {
		if( gain_exposure > capture_maximum_shutter * 16 ) {
			capture_exposure = capture_maximum_shutter;
			//printk(" capture_exposure %x\n", capture_exposure);
			capture_gain16 = (gain_exposure*2 + 1)/capture_maximum_shutter;
			capture_gain16 = capture_gain16 >> 1;
			if( capture_gain16 > capture_max_gain16 ) {
//				capture_exposure = (gain_exposure + (gain_exposure/10))/capture_max_gain16;
//				capture_exposure = capture_exposure/16;
//				capture_exposure = capture_exposure/capture_banding_filter;
//				capture_exposure = capture_exposure*capture_banding_filter;
				capture_exposure = (gain_exposure + (gain_exposure/10)) * capture_banding_filter;
				capture_exposure = capture_exposure / (16 * capture_max_gain16 * capture_banding_filter);	
				capture_gain16 = (gain_exposure*2 + 1)/capture_exposure;
				capture_gain16 = capture_gain16 >> 1;
				//printk(" -------------------2\n");
			}
			else {
//				capture_exposure = capture_exposure/16;
//				capture_exposure = capture_exposure/capture_banding_filter;
//				capture_exposure = capture_exposure*capture_banding_filter;
				capture_exposure = capture_exposure * capture_banding_filter;
				capture_exposure = capture_exposure / capture_banding_filter;
				capture_gain16 = (gain_exposure*2 + 1)/capture_maximum_shutter;
				capture_gain16 = capture_gain16 >> 1;
				//printk(" -------------------3\n");
			}
		}
		else {
//			capture_exposure = gain_exposure/16;
//			capture_exposure = capture_exposure/capture_banding_filter;
//			capture_exposure = capture_exposure*capture_banding_filter;
			capture_exposure = gain_exposure * capture_banding_filter;
			capture_exposure = capture_exposure / (16 * capture_banding_filter);
			capture_gain16 = (gain_exposure*2 + 1)/capture_exposure;
			capture_gain16 = capture_gain16 >> 1;
			//printk(" -------------------4\n");
		}
	}
	//printk(" capture_exposure %x\n", capture_exposure);
	//printk(" capture_gain16 %x\n", capture_gain16);

/********************************************\
			   SWITCH TO XUGA		
\********************************************/
	for(i = 0; i < ((sizeof(ov2655_xuga_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
//			//printk(KERN_ERR "to xuga:i=%x\n",i);
		}
	}

/********************************************\
			   WRITE REGISTRES 
\********************************************/
	// write exposure
	if( capture_exposure > capture_maximum_shutter ) {
		shutter = capture_maximum_shutter;
		extra_lines = capture_exposure - capture_maximum_shutter;
	}
	else {
		shutter = capture_exposure;
		extra_lines = 0;
	}
	reg0x3003 = shutter & 0x00ff;
	reg0x3002 = (shutter >>8) & 0x00ff;
	ov2655_3002->value = reg0x3002;
	ov2655_3003->value = reg0x3003;
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3002), SENSOR_OV2655);
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3003), SENSOR_OV2655);
	//printk(" ov2655_3002 %x\n ov2655_3003 %x\n", ov2655_3002->value, ov2655_3003->value);
	if(ret)
	{
		camif_error("Failed to transfer data to i2c\n");
		return -1;
	}
	else{
		//printk("write exposure\n");
	}

	// write gain
	capture_gain = capture_gain16;
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x10;
	}
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x20;
	}
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x40;
	}
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x80;
	}
	gain = gain | (char)(capture_gain - 16);
	ov2655_3000->value = gain;
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3000), SENSOR_OV2655);
	//printk(" ov2655_3000 %x\n", ov2655_3000->value);
	if(ret)
	{
		camif_error("Failed to transfer data to i2c\n");
		return -1;
	}
	else{
		//printk("write gain\n");
	}

	mdelay(30);
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);

	return 0;
}


static int cam_get_3008(void){
	char buf;

	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3000), &buf,1);
	//printk(" OV2655_3000 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3002), &buf,1);
	//printk(" OV2655_3002 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3003), &buf,1);
	//printk(" OV2655_3003 %x\n",buf);                                 
/*	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_301c), &buf,1);
	//printk(" OV2655_301c %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3070), &buf,1);
	//printk(" OV2655_3070 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3013), &buf,1);
	//printk(" OV2655_3013 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3014), &buf,1);
	//printk(" OV2655_3014 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3028), &buf,1);
	//printk(" OV2655_3028 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3029), &buf,1);
	//printk(" OV2655_3029 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302a), &buf,1);
	//printk(" OV2655_302a %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302b), &buf,1);
	//printk(" OV2655_302b %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302d), &buf,1);
	//printk(" OV2655_302d %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302e), &buf,1);
	//printk(" OV2655_302e %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3011), &buf,1);
	//printk(" OV2655_3011 %x\n",buf);                                 
*/	return 0;
}


static int cam_to_preview(void)
{
	int i, ret;
	char reg0x3013;
	struct ov2655_regval_list ov2655_3013[] = {
		{0x30, 0x13, 0x00},
	};
	
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3013), &reg0x3013,1);
	reg0x3013 = reg0x3013 | 0x05;   
	ov2655_3013->value = reg0x3013;
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3013), SENSOR_OV2655);                       

	for(i = 0; i < ((sizeof(ov2655_to_preview_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_to_preview_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to preview xuga:i=%d\n",i);
		}
	}
	return 0;
}


static int  cam_switch_low_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_low_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_low_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to low xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int cam_init(void)
{
	char buf = 0;
	int i, ret;
	uint32_t tmp;

    //set camara en here
    gpio_set_value(GPIO_CAMARA_EN,CAMARA_POWER_ON);

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_init_regs[0]), SENSOR_OV2655);
	msleep(50);

	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_PIDH), &buf, 1);
	if(buf != 0x26)
		//printk("i2c config OV2655 wrong with %d\n",buf); 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_PIDL), &buf, 1);
	if(buf != 0x56)
		//printk("i2c config OV2655 wrong with %d\n",buf); 

	for(i = 0; i < ((sizeof(ov2655_init_regs) / 3) -1); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_init_regs[i+1]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk("i=%d\n",i);
		}
	}
	return 0;

}

static int cam_close(void)
{
	int i, ret;
	uint32_t tmp;

	for(i = 0; i < ((sizeof(ov2655_stop_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_stop_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to xuga:i=%d\n",i);
		}
	}
    //need close power here
    gpio_set_value(GPIO_CAMARA_EN,CAMARA_POWER_OFF);

	return 0; 
} 

/*****************************************************\
 *************** Special Effects *********************
\*****************************************************/ 

static int cam_sepia(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sepia_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sepia_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sepia:i=%d\n",i);
		}
	} 
	
	return 0;
}  

static int cam_bluish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_bluish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_bluish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to bluish:i=%d\n",i);
		}
	} 
	
	return 0;
} 

static int cam_greenish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_greenish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_greenish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to greenish:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_reddish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_reddish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_reddish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to reddish:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_yellowish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_yellowish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_yellowish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to yellowish:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_bandw(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_bandw_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_bandw_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to bandw:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_negative(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_negative_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_negative_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to negative:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_normal(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_normal_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_normal_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to normal:i=%d\n",i);
		}
	} 
	
	return 0;
}

/*****************************************************\
 *************** Light Mode **************************
\*****************************************************/ 

static int cam_auto(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_auto_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_auto_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to auto:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_sunny(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sunny_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sunny_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sunny:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_cloudy(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_cloudy_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_cloudy_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to cloudy:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_office(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_office_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_office_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to office:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int cam_home(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_home_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_home_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to home:i=%d\n",i);
		}
	} 
	
	return 0;
}

/*****************************************************\
 *************** Color Saturation ********************
\*****************************************************/ 

static int cam_saturation_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation +2:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int cam_saturation_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation +1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int cam_saturation_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation +0:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_saturation_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation -1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_saturation_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation -2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Brightness **************************
\*****************************************************/ 

static int cam_brightness_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +3:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int cam_brightness_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int cam_brightness_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_brightness_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +0:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_brightness_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness -1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_brightness_5(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_5_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_5_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness -2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_brightness_6(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_6_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_6_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness -3:=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Contrast ****************************
\*****************************************************/
		
static int cam_contrast_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +3:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int cam_contrast_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int cam_contrast_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_contrast_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +0:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_contrast_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast -1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_contrast_5(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_5_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_5_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast -2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_contrast_6(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_6_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_6_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast -3:=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Sharpness ***************************
\*****************************************************/
                                                        
static int cam_sharpness_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 0:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int cam_sharpness_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int cam_sharpness_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_sharpness_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 3:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_sharpness_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 4:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int cam_sharpness_auto(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_auto_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_auto_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness auto:=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Night Mode **************************
\*****************************************************/
                                                        
static int cam_night_mode_on(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_night_mode_on_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_night_mode_on_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to night mode on:=%d\n",i);
		}
	} 
	
	return 0 ;
}
                                                        
static int cam_night_mode_off(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_night_mode_off_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_night_mode_off_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to night mode off:=%d\n",i);
		}
	} 
	
	return 0 ;
}

