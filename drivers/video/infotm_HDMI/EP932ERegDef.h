/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP932ERegDef.h

  Description :  Register Address definitions of EP932E.

\******************************************************************************/

#ifndef EP932EREGDEF_H
#define EP932EREGDEF_H

// Registers									Address		BitMask		Value
#define EP932E_Vendor_ID						0x0000
#define EP932E_Device_ID						0x0002
#define EP932E_Firmware_Revision__Major			0x0004
#define EP932E_Firmware_Revision__Minor			0x0005

#define EP932E_Interrupt_Flags					0x0100
#define EP932E_Interrupt_Flags__EDID_CHG					0x80
#define EP932E_Interrupt_Flags__VIDEO_CHG					0x40
#define EP932E_Interrupt_Flags__AUDIO_CHG					0x20
#define EP932E_Interrupt_Flags__VS_ALIGN_FAILED				0x10
#define EP932E_Interrupt_Flags__VS_ALIGN_DONE				0x08

#define EP932E_System_Status					0x0200
#define EP932E_System_Status__RSEN							0x80
#define EP932E_System_Status__HTPLG							0x40
#define EP932E_System_Status__KEY_FAIL						0x02
#define EP932E_System_Status__DEF_KEY						0x01

#define EP932E_HDCP_Status						0x0300
#define EP932E_HDCP_Status__BKSV							0x80
#define EP932E_HDCP_Status__AKSV							0x40
#define EP932E_HDCP_Status__R0								0x20
#define EP932E_HDCP_Status__Ri								0x10
#define EP932E_HDCP_Status__RepeaterRdy						0x08
#define EP932E_HDCP_Status__RepeaterSHA						0x04
#define EP932E_HDCP_Status__RSEN							0x02
#define EP932E_HDCP_Status__REVOKE							0x01

#define EP932E_HDCP_State						0x0301

#define EP932E_HDCP_AKSV						0x0302
 
#define EP932E_HDCP_BKSV						0x0307 

#define EP932E_HDCP_BCAPS						0x030C

#define EP932E_HDCP_BSTATUS						0x030D

#define EP932E_HDCP_KSV_FIFO					0x030F

#define EP932E_EDID_Status						0x0400
#define EP932E_EDID_Status__HDMI							0x10
#define EP932E_EDID_Status__DDC_Status						0x0F
typedef enum {
	EDID_DDC_Success = 0x00,
	EDID_DDC_Pending,
	EDID_DDC_NoAct = 0x02,
	EDID_DDC_TimeOut,
	EDID_DDC_ArbitrationLoss = 0x04,
	EDID_DDC_BlockNumber
} EDID_DDC_STATUS;
#define EP932E_EDID_Status_ASFreq				0x0401
#define EP932E_EDID_Status_AChannel				0x0402

#define EP932E_Video_Status_VS_Period			0x0500					// 2 Byte
#define EP932E_Video_Status_H_RES				0x0502					// 2 Byte
#define EP932E_Video_Status_V_RES				0x0504					// 2 Byte
#define EP932E_Video_Status_RATIO_24			0x0506					// 2 Byte
#define EP932E_Video_Status_Params				0x0508					// 8 Byte

#define EP932E_Audio_Status_AS_Freq				0x0600					// 2 Byte
#define EP932E_Audio_Status_AS_Period			0x0602					// 2 Byte
#define EP932E_Audio_Status_Params				0x0604					// 7 Byte

#define EP932E_EDID_Data						0x0700					// 256 Byte

#define EP932E_Analog_Test_Control				0x1C00
#define EP932E_Analog_Test_Control__PREEMPHASIS				0x03

#define EP932E_SIP_Test_Control					0x1D00
#define EP932E_SIP_Test_Control__VS_ALIGN					0x08
#define EP932E_SIP_Test_Control__BIST						0x04
#define EP932E_SIP_Test_Control__ANA_TEST					0x02
#define EP932E_SIP_Test_Control__IIC_STOP 					0x01

#define EP932E_Power_Control					0x2000
#define EP932E_Power_Control__PD_HDMI						0x02
#define EP932E_Power_Control__PD_TOT						0x01

#define EP932E_System_Configuration				0x2001
#define EP932E_System_Configuration__PACKET_RDY				0x80
#define EP932E_System_Configuration__HDCP_DIS				0x20
#define EP932E_System_Configuration__HDMI_DIS				0x10
#define EP932E_System_Configuration__FORCE_HDMI_CAP			0x08
#define EP932E_System_Configuration__AUDIO_DIS				0x02
#define EP932E_System_Configuration__VIDEO_DIS				0x01

