/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP932Controller.h

  Description :  Head file of EP932Controller.

\******************************************************************************/

#ifndef EP932CONTROLLER_H
#define EP932CONTROLLER_H

// Our includes
#include "EP932ERegDef.h"
#include "EP932_If.h"
#include "HDCP.h"

#define VERSION_MAJOR             0  // Beta
#define VERSION_MINOR             38 //      36

#define EP932C_TIMER_PERIOD       10		//     The EP932Controller.c must be re-compiled if user want to change this value.

typedef enum {
	EP932C_TASK_Idle = 0,
	EP932C_TASK_Error,
	EP932C_TASK_Pending
} EP932C_TASK_STATUS;

typedef struct _EP932C_REGISTER_MAP {

	// Read
	unsigned short		VendorID;			// 0x00
	unsigned short		DeviceID;
	unsigned char		Version_Major;
	unsigned char		Version_Minor;
	unsigned char		Configuration;

	unsigned char		Interrupt_Flags;		// 0x01

	unsigned char		System_Status;			// 0x02

	unsigned char		HDCP_Status;			// 0x03
	unsigned char		HDCP_State;
	unsigned char		HDCP_AKSV[5];
	unsigned char		HDCP_BKSV[5];
	unsigned char		HDCP_BCAPS3[3];
	unsigned char		HDCP_KSV_FIFO[5*16];
	unsigned char		HDCP_SHA[20];
	unsigned char		HDCP_M0[8];

	unsigned char		EDID_Status;			// 0x04
	unsigned char		EDID_VideoDataAddr;
	unsigned char		EDID_AudioDataAddr;
	unsigned char		EDID_SpeakerDataAddr;
	unsigned char		EDID_VendorDataAddr;
	unsigned char		EDID_ASFreq;
	unsigned char		EDID_AChannel;
						
	//unsigned short		VS_Period;				// 0x05 (Video Status)
	//unsigned short		H_Res;
	//unsigned short		V_Res;
	//unsigned short		Ratio_24;
	VDO_PARAMS 			Video_Params_Backup;

	//unsigned short		AS_Freq;				// 0x06 (Audio Status)
	//unsigned short		AS_Period;				// 
	ADO_PARAMS 			Audio_Params_Backup;

	unsigned char		Readed_EDID[256];		// 0x07

	// Read / Write
	unsigned char		Analog_Test_Control;	// 0X1C

	unsigned char		Power_Control;			// 0x20
	unsigned char		System_Configuration;

	unsigned char		Interrupt_Enable;		// 0x21

	unsigned char		Video_Interface[2];		// 0x22

	unsigned char		Audio_Interface;		// 0x23

	unsigned char		Video_Input_Format[2];	// 0x24

	unsigned char 		Video_Output_Format;

	unsigned char		Audio_Input_Format;		// 0x25

	unsigned char		End;

} EP932C_REGISTER_MAP, *PEP932C_REGISTER_MAP;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef void (*EP932C_CALLBACK)(void);

void EP932Controller_Initial(PEP932C_REGISTER_MAP pEP932C_RegMap, EP932C_CALLBACK IntCall);

unsigned char EP932Controller_Task(void);

void EP932Controller_Timer(void);
void  EP_HDMI_DumpMessage(void);
unsigned int EP932_HotPlugMonitor(void);
unsigned int EP932_HotPlugMonitorInt(void);

// -----------------------------------------------------------------------------
#endif

