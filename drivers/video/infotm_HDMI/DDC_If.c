/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED

--------------------------------------------------------------------------------

  File        :  DDC_If.c

  Description :  EP932E DDC Interface

\******************************************************************************/

#include "DDC_If.h"

//--------------------------------------------------------------------------------------------------

#define HDCP_RX_ADDR          0x74     // HDCP RX Address
#define EDID_ADDR       		0x50//0xA0     // EDID Address
#define EDID_SEGMENT_PTR		0x60

#define HDCP_RX_BKSV_ADDR       0x00     // HDCP RX, BKSV Register Address
#define HDCP_RX_RI_ADDR         0x08     // HDCP RX, RI Register Address
#define HDCP_RX_AKSV_ADDR       0x10     // HDCP RX, AKSV Register Address
#define HDCP_RX_AINFO_ADDR      0x15     // HDCP RX, AINFO Register Address
#define HDCP_RX_AN_ADDR         0x18     // HDCP RX, AN Register Address
#define HDCP_RX_SHA1_HASH_ADDR  0x20     // HDCP RX, SHA-1 Hash Value Start Address
#define HDCP_RX_BCAPS_ADDR      0x40     // HDCP RX, BCAPS Register Address
#define HDCP_RX_BSTATUS_ADDR    0x41     // HDCP RX, BSTATUS Register Address
#define HDCP_RX_KSV_FIFO_ADDR   0x43     // HDCP RX, KSV FIFO Start Address

//--------------------------------------------------------------------------------------------------

// Private Data

//int i, j;
SMBUS_STATUS status;

unsigned char DDC_Data[128];
unsigned char TempBit;

// Private Functions
SMBUS_STATUS DDC_Write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
SMBUS_STATUS DDC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
//SMBUS_STATUS DDC_NoStop(unsigned char IICAddr, unsigned char ByteAddr, void *Data, unsigned int Size);
//==================================================================================================
//
// Public Function Implementation
//

//--------------------------------------------------------------------------------------------------
// Hardware Interface


//--------------------------------------------------------------------------------------------------
//
// Downstream HDCP Control
//

unsigned char Downstream_Rx_read_BKSV(unsigned char *pBKSV)
{
	int i, j;
	status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_BKSV_ADDR, pBKSV, 5);
	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: BKSV read - DN DDC %d\r\n", (int)status));
		return 0;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		DBG_printf(("ERROR: BKSV read - Key Wrong\r\n"));
		DBG_printf(("ERROR: BKSV=0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n", (unsigned int)pBKSV[0], (unsigned int)pBKSV[1], (unsigned int)pBKSV[2], (unsigned int)pBKSV[3], (unsigned int)pBKSV[4]));
		return 0;
	}
	return 1;
}

unsigned char Downstream_Rx_BCAPS(void)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_BCAPS_ADDR, DDC_Data, 1);
	return DDC_Data[0];
}

void Downstream_Rx_write_AINFO(char ainfo)
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AINFO_ADDR, &ainfo, 1);
}

void Downstream_Rx_write_AN(unsigned char *pAN)
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AN_ADDR, pAN, 8);
}

void Downstream_Rx_write_AKSV(unsigned char *pAKSV)
{
	DDC_Write(HDCP_RX_ADDR, HDCP_RX_AKSV_ADDR, pAKSV, 5);
}

unsigned char Downstream_Rx_read_RI(unsigned char *pRI)
{
	// Short Read
	status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_RI_ADDR, pRI, 2);
	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: Rx Ri read - MCU IIC %d\r\n", (int)status));
		return 0;
	}
	return 1;
}

void Downstream_Rx_read_BSTATUS(unsigned char *pBSTATUS)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_BSTATUS_ADDR, pBSTATUS, 2);
}

void Downstream_Rx_read_SHA1_HASH(unsigned char *pSHA)
{
	DDC_Read(HDCP_RX_ADDR, HDCP_RX_SHA1_HASH_ADDR, pSHA, 20);
}

// Retrive a 5 byte KSV at "Index" from FIFO
unsigned char Downstream_Rx_read_KSV_FIFO(unsigned char *pBKSV, unsigned char Index, unsigned char DevCount)
{
	int i, j;

	// Try not to re-read the previous KSV
	if(Index == 0) { // Start
		// Support a max 25 device count because of DDC_Data[] size is 128 byte
		status = DDC_Read(HDCP_RX_ADDR, HDCP_RX_KSV_FIFO_ADDR, DDC_Data, min(DevCount, 25));
	}
	memcpy(pBKSV, DDC_Data+(Index*5), 5);

	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: KSV FIFO read - DN DDC %d\r\n", (int)status));
		return 0;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		TempBit = 1;
		while (TempBit) {
			if (pBKSV[i] & TempBit) j++;
			TempBit <<= 1;
		}
		i++;
	}
	if(j != 20) {
		DBG_printf(("ERROR: KSV FIFO read - Key Wrong\r\n"));
		return 0;
	}
	return 1;
}


