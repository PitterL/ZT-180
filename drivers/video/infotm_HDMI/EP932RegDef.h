/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP932RegDef.h

  Description :  Register Address definitions of EP932.

\******************************************************************************/

#ifndef EP932REGDEF_H
#define EP932REGDEF_H

// Registers								Word	BitMask
#define EP932_SMPRD							0x06			// 2 Byte

#define EP932_General_Control_1				0x08
#define EP932_General_Control_1__TSEL_HTP			0x80
#define EP932_General_Control_1__INT_OD				0x40
#define EP932_General_Control_1__INT_POL			0x20
#define EP932_General_Control_1__VTX				0x10
#define EP932_General_Control_1__DSEL				0x08
#define EP932_General_Control_1__BSEL				0x04
#define EP932_General_Control_1__EDGE				0x02
#define EP932_General_Control_1__PU					0x01

#define EP932_General_Control_2				0x09
#define EP932_General_Control_2__RSEN				0x80
#define EP932_General_Control_2__HTPLG				0x40
#define EP932_General_Control_2__RIE				0x20
#define EP932_General_Control_2__VIE				0x10
#define EP932_General_Control_2__MIE				0x08
#define EP932_General_Control_2__RIF				0x04
#define EP932_General_Control_2__VIF				0x02
#define EP932_General_Control_2__MIF				0x01

#define EP932_General_Control_3				0x0A	

#define EP932_Configuration					0x0B

#define EP932_Color_Space_Control			0x0C
#define EP932_Color_Space_Control__422_OUT			0x80
#define EP932_Color_Space_Control__YCC_OUT			0x40
#define EP932_Color_Space_Control__COLOR			0x20
#define EP932_Color_Space_Control__YCC_Range		0x10
#define EP932_Color_Space_Control__VMUTE			0x08
#define EP932_Color_Space_Control__AMUTE			0x04
#define EP932_Color_Space_Control__TREG				0x03
		
#define EP932_Pixel_Repetition_Control		0x0D	
#define EP932_Pixel_Repetition_Control__CS_M		0x80
#define EP932_Pixel_Repetition_Control__CTS_M		0x40
#define EP932_Pixel_Repetition_Control__ADSR		0x30
#define EP932_Pixel_Repetition_Control__OSCSEL		0x08
#define EP932_Pixel_Repetition_Control__VSYNC		0x04	
#define EP932_Pixel_Repetition_Control__PR			0x03

#define EP932_General_Control_4				0x0E
#define EP932_General_Control_4__FMT12				0x80
#define EP932_General_Control_4__422_IN				0x40
#define EP932_General_Control_4__YCC_IN				0x20
#define EP932_General_Control_4__E_SYNC				0x10
#define EP932_General_Control_4__VPOL_DET			0x08
#define EP932_General_Control_4__HPOL_DET			0x04
#define EP932_General_Control_4__EESS				0x02
#define EP932_General_Control_4__HDMI				0x01

#define EP932_General_Control_5				0x0F
#define EP932_General_Control_5__AKSV_RDY			0x80
#define EP932_General_Control_5__RPTR				0x10
#define EP932_General_Control_5__RI_RDY				0x02
#define EP932_General_Control_5__ENC_EN				0x01

#define EP932_BKSV							0x10			// BKSV1-BKSV5 0x10-0x14

#define EP932_AN							0x15			// AN1-AN8 0x15-0x1C

#define EP932_AKSV							0x1D			// AKSV1-AKSV5 0x1D-0x21

#define EP932_RI							0x22			// RI1-RI2 0x22-0x23

#define EP932_M0							0x25			// 0x25-0x32

#define EP932_DE_DLY						0x32			// 10 bit

#define EP932_DE_Control					0x33			// 10 bit
#define EP932_DE_Control__DE_GEN					0x40
#define EP932_DE_Control__VSO_POL					0x08
#define EP932_DE_Control__HSO_POL					0x04

#define EP932_DE_TOP						0x34			// 6 bit

#define EP932_DE_CNT						0x36			// 10 bit

#define EP932_DE_LIN						0x38			// 10 bit

#define EP932_H_RES							0x3A			// 11 bit

#define EP932_V_RES							0x3C			// 11 bit

#define EP932_Audio_Subpacket_Allocation	0x3E			// Default 0xE4

#define EP932_IIS_Control					0x3F			// Default 0x00
#define EP932_IIS_Control__ACR_EN					0x80
#define EP932_IIS_Control__AVI_EN					0x40
#define EP932_IIS_Control__ADO_EN					0x20
#define EP932_IIS_Control__AUDIO_EN					0x10
#define EP932_IIS_Control__WS_M						0x04
#define EP932_IIS_Control__WS_POL					0x02
#define EP932_IIS_Control__SCK_POL					0x01

#define EP932_Packet_Control				0x40			// Default 0x00
#define EP932_Packet_Control__FLAT3					0x80
#define EP932_Packet_Control__FLAT2					0x40
#define EP932_Packet_Control__FLAT1					0x20
#define EP932_Packet_Control__FLAT0					0x10
#define EP932_Packet_Control__LAYOUT				0x08
#define EP932_Packet_Control__IIS					0x04
#define EP932_Packet_Control__PKT_RDY				0x01

#define EP932_Data_Packet_Header 			0x41			// HB0-HB2 0x41-0x43

#define EP932_Data_Packet 					0x44			// PB0-PB27 0x44-0x5F

#define EP932_CTS		 					0x60			// 20bit (3 Byte)

#define EP932_N			 					0x63			// 20bit (3 Byte)

#define EP932_AVI_Packet 					0x66			// 14 Byte 0x66-0x73

#define EP932_ADO_Packet 					0x74			// 6 Byte 0x74-0x79

#define EP932_SPDIF_Sampling_Frequency 		0x7A			// 1 Byte

#define EP932_Channel_Status		 		0x7B			// 5 Byte 0x7B-0x7F

#define EP932_Embedded_Sync			 		0x80			// Default 0x00

#define EP932_H_Delay			 			0x81			// 10 bit (2 Byte)

#define EP932_H_Width			 			0x83			// 10 bit (2 Byte)

#define EP932_V_Delay			 			0x85			// 6 bit

#define EP932_V_Width			 			0x86			// 6 bit

#define EP932_V_Off_Set			 			0x87			// 12 bit (2 Byte)

#define EP932_Key_Add			 			0xF0			// 1 Byte

#define EP932_Key_Data			 			0xF1			// 7 Byte

#endif
