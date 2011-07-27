/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED 

--------------------------------------------------------------------------------

  File        :  DDC_If.h 

  Description :  Head file of DDC Interface  

\******************************************************************************/

#ifndef DDC_IF_H
#define DDC_IF_H

//#include "type.h"

//#include "EPcsapi.h"
#include "EP932api.h"

//==================================================================================================
//
// Protected Data Member
//
extern SMBUS_STATUS status;
extern unsigned char DDC_Data[128]; // The DDC Buffer

// EDID status error code
typedef enum {
	// Master
	EDID_STATUS_Success = 0x00,
	EDID_STATUS_Pending,//	SMBUS_STATUS_Abort,
	EDID_STATUS_NoAct = 0x02,
	EDID_STATUS_TimeOut,
	EDID_STATUS_ArbitrationLoss = 0x04,
	EDID_STATUS_ExtensionOverflow,
	EDID_STATUS_ChecksumError
} EDID_STATUS;


//==================================================================================================
//
// Public Functions
//

//--------------------------------------------------------------------------------------------------
//
// General
//

// All Interface Inital
//extern void DDC_If_Initial(CSI2C_HANDLE E_handle, CSI2C_HANDLE H_handle);


//--------------------------------------------------------------------------------------------------
//
// Downstream HDCP Control Interface
//

extern unsigned char Downstream_Rx_read_BKSV(unsigned char *pBKSV);
extern unsigned char Downstream_Rx_BCAPS(void);
extern void Downstream_Rx_write_AINFO(char ainfo);
extern void Downstream_Rx_write_AN(unsigned char *pAN);
extern void Downstream_Rx_write_AKSV(unsigned char *pAKSV);
extern unsigned char Downstream_Rx_read_RI(unsigned char *pRI);
extern void Downstream_Rx_read_BSTATUS(unsigned char *pBSTATUS);
extern void Downstream_Rx_read_SHA1_HASH(unsigned char *pSHA);
extern unsigned char Downstream_Rx_read_KSV_FIFO(unsigned char *pBKSV, unsigned char Index, unsigned char DevCount);


//--------------------------------------------------------------------------------------------------
//
// Downstream EDID Control Interface
//

extern unsigned char Downstream_Rx_poll_EDID(void);
extern EDID_STATUS Downstream_Rx_read_EDID(unsigned char *pEDID);


#endif // DDC_IF_H