//--------------------------------------------------------------------------------------------------
//
// Downstream EDID Control
//

unsigned char Downstream_Rx_poll_EDID(void)
{
	// Read the EDID test

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// How to implement this with CSI2C ?????????????????????????????
	// Without the Segment address implementation, it works normally.
	// But, this must be implemented for ATC test.
	//DDC_Data[0] = 0;	// Segment Pointer Address
	//SMBUS_master_rw_synchronous(DDC_Bus, EDID_SEGMENT_PTR, DDC_Data, 1, SMBUS_SkipStop);
	/////////////////////////////////////////////////////////////////////////////////////////////////

	// Base Address and Read 1
	status = DDC_Read(EDID_ADDR, 0, DDC_Data, 1);

	if(status != SMBUS_STATUS_Success) // can't read EDID
	{
		return 2;
	}
	if(DDC_Data[0] != 0x00)				// EDID header fail
	{
		return 2;
	}
	return 0;							// Read EDID success

}

EDID_STATUS Downstream_Rx_read_EDID(unsigned char *pEDID)
{
	int i;
	unsigned char seg_ptr, BlockCount, Block1Found, ChkSum;

	// =========================================================
	// I. Read the block 0

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// How to implement this with CSI2C ?????????????????????????????
	// Without the Segment address implementation, it works normally.
	// But, this must be implemented for ATC test.
	//DDC_Data[0] = 0;	// Segment Pointer Address
	//SMBUS_master_rw_synchronous(DDC_Bus, EDID_SEGMENT_PTR, DDC_Data, 1, SMBUS_SkipStop);
	/////////////////////////////////////////////////////////////////////////////////////////////////

	// Base Address and Read 128
	//sys_ram[0] = 0;
	//DDC_NoStop(EDID_SEGMENT_PTR, 0, sys_ram, 1);	// skip stop 

	status = DDC_Read(EDID_ADDR, 0, pEDID, 128);
	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: EDID b0 read - DN DDC %d\r\n", (int)status));
		return status;
	}
	DBG_printf(("EDID b0 read:"));
	for(i=0; i<128; ++i) {
		if(i%16 == 0) DBG_printf(("\r\n"));
		if(i%8 == 0) DBG_printf((" "));
		DBG_printf(("0x%02X, ", (int)pEDID[i] ));
	}
	DBG_printf(("\r\n"));

	if( (pEDID[0] != 0x00) ||
	    (pEDID[1] != 0xFF) ||
	    (pEDID[2] != 0xFF) ||
	    (pEDID[3] != 0xFF) ||
	    (pEDID[4] != 0xFF) ||
	    (pEDID[5] != 0xFF) ||
	    (pEDID[5] != 0xFF) ||
	    (pEDID[7] != 0x00))
	{
	    DBG_printf(("ERROR: EDID[0~8] Check failed\n\r", (int)pEDID[126] ));
		//return EDID_STATUS_NoAct;
	}

	// Check EDID
	if(pEDID[126] > 8) {
		DBG_printf(("ERROR: EDID[126] Check failed, pEDID[126]=0x%02X > 8\n\r", (int)pEDID[126] ));
		return EDID_STATUS_ExtensionOverflow;
	}

	// =========================================================
	// II. Read other blocks and find Timing Extension Block

	BlockCount = pEDID[126];
	Block1Found = 0;
	for (seg_ptr = 1; seg_ptr <= BlockCount; ++seg_ptr) {

		/////////////////////////////////////////////////////////////////////////////////////////////////
		// How to implement this with Customer's I2C ?????????????????????????????
		// Without the Segment address implementation, it works normally.
		// But, this must be implemented for ATC test.
		//DDC_Data[0] = seg_ptr >> 1;	// Segment Pointer Address
		//SMBUS_master_rw_synchronous(DDC_Bus, EDID_SEGMENT_PTR, DDC_Data, 1, SMBUS_SkipStop);
		/////////////////////////////////////////////////////////////////////////////////////////////////

		// Base Address and Read 128
		//sys_ram[0] = seg_ptr >> 1;	// Segment Pointer Address
		//DDC_NoStop(EDID_SEGMENT_PTR, 0, sys_ram, 1);	// skip stop 

		status = DDC_Read(EDID_ADDR, (seg_ptr & 0x01) << 7, DDC_Data, 128);
		if(status != SMBUS_STATUS_Success) {
			DBG_printf(("ERROR: EDID bi read - DN DDC %d\r\n", (int)status));
			return status;
		}

		if(DDC_Data[0] == 0x02 && Block1Found == 0) {
			Block1Found = 1;
			memcpy(&pEDID[128], DDC_Data, 128);
		}

		DBG_printf(("EDID b%d read:", (int)seg_ptr));
		for(i=0; i<128; ++i) {
			if(i%16 == 0) DBG_printf(("\r\n"));
			if(i%8 == 0) DBG_printf((" "));
			DBG_printf(("0x%02X, ", (int)DDC_Data[i] ));
		}
		DBG_printf(("\r\n"));
	}

	// Check CheckSum
	ChkSum = 0;
	for(i=0; i<((Block1Found)?256:128); ++i) {
		ChkSum += pEDID[i];
	}
	if(ChkSum != 0) {
		return EDID_STATUS_ChecksumError;
	}
	if(Block1Found) {
		pEDID[126] = 1;
	}
	else {
		pEDID[126] = 0;
	}
	return EDID_STATUS_Success;
}

