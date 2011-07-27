#include <linux/delay.h>
#include "EP932Controller.h"  // HDMI Transmiter
#include "EP932api.h"


static EP932C_REGISTER_MAP EP932C_Registers;


/*
============================================================================
	need use customer's function 
		
	1. EP932 reset - use customer's GPIO function 

		EP_EP932M_Reset()
		
		### EP932 must reset after video format timing change ###
			
	2. EP932 IIC/DDC - use customer's IIC function

		DDC_If.c  	=> 	DDC_Write(......) , DDC_Read(.......)
		EP932_If.c	=> 	IIC_Write(......) , IIC_Read(.......)

		### customer' IIC function must can check IIC error (no ack, write error, read error) ###
	
	3. Timer 

		10ms timer to run EP932Controller_Task(); and EP932Controller_Timer();


   ============================================================================
	initial EP932 

	1. initial EP932 variable and customer's GPIO + I2C (if need).

		EP_HDMI_Init();

	2. set video interface and timing, timing need modify to fit with customer's require

		EP_HDMI_Set_Video_Timing( 3 );   // 480P

		EP_HDMI_Set_Video_Output(Video_OUT_Auto);

	3. set audio interface

		EP_HDMI_SetAudFmt(AUD_I2S, AUD_SF_48000Hz);

	4. need to run EP932Controller_Task and EP932Controller_Timer every 10ms

		while(1)
		{
			EP932Controller_Task();
			EP932Controller_Timer();
		}

============================================================================
*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EP932_EnableHPInit(void)
{
	EP932_Reg_Set_Bit(EP932_General_Control_1,EP932_General_Control_1__TSEL_HTP);
	EP932_Reg_Set_Bit(EP932_General_Control_1,EP932_General_Control_1__INT_OD);
	//EP932_Reg_Set_Bit(EP932_General_Control_1,EP932_General_Control_1__EDGE);
	EP932_Reg_Clear_Bit(EP932_General_Control_1,EP932_General_Control_1__EDGE);
	EP932_Reg_Clear_Bit(EP932_General_Control_1,EP932_General_Control_1__INT_POL);
	EP932_Reg_Clear_Bit(EP932_General_Control_2, EP932_General_Control_2__RIE);
	EP932_Reg_Clear_Bit(EP932_General_Control_2, EP932_General_Control_2__VIE);
	EP932_Reg_Set_Bit(EP932_General_Control_2, EP932_General_Control_2__MIE);
	//EP932_Reg_Clear_Bit(EP932_General_Control_2, EP932_General_Control_2__MIE);

}

void EP_EP932M_Reset(void)
{
	////////////////////////////////////////////////////////////////
	// need use customer's GPIO to reset EP932 after video timing change
	//
	// 1. GPIO set to low level
	// 2. delay 200ms
	// 3. GPIO set to high level
	//
	///////////////////////////////////////////////////////////////

    //hdmi reset will cause iic bus locked

#if 0   //move to screen off
    //local_irq_disable();
    
    gpio_set_value(GPIO_HDMI_RESET_PIN,0);

    
    //for(i = 0;i<200;i++)
    //    udelay(1000);
    msleep(100);

    gpio_set_value(GPIO_HDMI_RESET_PIN,1);
    //local_irq_enable();
#endif
}

void hdmi_main (LCD_TIMING timing)
{
	unsigned int i=0;
	//////////////////////////////////////////////////////
	//
	// initial EP932 variable and customer's GPIO + I2C (if need).
	//
	/////////////////////////////////////////////////////
	EP_HDMI_Init();

	/////////////////////////////////////////////////////
	//
	// set video interface and timing, timing need modify to fit with customer's require
	//
	/////////////////////////////////////////////////////
	EP_HDMI_Set_Video_Timing(timing);	

	EP_HDMI_Set_Video_Output(Video_OUT_YUV_444);
	////////////////////////////////////////////////////
	//
	// set audio interface
	//
	///////////////////////////////////////////////////
	EP_HDMI_SetAudFmt(AUD_I2S, AUD_SF_44100Hz);

	//////////////////////////////////////////////////////////////////
	//
	// need to run EP932Controller_Task and EP932Controller_Timer every 10ms
	//
	/////////////////////////////////////////////////////////////////
	while(1)
	{
		i++;
		if(i>200)
			break;

		EP932Controller_Task();
		EP932Controller_Timer();
	}
}

void  EP_HDMI_SetAudFmt(HDMI_AudFmt_t  Audfmt, HDMI_AudFreq  Audfreq)
{
	if(Audfmt == AUD_I2S)
	{
		EP932C_Registers.Audio_Interface = 0x18;		// 2 channel IIS
		DBG_printf(("Audio interface is IIS - 2.0 CH, "));
	}
	else
	{
		EP932C_Registers.Audio_Interface = 0x10;		// SPDIF
		DBG_printf(("Audio interface is SPDIF, "));
	}

    //EP932C_Registers.System_Configuration = EP932E_System_Configuration__HDCP_DIS;

	if(Audfreq == AUD_Mute)
	{
		EP932C_Registers.System_Configuration |= 
            /*0x22*/EP932E_System_Configuration__AUDIO_DIS;   // Audio mute enable    
	}
	else
	{
	    /*
		EP932C_Registers.System_Configuration = 
            ;*/  // Audio mute disable   
	}

	EP932C_Registers.Audio_Input_Format = Audfreq;	// set Audio frequency
	DBG_printf(("freq = "));
	switch(Audfreq)
	{
		case AUD_Mute:
			DBG_printf(("Audio Mute\r\n"));
			break;

		case AUD_SF_32000Hz:
			DBG_printf(("32K Hz\r\n"));
			break;
			
		case AUD_SF_44100Hz:
			DBG_printf(("44.1K Hz\r\n"));
			break;
			
		case AUD_SF_48000Hz:
			DBG_printf(("48K Hz\r\n"));
			break;
			
		case AUD_SF_88200Hz:
			DBG_printf(("88.2K Hz\r\n"));
			break;
			
		case AUD_SF_96000Hz:
			DBG_printf(("96K Hz\r\n"));
			break;
			
		case AUD_SF_176400Hz:
			DBG_printf(("176.4K Hz\r\n"));
			break;
			
		case AUD_SF_192000Hz:
			DBG_printf(("192K Hz\r\n"));
			break;

		default:
			DBG_printf(("Unknown %d\r\n",Audfreq));
			break;
			
	}
}