#define EP932E_Interrupt_Enable					0x2100
#define EP932E_Interrupt_Enable__EDID_CHG					0x80
#define EP932E_Interrupt_Enable__VIDEO_CHG					0x40
#define EP932E_Interrupt_Enable__AUDIO_CHG					0x20
#define EP932E_Interrupt_Enable__VS_ALIGN_DONE				0x08

#define EP932E_Video_Interface_Setting_0		0x2200
#define EP932E_Video_Interface_Setting_0__DK				0xE0
#define EP932E_Video_Interface_Setting_0__DKEN				0x10
#define EP932E_Video_Interface_Setting_0__DSEL				0x08
#define EP932E_Video_Interface_Setting_0__BSEL				0x04
#define EP932E_Video_Interface_Setting_0__EDGE				0x02
#define EP932E_Video_Interface_Setting_0__FMT12				0x01

#define EP932E_Video_Interface_Setting_1		0x2201
#define EP932E_Video_Interface_Setting_1__COLOR				0x30
#define EP932E_Video_Interface_Setting_1__COLOR__Auto					0x00
#define EP932E_Video_Interface_Setting_1__COLOR__601					0x10
#define EP932E_Video_Interface_Setting_1__COLOR__709					0x20
#define EP932E_Video_Interface_Setting_1__xvYCC_EN			0x80
#define EP932E_Video_Interface_Setting_1__SYNC				0x0C
#define EP932E_Video_Interface_Setting_1__SYNC__HVDE					0x00
#define EP932E_Video_Interface_Setting_1__SYNC__HV						0x04
#define EP932E_Video_Interface_Setting_1__SYNC__Embeded					0x08
#define EP932E_Video_Interface_Setting_1__VIN_FMT			0x03
#define EP932E_Video_Interface_Setting_1__VIN_FMT__RGB					0x00
#define EP932E_Video_Interface_Setting_1__VIN_FMT__YCC444				0x01
#define EP932E_Video_Interface_Setting_1__VIN_FMT__YCC422				0x02

#define EP932E_Audio_Interface_Setting			0x2300
#define EP932E_Audio_Interface_Setting__CHANNEL				0xF0
#define EP932E_Audio_Interface_Setting__IIS					0x08
#define EP932E_Audio_Interface_Setting__WS_M				0x04
#define EP932E_Audio_Interface_Setting__WS_POL				0x02
#define EP932E_Audio_Interface_Setting__SCK_POL				0x01

#define EP932E_Video_Input_Format_VIC			0x2400
#define EP932E_Video_Input_Format_1				0x2401
#define EP932E_Video_Input_Format_1__AFAR					0x30
#define EP932E_Video_Input_Format_1__AFAR__Auto							0x00
#define EP932E_Video_Input_Format_1__AFAR__4_3							0x10
#define EP932E_Video_Input_Format_1__AFAR__16_9							0x20
#define EP932E_Video_Input_Format_1__AFAR__14_9							0x30
#define EP932E_Video_Input_Format_1__VIF								0x08	 //0x01  //modify by eric.lu 
#define EP932E_Video_Input_Format_1__SCAN								0x03	 		//add by eric.lu 

#define EP932E_Audio_Input_Format				0x2500
#define EP932E_Audio_Input_Format__NoCopyRight				0x10
#define EP932E_Audio_Input_Format__ADO_FREQ					0x07
#define EP932E_Audio_Input_Format__ADO_FREQ__Auto						0x00
#define EP932E_Audio_Input_Format__ADO_FREQ__32000Hz					0x01
#define EP932E_Audio_Input_Format__ADO_FREQ__44100Hz					0x02
#define EP932E_Audio_Input_Format__ADO_FREQ__48000Hz					0x03
#define EP932E_Audio_Input_Format__ADO_FREQ__88200Hz					0x04
#define EP932E_Audio_Input_Format__ADO_FREQ__96000Hz					0x05
#define EP932E_Audio_Input_Format__ADO_FREQ__176400Hz					0x06
#define EP932E_Audio_Input_Format__ADO_FREQ__192000Hz					0x07

#define EP932E_KSV_Revocation_List				0x2600

#define EP932E_Gamut_Packet_Header				0x2700
#define EP932E_Gamut_Packet_Data				0x2703

#define EP932E_Select_Packet_Header				0x2800
#define EP932E_Select_Packet_Data				0x2803

#endif