//==================================================================================================
//
// Private Functions
//

SMBUS_STATUS DDC_Write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// How to implement this with Customer's I2C ?????????????????????????????
	// return 0; for success
	// return 2; for No_ACK
	// return 4; for Arbitration
	/////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
	int result = 1;
	int i;
 	unsigned long to, tn;
	to = get_ticks();

	// need to use customer's I2C  function
	//result = I2C_WriteReg_EP932M(IICAddr, ByteAddr, Data, Size);

	if(iic_init(0,1,IICAddr,0,1,0)== 0)
		return 1;

	for(i = 0;i < Size;i++)
	{
		while(iic_write(0,ByteAddr+i,Data[i]) == 0)
		{
			 tn = get_ticks();
			 if(tn > to + 500000) break;
		}
	}
	return 0;
#endif

	struct i2c_adapter *adapter;
	unsigned char *buf = kmalloc(Size + 1, GFP_KERNEL);
	unsigned int i;

	buf[0] = ByteAddr;
	for(i=0;i<Size;i++)
		buf[i + 1] = Data[i];

	struct i2c_msg msgs[] = { 
		{
			.addr   = IICAddr,
			.flags  = 0,
			.len            = Size + 1,
			.buf            = buf,
		}
	};

	if (!buf)
	{
		printk(KERN_ERR "[DDC_Write]: unable to allocate memory for EDID.\n");
		return -1; 
	}


	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		printk(KERN_ERR "[DDC_Write]: can't get i2c adapter\n");

		return -1; 
	}

	if (i2c_transfer(adapter, msgs, 1) != 1)
		return -1; 

	kfree(buf);

	return 0;

}

SMBUS_STATUS DDC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// How to implement this with CSI2C ?????????????????????????????
	// return 0; for success
	// return 2; for No_ACK
	// return 4; for Arbitration
	/////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
	int result = 1;
	int i;
 	unsigned long to, tn;
	to = get_ticks();

	// need to use customer's I2C  function
	//result = I2C_ReadReg_EP932M(IICAddr, ByteAddr, Data, Size);
	if(iic_init(0,1,IICAddr,0,1,0)== 0)
		return 1;

	for(i=0;i<Size;i++)
	{
		while(iic_read(0,ByteAddr+i,&Data[i]) == 0)
		{
			 tn = get_ticks();
			 if(tn > to + 500000) break;
		}
	}

	return 0;
#endif

	struct i2c_adapter *adapter;
	
	unsigned char *buf = kmalloc(Size, GFP_KERNEL);
	struct i2c_msg msgs1[] = { 
		{
			.addr   = IICAddr,
			.flags  = 0,
			.len            = 1,
			.buf            = &ByteAddr,
		}
	};

	struct i2c_msg msgs2[] = { 
		{
			.addr   = IICAddr,
			.flags  = I2C_M_RD,
			.len            = Size,
			.buf            = Data,
		}
	};

	if (!buf)
	{
		printk(KERN_ERR "[DDC_Read]: unable to allocate memory for EDID.\n");
		return -1; 
	}

	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		printk(KERN_ERR "[DDC_Read]: can't get i2c adapter\n");

		return -1; 
	}

	if (i2c_transfer(adapter, msgs1, 1) != 1)
		return -1; 

	if (i2c_transfer(adapter, msgs2, 1) != 1)
		return -1; 

	kfree(buf);

	return 0;

}

/*
SMBUS_STATUS DDC_NoStop(unsigned char IICAddr, unsigned char ByteAddr, void *Data, unsigned int Size)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// How to implement this with CSI2C ?????????????????????????????
	// return 0; for success
	// return 2; for No_ACK
	// return 4; for Arbitration
	/////////////////////////////////////////////////////////////////////////////////////////////////
	int result = 1;
	
	// need to use customer's I2C  function
	//result = I2C_ReadReg_EP932M(IICAddr, ByteAddr, Data, Size);
	
	return result;
}
*/