void  EP_HDMI_Set_Video_Timing(LCD_TIMING Timing)
{
		DBG_printf(("\r\n\r\n"));
		DBG_printf(("##############################################\r\n"));
	
		// no skew, Dual edge - falling edge first, 12 bit, FMT12 = 0, 
		EP932C_Registers.Video_Interface[0] = 0x04 /*| BSEL_24bit*/ /*| EDGE_rising */ /*| FMT_12*/;
		DBG_printf(("Video_Interface_0 = 0x%02X \r\n",(int)EP932C_Registers.Video_Interface[0] ));
		
		// mode: DE + Hsync + Vsync , input: YUV422
		EP932C_Registers.Video_Interface[1] = 0x0; 	// DE,HS,VS, YUV422
		DBG_printf(("Video_Interface_1 = 0x%02X \r\n",(int)EP932C_Registers.Video_Interface[1] ));
	
		switch (Timing)
		{
			case HDMI_1080P:/*1920x1080p 60Hz[16:9]*/	
				DBG_printf(("TVOUT_MODE_1080P60\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x10;
				break;
				
//			case HDMI_1080I: /*1920x1080I 60Hz[16:9]*/
//				DBG_printf(("TVOUT_MODE_1080I60\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x05;
//				break;
				
			case HDMI_720P: /*1280x720p 60Hz[16:9]*/
				DBG_printf(("TVOUT_MODE_720p\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x04;
				break;
				
			case HDMI_480P_16_9: /*720x480p 60Hz[16:9]*/
				DBG_printf(("TVOUT_MODE_480p[16:9]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x03;
				break;
				
			case HDMI_480P_4_3: /*720x480p 60Hz[4:3]*/
				DBG_printf(("TVOUT_MODE_480p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x02;
				break;
				
//			case HDMI_480I_16_9: /*720x480I 60Hz[16:9]*/
//				DBG_printf(("TVOUT_MODE_480I[16:9]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x07;
//				break;
				
//			case HDMI_480I_4_3: /*720x480I 60Hz[4:3]*/
//				DBG_printf(("TVOUT_MODE_480I[4:3]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x06;
//				break;
				
			case HDMI_576P_16_9: /*720x576p 50Hz[16:9]*/
				DBG_printf(("TVOUT_MODE_576p[16:9]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x12;
				break;
				
			case HDMI_576P_4_3: /*720x576p 50Hz[4:3]*/
				DBG_printf(("TVOUT_MODE_576p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x11;
				break;
				
//			case HDMI_576I_16_9: /*720x576I 50Hz[16:9]*/
//				DBG_printf(("TVOUT_MODE_576I[16:9]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x16;
//				break;
				
//			case HDMI_576I_4_3: /*720x576I 50Hz[4:3]*/
//				DBG_printf(("TVOUT_MODE_576I[4:3]\r\n"));
//				EP932C_Registers.Video_Input_Format[0] = 0x15;
//				break;

			case HDMI_640_480: //640x480p 60Hz[4:3]:
				DBG_printf(("TVOUT_MODE_640x480p[4:3]\r\n"));
				EP932C_Registers.Video_Input_Format[0] = 0x01;
				break;
			default:
				DBG_printf(("TVOUT_MODE_Unknown : %d\r\n",Timing));
				EP932C_Registers.Video_Input_Format[0] = 0x00;
				break;
		}
	
		// power on 
		EP932C_Registers.Power_Control = 0x00;
	
	//===================================================================
	
		DBG_printf(("##############################################\r\n"));
	
}

void EP_HDMI_Set_Video_Output(int HDMI_output)
{	
	switch(HDMI_output)	
	{		
		default:		
		case Video_OUT_Auto:
			EP932C_Registers.Video_Output_Format = 0x00;		// Auto select output			
			break;	
			
		case Video_OUT_YUV_444:			
			EP932C_Registers.Video_Output_Format = 0x01;		// YUV 444 output			
			break;		
			
		case Video_OUT_YUV_422:			
			EP932C_Registers.Video_Output_Format = 0x02;		// YUV 422 output			
			break;		
			
		case Video_OUT_RGB_444:			
			EP932C_Registers.Video_Output_Format = 0x03;		// RGB 444 output			
			break;	
	}
}

unsigned char EP_HDMI_Init(void)
{

	EP932Controller_Initial(&EP932C_Registers, NULL);

	return 0; //HDMI_SUCCESS;
}


