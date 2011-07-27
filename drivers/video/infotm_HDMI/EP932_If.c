/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005
                           ALL RIGHTS RESERVED

--------------------------------------------------------------------------------

  File        :  EP932_If.c

  Description :  EP932 IIC Interface

\******************************************************************************/
#include "EP932_If.h"
#include "DDC_If.h"

#include "EP932SettingsData.h"

#include <mach/debug.h>


//--------------------------------------------------------------------------------------------------

//#define EDID_ADDR       		0xA0     // EDID Address
//#define EDID_SEGMENT_PTR		0x60
#define EP932_ADDR				0x70
#define EP932_ADDR_2			0x72
#define HEY_ADDR				0xA8


//#define Little_Endian

//--------------------------------------------------------------------------------------------------

// Private Data
unsigned char IIC_EP932_Addr,IIC_Key_Addr;

//int i, j;
//SMBUS_STATUS status;
//USHORT TempUSHORT;
unsigned short TempUSHORT;

unsigned char Temp_Data[15];
unsigned char W_Data[2];

// Global date for HDMI Transmiter
unsigned char is_HDCP_AVMute;
unsigned char is_AMute;
unsigned char is_VMute;
unsigned char is_HDMI;
unsigned char is_RSEN;
unsigned char Cache_EP932_DE_Control;

// Private Functions
SMBUS_STATUS IIC_Write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);
SMBUS_STATUS IIC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size);


//==================================================================================================
//
// Public Function Implementation
//

//--------------------------------------------------------------------------------------------------
// Hardware Interface

void EP932_If_Initial(void)	//customer setting
{
//	IIC_EP932_Addr = 0x70;
	IIC_EP932_Addr = 0x38;
	IIC_Key_Addr = 0xA8;	
}

void EP932_If_Reset(void)
{
	int i;

	// Global date for HDMI Transmiter
	is_HDCP_AVMute = 0;
	is_AMute = 1;
	is_VMute = 1;
	is_HDMI = 0;
	is_RSEN = 0;
	Cache_EP932_DE_Control = 0x03;

	// Initial Settings
	EP932_Reg_Set_Bit(EP932_General_Control_1, EP932_General_Control_1__VTX);
	EP932_Reg_Set_Bit(EP932_General_Control_1, EP932_General_Control_1__INT_OD);

	// Default Audio Mute
	EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__AMUTE);
	EP932_Reg_Set_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__CTS_M);
	// Default Video Mute
	EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__VMUTE);

	//
	// Set Default AVI Info Frame
	//
	memset(Temp_Data, 0x00, 14);

	// Set AVI Info Frame to RGB
	Temp_Data[1] &= 0x60;
	Temp_Data[1] |= 0x00; // RGB

	// Set AVI Info Frame to 601
	Temp_Data[2] &= 0xC0;
	Temp_Data[2] |= 0x40;

	// Write AVI Info Frame
	Temp_Data[0] = 0;
	for(i=1; i<14; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP932_Reg_Write(EP932_AVI_Packet, Temp_Data, 14);


	//
	// Set Default ADO Info Frame
	//
	memset(Temp_Data, 0x00, 6);

	// Write ADO Info Frame
	Temp_Data[0] = 0;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP932_Reg_Write(EP932_ADO_Packet, Temp_Data, 6);
}

//--------------------------------------------------------------------------------------------------
//
// HDMI Transmiter (EP932-Tx Implementation)
//

void HDMI_Tx_Power_Down(void)
{
	// Software power down
	EP932_Reg_Clear_Bit(EP932_General_Control_1, EP932_General_Control_1__PU);
}

void HDMI_Tx_Power_Up(void)
{
	// Software power up
	EP932_Reg_Set_Bit(EP932_General_Control_1, EP932_General_Control_1__PU);
}

unsigned char HDMI_Tx_HTPLG(void)
{
	// Software HotPlug Detect
	EP932_Reg_Read(EP932_General_Control_2, Temp_Data, 1);
	is_RSEN = (Temp_Data[0] & EP932_General_Control_2__RSEN)? 1:0;
	if(Temp_Data[0] & EP932_General_Control_2__HTPLG)
	{
		DBG_printf(("HDMI_Tx_HTPLG connect\r\n"));
		return 1;
	}
	else
	{
		DBG_printf(("HDMI_Tx_HTPLG disconnect\r\n"));
		return 0;
	}
	// This is for old DVI monitor compatibility. For HDMI TV, there is no need to poll the EDID.
	//return Downstream_Rx_poll_EDID();
}

unsigned char HDMI_Tx_RSEN(void)
{
	return is_RSEN;
}

void HDMI_Tx_HDMI(void)
{
	if(!is_HDMI) {
		is_HDMI = 1;
		EP932_Reg_Set_Bit(EP932_General_Control_4, EP932_General_Control_4__HDMI);
		DBG_printf(("Set to HDMI mode\r\n"));
	}
}

void HDMI_Tx_DVI(void)
{
	if(is_HDMI) {
		is_HDMI = 0;
		EP932_Reg_Clear_Bit(EP932_General_Control_4, EP932_General_Control_4__HDMI);
		DBG_printf(("Set to DVI mode\r\n"));
	}
}

//------------------------------------
// HDCP

void HDMI_Tx_Mute_Enable(void)
{
	is_HDCP_AVMute = 1;
	EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__AMUTE | EP932_Color_Space_Control__VMUTE);
	EP932_Reg_Set_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__CTS_M);
}

void HDMI_Tx_Mute_Disable(void)
{
	is_HDCP_AVMute = 0;

	if(!is_AMute) {
		EP932_Reg_Clear_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__CTS_M);
		EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__AMUTE);
	}
	if(!is_VMute) {
		EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__VMUTE);
	}
}

void HDMI_Tx_HDCP_Enable(void)
{
	EP932_Reg_Set_Bit(EP932_General_Control_5, EP932_General_Control_5__ENC_EN);
}

void HDMI_Tx_HDCP_Disable(void)
{
	EP932_Reg_Clear_Bit(EP932_General_Control_5, EP932_General_Control_5__ENC_EN);
}

void HDMI_Tx_RPTR_Set(void)
{
	EP932_Reg_Set_Bit(EP932_General_Control_5, EP932_General_Control_5__RPTR);
}

void HDMI_Tx_RPTR_Clear(void)
{
	EP932_Reg_Clear_Bit(EP932_General_Control_5, EP932_General_Control_5__RPTR);
}

void HDMI_Tx_write_AN(unsigned char *pAN)
{
	EP932_Reg_Write(EP932_AN, pAN, 8);
}

unsigned char HDMI_Tx_AKSV_RDY(void)
{
	status = EP932_Reg_Read(EP932_General_Control_5, Temp_Data, 1);
	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: AKSV RDY - MCU IIC %d\r\n", (int)status));
		return 0;
	}
	return (Temp_Data[0] & EP932_General_Control_5__AKSV_RDY)? 1:0;
}

unsigned char HDMI_Tx_read_AKSV(unsigned char *pAKSV)
{
	int i, j;

	status = EP932_Reg_Read(EP932_AKSV, pAKSV, 5);
	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: AKSV read - MCU IIC %d\r\n", (int)status));
		return 0;
	}

	i = 0;
	j = 0;
	while (i < 5) {
		Temp_Data[0] = 1;
		while (Temp_Data[0]) {
			if (pAKSV[i] & Temp_Data[0]) j++;
			Temp_Data[0] <<= 1;
		}
		i++;
	}
	if(j != 20) {
		DBG_printf(("ERROR: AKSV read - Key Wrong\r\n"));
		return 0;
	}
	return 1;
}

void HDMI_Tx_write_BKSV(unsigned char *pBKSV)
{
	EP932_Reg_Write(EP932_BKSV, pBKSV, 5);
}

unsigned char HDMI_Tx_RI_RDY(void)
{
	EP932_Reg_Read(EP932_General_Control_5, Temp_Data, 1);
	return (Temp_Data[0] & EP932_General_Control_5__RI_RDY)? 1:0;
}

unsigned char HDMI_Tx_read_RI(unsigned char *pRI)
{
	status = EP932_Reg_Read(EP932_RI, pRI, 2);
	if(status != SMBUS_STATUS_Success) {
		DBG_printf(("ERROR: Tx Ri read - MCU IIC %d\r\n", (int)status));
		return 0;
	}
	return 1;
}

void HDMI_Tx_read_M0(unsigned char *pM0)
{
	status = EP932_Reg_Read(EP932_M0, pM0, 8);
}

SMBUS_STATUS HDMI_Tx_Get_Key(unsigned char *Key)
{
	return IIC_Read(HEY_ADDR, 0, Key, 512);
}

//------------------------------------
// Special for config

void HDMI_Tx_AMute_Enable(void)
{
	unsigned char Temp_byte[2];
	if(!is_AMute) {
		is_AMute = 1;
		EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__AMUTE);
		EP932_Reg_Set_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__CTS_M);

		EP932_Reg_Clear_Bit(EP932_IIS_Control, EP932_IIS_Control__ADO_EN);	// add by eric.lu
		EP932_Reg_Clear_Bit(EP932_IIS_Control, EP932_IIS_Control__AUDIO_EN);	// add by eric.lu

		DBG_printf(("<<< AMute_enable >>>\r\n"));

		//read for verify
		EP932_Reg_Read(EP932_Color_Space_Control, Temp_byte, 1);
		DBG_printf(("EP932_Color_Space_Control = 0x%02X\r\n",(int)Temp_byte[0]));
		EP932_Reg_Read(EP932_Pixel_Repetition_Control, Temp_byte, 1);
		DBG_printf(("EP932_Pixel_Repetition_Control = 0x%02X\r\n",(int)Temp_byte[0]));
		// add end
	}
}

void HDMI_Tx_AMute_Disable(void)
{
	unsigned char Temp_byte[2];
	if(is_AMute) {
		is_AMute = 0;
		if(!is_HDCP_AVMute) {
			EP932_Reg_Clear_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__CTS_M);
			EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__AMUTE);

			EP932_Reg_Set_Bit(EP932_IIS_Control, EP932_IIS_Control__ADO_EN);		// add by eric.lu
			EP932_Reg_Set_Bit(EP932_IIS_Control, EP932_IIS_Control__AUDIO_EN);	// add by eric.lu

			DBG_printf(("<<< AMute_disable >>>\r\n"));

			//read for verify
			EP932_Reg_Read(EP932_Color_Space_Control, Temp_byte, 1);
			DBG_printf(("EP932_Color_Space_Control = 0x%02X\r\n",(int)Temp_byte[0]));
			EP932_Reg_Read(EP932_Pixel_Repetition_Control, Temp_byte, 1);
			DBG_printf(("EP932_Pixel_Repetition_Control = 0x%02X\r\n",(int)Temp_byte[0]));
			// add end
		}
	}
}

void HDMI_Tx_VMute_Enable(void)
{
	if(!is_VMute) {
		is_VMute = 1;
		EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__VMUTE);

		DBG_printf(("<<< VMute_enable >>>\r\n"));
	}
}

void HDMI_Tx_VMute_Disable(void)
{
	if(is_VMute) {
		is_VMute = 0;
		if(!is_HDCP_AVMute) {
			EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__VMUTE);

			DBG_printf(("<<< VMute_disable >>>\r\n"));
		}
	}
}

void HDMI_Tx_Video_Config(PVDO_PARAMS Params)
{
	int i;
	DBG_printf(("\r\nStart Tx Video Config\r\n"));

	//
	// Disable Video
	//
	EP932_Reg_Clear_Bit(EP932_IIS_Control, EP932_IIS_Control__AVI_EN);

	//
	// Video Settings
	//
	// Interface
	EP932_Reg_Read(EP932_General_Control_3, Temp_Data, 1);
	Temp_Data[0] &= ~0xF0;
	Temp_Data[0] |= Params->Interface & 0xF0;
	EP932_Reg_Write(EP932_General_Control_3, Temp_Data, 1);

	EP932_Reg_Read(EP932_General_Control_1, Temp_Data, 1);
	Temp_Data[0] &= ~0x0E;
	Temp_Data[0] |= Params->Interface & 0x0E;
	EP932_Reg_Write(EP932_General_Control_1, Temp_Data, 1);

	if(Params->Interface & 0x01) {
		EP932_Reg_Set_Bit(EP932_General_Control_4, EP932_General_Control_4__FMT12);
	}
	else {
		EP932_Reg_Clear_Bit(EP932_General_Control_4, EP932_General_Control_4__FMT12);
	}

	// Sync Mode
	switch(Params->SyncMode) {
		default:
	 	case SYNCMODE_HVDE:
			// Disable E_SYNC
			EP932_Reg_Clear_Bit(EP932_General_Control_4, EP932_General_Control_4__E_SYNC);
			// Disable DE_GEN
			Cache_EP932_DE_Control &= ~EP932_DE_Control__DE_GEN;
			//EP932_Reg_Write(EP932_DE_Control, &Cache_EP932_DE_Control, 1);

			// Regular VSO_POL, HSO_POL
			if((Params->HVPol & VNegHPos) != (EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos)) { // V
				Cache_EP932_DE_Control |= EP932_DE_Control__VSO_POL; // Invert
			}
			else {
				Cache_EP932_DE_Control &= ~EP932_DE_Control__VSO_POL;
			}
			if((Params->HVPol & VPosHNeg) != (EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg)) { // H
				Cache_EP932_DE_Control |= EP932_DE_Control__HSO_POL; // Invert
			}
			else {
				Cache_EP932_DE_Control &= ~EP932_DE_Control__HSO_POL;
			}
			DBG_printf(("Set Sync mode to DE mode\r\n"));
			break;

		case SYNCMODE_HV:
			// Disable E_SYNC
			EP932_Reg_Clear_Bit(EP932_General_Control_4, EP932_General_Control_4__E_SYNC);
			// Enable DE_GEN
			Cache_EP932_DE_Control |= EP932_DE_Control__DE_GEN;
			//EP932_Reg_Write(EP932_DE_Control, &Cache_EP932_DE_Control, 1);

			// Regular VSO_POL, HSO_POL
			if((Params->HVPol & VNegHPos) != (EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos)) { // V
				Cache_EP932_DE_Control |= EP932_DE_Control__VSO_POL; // Invert
			}
			else {
				Cache_EP932_DE_Control &= ~EP932_DE_Control__VSO_POL;
			}
			if((Params->HVPol & VPosHNeg) != (EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg)) { // H
				Cache_EP932_DE_Control |= EP932_DE_Control__HSO_POL; // Invert
			}
			else {
				Cache_EP932_DE_Control &= ~EP932_DE_Control__HSO_POL;
			}

			// Set DE generation params
			if(Params->VideoSettingIndex < EP932_VDO_Settings_Max) {
				Cache_EP932_DE_Control &= ~0x03;

			#ifdef Little_Endian
				Cache_EP932_DE_Control |= ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[1];
				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[0];
				EP932_Reg_Write(EP932_DE_DLY, Temp_Data, 1);
			#else	// Big Endian
				Cache_EP932_DE_Control |= ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[0];
				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY)[1];
				EP932_Reg_Write(EP932_DE_DLY, Temp_Data, 1);
			#endif

				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_TOP)[0];
				EP932_Reg_Write(EP932_DE_TOP, Temp_Data, 1);

			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[0];
				Temp_Data[1] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[1];
				EP932_Reg_Write(EP932_DE_CNT, Temp_Data, 2);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[1];
				Temp_Data[1] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT)[0];
				EP932_Reg_Write(EP932_DE_CNT, Temp_Data, 2);
			#endif


			#ifdef Little_Endian
				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[1];
				Temp_Data[1] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[0];
				EP932_Reg_Write(EP932_DE_LIN, Temp_Data, 2);
			#else	// Big Endian
				Temp_Data[0] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[1];
				Temp_Data[1] = ((unsigned char *)&EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN)[0];
				EP932_Reg_Write(EP932_DE_LIN, Temp_Data, 2);
			#endif

				DBG_printf(("Update DE_GEN params %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_DLY));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_CNT));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_TOP));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].DE_Gen.DE_LIN));
				DBG_printf(("\r\n"));
			}
			else {
				DBG_printf(("ERROR: VideoCode overflow DE_GEN table\r\n"));
			}
			break;

		case SYNCMODE_Embeded:
			// Disable DE_GEN
			Cache_EP932_DE_Control &= ~EP932_DE_Control__DE_GEN;
			// Enable E_SYNC
			EP932_Reg_Set_Bit(EP932_General_Control_4, EP932_General_Control_4__E_SYNC);

			// Set E_SYNC params
			if(Params->VideoSettingIndex < EP932_VDO_Settings_Max) {

				Temp_Data[0] = EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.CTL;
				EP932_Reg_Write(EP932_Embedded_Sync, Temp_Data, 1);

				TempUSHORT = EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY;
				if(!(Params->Interface & 0x04)) { // Mux Mode
					TempUSHORT += 2;
				}

			#ifdef Little_Endian
				Temp_Data[0] = ((BYTE *)&TempUSHORT)[0];
				Temp_Data[1] = ((BYTE *)&TempUSHORT)[1];
				EP932_Reg_Write(EP932_H_Delay, Temp_Data, 2);
			#else	// Big Endian
				Temp_Data[0] = ((BYTE *)&TempUSHORT)[1];
				Temp_Data[1] = ((BYTE *)&TempUSHORT)[0];
				EP932_Reg_Write(EP932_H_Delay, Temp_Data, 2);
			#endif

				
			#ifdef Little_Endian
				Temp_Data[0] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[0];
				Temp_Data[1] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[1];
				EP932_Reg_Write(EP932_H_Width, Temp_Data, 2);
			#else	// Big Endian
				Temp_Data[0] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[1];
				Temp_Data[1] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH)[0];
				EP932_Reg_Write(EP932_H_Width, Temp_Data, 2);
			#endif

				Temp_Data[0] = EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_DLY;
				EP932_Reg_Write(EP932_V_Delay, Temp_Data, 1);
				//DBG_printf(("[0x85]= 0x%02X\r\n",(int)Temp_Data[0]));

				Temp_Data[0] = EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_WIDTH;
				EP932_Reg_Write(EP932_V_Width, Temp_Data, 1);
				//DBG_printf(("[0x86]= 0x%02X\r\n",(int)Temp_Data[0]));

			#ifdef Little_Endian
				Temp_Data[0] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[0];
				Temp_Data[1] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[1];
				EP932_Reg_Write(EP932_V_Off_Set, Temp_Data, 2);
			#else	// Big Endian
				Temp_Data[0] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[1];
				Temp_Data[1] = ((BYTE *)&EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST)[0];
				EP932_Reg_Write(EP932_V_Off_Set, Temp_Data, 2);
			#endif

				DBG_printf(("Update E_SYNC params 0x%02X", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.CTL));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_DLY));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.H_WIDTH));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_DLY));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_WIDTH));
				DBG_printf((", %u", (unsigned short)EP932_VDO_Settings[Params->VideoSettingIndex].E_Sync.V_OFST));
				DBG_printf(("\r\n"));


				for(i=0x80; i<=0x88; i++)
				{
					EP932_Reg_Read(i, Temp_Data, 1);
					DBG_printf(("EP932_reg[0x%02X]=0x%02X\r\n",(int)i,(int)Temp_Data[0]));
				}


				// Regular VSO_POL, HSO_POL
				if(EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VNegHPos) { // VNeg?
					Cache_EP932_DE_Control |= EP932_DE_Control__VSO_POL;
				}
				else {
					Cache_EP932_DE_Control &= ~EP932_DE_Control__VSO_POL;
				}
				if(EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.HVPol & VPosHNeg) { // HNeg?
					Cache_EP932_DE_Control |= EP932_DE_Control__HSO_POL;
				}
				else {
					Cache_EP932_DE_Control &= ~EP932_DE_Control__HSO_POL;
				}
			}
			else {
				DBG_printf(("ERROR: VideoCode overflow E_SYNC table\r\n"));
			}
			break;
	}
	EP932_Reg_Write(EP932_DE_Control, &Cache_EP932_DE_Control, 1);

	// Pixel Repetition
	EP932_Reg_Read(EP932_Pixel_Repetition_Control, Temp_Data, 1);
	Temp_Data[0] &= ~EP932_Pixel_Repetition_Control__PR;
	if(Params->VideoSettingIndex < EP932_VDO_Settings_Max) {
		Temp_Data[0] |= EP932_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x03;
	}
	EP932_Reg_Write(EP932_Pixel_Repetition_Control, Temp_Data, 1);

	// Color Space
	switch(Params->FormatIn) {
		default:
	 	case COLORFORMAT_RGB:
			EP932_Reg_Clear_Bit(EP932_General_Control_4, EP932_General_Control_4__YCC_IN | EP932_General_Control_4__422_IN);
			DBG_printf(("Set to RGB In\r\n"));
			break;
	 	case COLORFORMAT_YCC444:
			EP932_Reg_Set_Bit(EP932_General_Control_4, EP932_General_Control_4__YCC_IN);
			EP932_Reg_Clear_Bit(EP932_General_Control_4, EP932_General_Control_4__422_IN);
			DBG_printf(("Set to YCC444 In\r\n"));
			break;
	 	case COLORFORMAT_YCC422:
			EP932_Reg_Set_Bit(EP932_General_Control_4, EP932_General_Control_4__YCC_IN | EP932_General_Control_4__422_IN);
			DBG_printf(("Set to YCC422 In\r\n"));
			break;
	}
	switch(Params->FormatOut) {
		default:
	 	case COLORFORMAT_RGB:
			// Set to RGB
			if(Params->VideoSettingIndex < EP932_VDO_Settings_IT_Start) { // CE Timing
				EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__YCC_OUT | EP932_Color_Space_Control__422_OUT);
				EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__YCC_Range); // Output limit range RGB
			}
			else { // IT Timing
				EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__YCC_OUT | EP932_Color_Space_Control__422_OUT | EP932_Color_Space_Control__YCC_Range);
			}
			DBG_printf(("Set to RGB Out\r\n"));
			break;

	 	case COLORFORMAT_YCC444:
			// Set to YCC444
			EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__YCC_OUT);
			EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__422_OUT);
			DBG_printf(("Set to YCC444 Out\r\n"));
			break;
			
	 	case COLORFORMAT_YCC422:
			// Set to YCC422
			EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__YCC_OUT | EP932_Color_Space_Control__422_OUT);
			DBG_printf(("Set to YCC422 Out\r\n"));
			break;
	}

	// Color Space
	switch(Params->ColorSpace) {
		default:
	 	case COLORSPACE_601:
			// Set to 601
			EP932_Reg_Clear_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__COLOR);
			DBG_printf(("Set to 601 color definition\r\n"));
			break;

	 	case COLORSPACE_709:
			// Set to 709
			EP932_Reg_Set_Bit(EP932_Color_Space_Control, EP932_Color_Space_Control__COLOR);
			DBG_printf(("Set to 709 color definition\r\n"));
			break;
	}

	//
	// Update AVI Info Frame
	//
	// Read AVI Info Frame
	memset(Temp_Data, 0x00, 14);
	//Temp_Data[1] &= 0x60;
	switch(Params->FormatOut) {
		default:
	 	case COLORFORMAT_RGB:
			// Set AVI Info Frame to RGB
			Temp_Data[1] |= 0x00; // RGB
			break;

	 	case COLORFORMAT_YCC444:
			// Set AVI Info Frame to RGB
			Temp_Data[1] |= 0x40; // YCC 444
			break;
			
	 	case COLORFORMAT_YCC422:
			// Set AVI Info Frame to RGB
			Temp_Data[1] |= 0x20; // YCC 422
			break;
	}
	Temp_Data[1] |= 0x10; // Active Format Information
	//Temp_Data[2] &= 0xC0;

// add by eric.lu
	switch(Params->SCAN) {
		default:
		case 3:						// future 	
		case 0:						// NoData
			Temp_Data[1] &= ~0x03; 	
			break;
		case 1:						// overscan
			Temp_Data[1] |= 0x01; 	
			break;
		case 2:						// underscan
			Temp_Data[1] |= 0x02; 	
			break;
	}
// add of end

	switch(Params->ColorSpace) {
		default:
	 	case COLORSPACE_601:
			// Set AVI Info Frame to 601
			Temp_Data[2] |= 0x40;
			break;

	 	case COLORSPACE_709:
			// Set AVI Info Frame to 709
			Temp_Data[2] |= 0x80;
			break;
	}
	if(Params->VideoSettingIndex < EP932_VDO_Settings_Max) {
		Temp_Data[2] |= EP932_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x30;// AVI Info Frame : Picture Aspect Ratio
	}
	Temp_Data[2] |= Params->AFARate & 0x0F;// AVI Info Frame : Active format Aspect Ratio
	if(Params->VideoSettingIndex < EP932_VDO_Settings_IT_Start) {
		Temp_Data[4] |= EP932_VDO_Settings[Params->VideoSettingIndex].VideoCode;// AVI Info Frame : Vedio Identification code
	}
	if(Params->VideoSettingIndex < EP932_VDO_Settings_Max) {
		Temp_Data[5] |= (EP932_VDO_Settings[Params->VideoSettingIndex].AR_PR & 0x0C) >> 2;// AVI Info Frame : Pixel Repetition
	}

	// Write AVI Info Frame
	Temp_Data[0] = 0x91;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);	// checksum
	EP932_Reg_Write(EP932_AVI_Packet, Temp_Data, 14);

	DBG_printf(("AVI Info: "));
	for(i=0; i<6; ++i) {
		DBG_printf(("0x%0.2X, ", (int)Temp_Data[i] ));
	}
	DBG_printf(("\r\n"));

	//
	// Enable Video
	//
	EP932_Reg_Set_Bit(EP932_IIS_Control, EP932_IIS_Control__AVI_EN);
}

void HDMI_Tx_Audio_Config(PADO_PARAMS Params)
{
	int i;
	unsigned char N_CTS_Index;
	unsigned long N_Value, CTS_Value;
	ADSFREQ FinalFrequency;
	unsigned char FinalADSRate;

	DBG_printf(("\r\nStart Tx Audio Config\r\n"));

	//
	// Audio Settings
	//
	// Update WS_M, WS_POL, SCK_POL
	EP932_Reg_Read(EP932_IIS_Control, Temp_Data, 1);
	Temp_Data[0] &= ~0x07;
	Temp_Data[0] |= Params->Interface & 0x07;
	EP932_Reg_Write(EP932_IIS_Control, Temp_Data, 1);

	// Update Channel Status
	if(Params->Interface & 0x08) { // IIS

		Temp_Data[0] = 0;
		// Update Flat | IIS
		Temp_Data[0] |= EP932_ADO_Settings[Params->ChannelNumber].Flat;
		// Update Channel Number
		if(Params->ChannelNumber > 1) {	// 3 - 8 channel
			Temp_Data[0] |= EP932_Packet_Control__LAYOUT;
		}
		EP932_Reg_Write(EP932_Packet_Control, Temp_Data, 1); // Clear IIS
		Temp_Data[0] |= EP932_Packet_Control__IIS;
		EP932_Reg_Write(EP932_Packet_Control, Temp_Data, 1); // Set   IIS

		// Downsample Convert
		FinalADSRate = Params->ADSRate;
		switch(Params->ADSRate) {
			default:
			case 0: // Bypass
				//DBG_printf(("Audio ADS = 0\r\n"));
				FinalADSRate = 0;
				FinalFrequency = Params->InputFrequency;
				break;
			case 1: // 1/2
				//DBG_printf(("Audio ADS = 1_2\r\n"));
				switch(Params->InputFrequency) {
					default: // Bypass
						//DBG_printf(("Audio ADS = 0\r\n"));
						FinalADSRate = 0;
						FinalFrequency = Params->InputFrequency;
						break;
					case ADSFREQ_88200Hz:
						FinalFrequency = ADSFREQ_44100Hz;
						break;
					case ADSFREQ_96000Hz:
						FinalFrequency = ADSFREQ_48000Hz;
						break;
					case ADSFREQ_176400Hz:
						FinalFrequency = ADSFREQ_88200Hz;
						break;
					case ADSFREQ_192000Hz:
						FinalFrequency = ADSFREQ_96000Hz;
						break;
				}
				break;
			case 2: // 1/3
				//DBG_printf(("Audio ADS = 1_3\r\n"));
				switch(Params->InputFrequency) {
					default: // Bypass
						//DBG_printf(("Audio ADS = 0\r\n"));
						FinalADSRate = 0;
						FinalFrequency = Params->InputFrequency;
						break;
					case ADSFREQ_96000Hz:
						FinalFrequency = ADSFREQ_32000Hz;
						break;
				}
				break;
			case 3: // 1/4
				//DBG_printf(("Audio ADS = 1_4\r\n"));
				switch(Params->InputFrequency) {
					default: // Bypass
						//DBG_printf(("Audio ADS = 0\r\n"));
						FinalADSRate = 0;
						FinalFrequency = Params->InputFrequency;
						break;
					case ADSFREQ_176400Hz:
						FinalFrequency = ADSFREQ_44100Hz;
						break;
					case ADSFREQ_192000Hz:
						FinalFrequency = ADSFREQ_48000Hz;
						break;
				}
				break;
		}

		// Update Down Sample ADSRate
		EP932_Reg_Read(EP932_Pixel_Repetition_Control, Temp_Data, 1);
		Temp_Data[0] &= ~0x30;
		Temp_Data[0] |= (FinalADSRate << 4) & 0x30;
		EP932_Reg_Write(EP932_Pixel_Repetition_Control, Temp_Data, 1);


		// Set Channel Status
		memset(Temp_Data, 0x00, 5);
		Temp_Data[0] = (Params->NoCopyRight)? 0x04:0x00;
		Temp_Data[1] = 0x00; 			// Category code ??
		Temp_Data[2] = 0x00; 			// Channel number ?? | Source number ??
		Temp_Data[3] = FinalFrequency; 	// Clock accuracy ?? | Sampling frequency
		Temp_Data[4] = 0x01; 			// Original sampling frequency ?? | Word length ??
		EP932_Reg_Write(EP932_Channel_Status, Temp_Data, 5);

		DBG_printf(("CS Info: "));
		for(i=0; i<5; ++i) {
			DBG_printf(("0x%02X, ", (int)Temp_Data[i] ));
		}
		DBG_printf(("\r\n"));

		EP932_Reg_Set_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__CS_M);
	}
	else { // SPIDIF

		EP932_Reg_Set_Bit(EP932_Packet_Control, EP932_Packet_Control__IIS);
		EP932_Reg_Clear_Bit(EP932_Packet_Control, EP932_Packet_Control__FLAT3 | EP932_Packet_Control__FLAT2 | EP932_Packet_Control__FLAT1 | EP932_Packet_Control__FLAT0 |
			EP932_Packet_Control__IIS | EP932_Packet_Control__LAYOUT);

		// No Downsample
		FinalADSRate = 0;
		FinalFrequency = Params->InputFrequency;

		// Disable Down Sample and Bypass Channel Status
		EP932_Reg_Clear_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__ADSR | EP932_Pixel_Repetition_Control__CS_M);

		Params->ChannelNumber = 0;
	}

	// Set CTS/N
	if(Params->VideoSettingIndex < EP932_VDO_Settings_Max) {
		N_CTS_Index = EP932_VDO_Settings[Params->VideoSettingIndex].Pix_Freq_Type;
		if(EP932_VDO_Settings[Params->VideoSettingIndex].HVRes_Type.Vprd % 500) { // 59.94/60 Hz
			N_CTS_Index += Params->VFS;
			DBG_printf(("N_CTS_Index Shift %d\r\n", (int)Params->VFS));
		}
	}
	else {
		DBG_printf(("Use default N_CTS_Index\r\n"));
		N_CTS_Index = PIX_FREQ_25200KHz;
	}
	switch(FinalFrequency) {

		default:
		case ADSFREQ_32000Hz:
			DBG_printf(("Set to 32KHz"));
			N_Value = N_CTS_32K[N_CTS_Index].N;
			CTS_Value = N_CTS_32K[N_CTS_Index].CTS;
			break;
		case ADSFREQ_44100Hz:
			DBG_printf(("Set to 44.1KHz"));
			N_Value = N_CTS_44K1[N_CTS_Index].N;
			CTS_Value = N_CTS_44K1[N_CTS_Index].CTS;
			break;
		case ADSFREQ_48000Hz:
			DBG_printf(("Set to 48KHz"));
			N_Value = N_CTS_48K[N_CTS_Index].N;
			CTS_Value = N_CTS_48K[N_CTS_Index].CTS;
			break;
		case ADSFREQ_88200Hz:
			DBG_printf(("Set to 88.2KHz"));
			N_Value = N_CTS_44K1[N_CTS_Index].N * 2;
			CTS_Value = N_CTS_44K1[N_CTS_Index].CTS * 2;
			break;
		case ADSFREQ_96000Hz:
			DBG_printf(("Set to 96KHz"));
			N_Value = N_CTS_48K[N_CTS_Index].N * 2;
			CTS_Value = N_CTS_48K[N_CTS_Index].CTS * 2;
			break;
		case ADSFREQ_176400Hz:
			DBG_printf(("Set to 176.4KHz"));
			N_Value = N_CTS_44K1[N_CTS_Index].N * 4;
			CTS_Value = N_CTS_44K1[N_CTS_Index].CTS * 4;
			break;
		case ADSFREQ_192000Hz:
			DBG_printf(("Set to 192KHz"));
			N_Value = N_CTS_48K[N_CTS_Index].N * 4;
			CTS_Value = N_CTS_48K[N_CTS_Index].CTS * 4;
			break;
	}

	DBG_printf((", N[%d]=%d(0x%X)", (int)N_CTS_Index, N_Value, N_Value));
	DBG_printf((", CTS=%d(0x%X) \r\n", CTS_Value, CTS_Value));

	Temp_Data[0] = CTS_Value>>16;
	EP932_Reg_Write(EP932_CTS, Temp_Data, 1);
	Temp_Data[0] = CTS_Value>>8;
	EP932_Reg_Write(0x61, Temp_Data, 1);
	Temp_Data[0] = CTS_Value;
	EP932_Reg_Write(0x62, Temp_Data, 1);

	Temp_Data[0] = N_Value>>16;
	EP932_Reg_Write(EP932_N, Temp_Data, 1);
	Temp_Data[0] = N_Value>>8;
	EP932_Reg_Write(0x64, Temp_Data, 1);
	Temp_Data[0] = N_Value;
	EP932_Reg_Write(0x65, Temp_Data, 1);

// read for verify

	EP932_Reg_Read(EP932_CTS, Temp_Data, 1);
	DBG_printf(("EP932_CTS_0(Reg addr 0x60) = 0x%02X\r\n",(int)Temp_Data[0]));
	EP932_Reg_Read(0x61, Temp_Data, 1);
	DBG_printf(("EP932_CTS_1(Reg addr 0x61) = 0x%02X\r\n",(int)Temp_Data[0]));
	EP932_Reg_Read(0x62, Temp_Data, 1);
	DBG_printf(("EP932_CTS_2(Reg addr 0x62) = 0x%02X\r\n",(int)Temp_Data[0]));

	EP932_Reg_Read(EP932_N, Temp_Data, 1);
	DBG_printf(("EP932_N_0(Reg addr 0x63) = 0x%02X\r\n",(int)Temp_Data[0]));
	EP932_Reg_Read(0x64, Temp_Data, 1);
	DBG_printf(("EP932_N_1(Reg addr 0x64) = 0x%02X\r\n",(int)Temp_Data[0]));
	EP932_Reg_Read(0x65, Temp_Data, 1);
	DBG_printf(("EP932_N_2(Reg addr 0x65) = 0x%02X\r\n",(int)Temp_Data[0]));

	//
	// Update ADO Info Frame
	//
	// Set Default ADO Info Frame
	memset(Temp_Data, 0x00, 6);

	// Overwrite ADO Info Frame
	Temp_Data[1] = Params->ChannelNumber;
	Temp_Data[4] = EP932_ADO_Settings[Params->ChannelNumber].SpeakerMapping;

	// Write ADO Info Frame back
	Temp_Data[0] = 0x8F;
	for(i=1; i<6; ++i) {
		Temp_Data[0] += Temp_Data[i];
	}
	Temp_Data[0] = ~(Temp_Data[0] - 1);
	EP932_Reg_Write(EP932_ADO_Packet, Temp_Data, 6);

	DBG_printf(("ADO Info: "));
	for(i=0; i<6; ++i) {
		DBG_printf(("0x%0.2X, ", (int)Temp_Data[i] ));
	}
	DBG_printf(("\r\n"));

	EP932_Reg_Set_Bit(EP932_IIS_Control, EP932_IIS_Control__ACR_EN | EP932_IIS_Control__ADO_EN | EP932_IIS_Control__AUDIO_EN);
}

//--------------------------------------------------------------------------------------------------
//
// Hardware Interface
//
SMBUS_STATUS Key_Read(unsigned char ByteAddr, void *Data, unsigned int Size)
{
	return IIC_Read(IIC_Key_Addr, ByteAddr, Data, Size);
}

SMBUS_STATUS Key_Write(unsigned char ByteAddr, void *Data, unsigned int Size)
{
	return IIC_Write(IIC_Key_Addr, ByteAddr, Data, Size);
}

SMBUS_STATUS EP932_Reg_Read(unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	return IIC_Read(IIC_EP932_Addr, ByteAddr, Data, Size);
}

SMBUS_STATUS EP932_Reg_Write(unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
#if 0
	//DBG_printf(("EP932_Reg_Write 0x%02X, 0x%02X\r\n",(int)ByteAddr,(int)Data[0]));
	unsigned char temp_data[255];
	int i;
	int ret;
	IIC_Read(IIC_EP932_Addr,ByteAddr,temp_data,Size);	
	for(i =0;i<Size;i++)
		printk("[EP932_Reg_Write]before write [%x]%x\r\n",ByteAddr+i,temp_data[i]);

	ret = IIC_Write(IIC_EP932_Addr, ByteAddr, Data, Size);

	IIC_Read(IIC_EP932_Addr,ByteAddr,temp_data,Size);	
	for(i =0;i<Size;i++)
		printk("[EP932_Reg_Write]after write [%x]%x\r\n",ByteAddr+i,temp_data[i]);
	 
	return ret;
#else
	return  IIC_Write(IIC_EP932_Addr, ByteAddr, Data, Size);
#endif
}

SMBUS_STATUS EP932_Reg_Set_Bit(unsigned char ByteAddr, unsigned char BitMask)
{
#if 0
	unsigned char temp_data[255]; 
	int ret;
	IIC_Read(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
	printk("[EP932_Reg_Set_Bit]bfore set [%x]%x bitMask[%x]\r\n",ByteAddr,Temp_Data[0],BitMask);

	// Write back to Reg Reg_Addr
	Temp_Data[0] |= BitMask;

	ret = IIC_Write(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
	IIC_Read(IIC_EP932_Addr, ByteAddr, temp_data, 1);
	printk("[EP932_Reg_Set_Bit] after set [%x]%x\r\n",ByteAddr,temp_data[0]);

	return ret;
#else
	IIC_Read(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);

	// Write back to Reg Reg_Addr
	Temp_Data[0] |= BitMask;

	return IIC_Write(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
#endif
}

SMBUS_STATUS EP932_Reg_Clear_Bit(unsigned char ByteAddr, unsigned char BitMask)
{
#if 0
	unsigned char temp_data[255]; 
	int ret;
	IIC_Read(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
	printk("[EP932_Reg_Clear_Bit]bfore clear [%x]%x bitMask[%x]\r\n",ByteAddr,Temp_Data[0],BitMask);

	// Write back to Reg Reg_Addr
	Temp_Data[0] &= ~BitMask;

	ret = IIC_Write(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);

	IIC_Read(IIC_EP932_Addr, ByteAddr, temp_data, 1);
	printk("[EP932_Reg_Clear_Bit] after clear [%x]%x\r\n",ByteAddr,temp_data[0]);

	return ret;
#else
	IIC_Read(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);

	// Write back to Reg Reg_Addr
	Temp_Data[0] &= ~BitMask;

	return IIC_Write(IIC_EP932_Addr, ByteAddr, Temp_Data, 1);
#endif
}


//==================================================================================================
//
// Private Functions
//

SMBUS_STATUS IIC_Write(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////
	// How to implement this with Customer's I2C ?????????????????????????????
	// return 0; for success
	// return 2; for No_ACK
	// return 4; for Arbitration
	/////////////////////////////////////////////////////////////////////////////////////////////////

	//DBG_printf(("IIC_Write 0x%02X, 0x%02X\r\n",(int)ByteAddr,(int)Data[0]));

	// need to use customer's I2C  function
	//result = TLGI2C_WriteReg_EP932M(IICAddr, ByteAddr, Data, Size);
	
#if 0
	int result = 1;
	int i;
	unsigned long to, tn;
	to = get_ticks();

	//DBG_printf(("IIC_Write 0x%02X, 0x%02X\r\n",(int)ByteAddr,(int)Data[0]));

	// need to use customer's I2C  function
	//result = TLGI2C_WriteReg_EP932M(IICAddr, ByteAddr, Data, Size);
	if(iic_init(1,1,IICAddr,0,1,0)== FALSE)
		return 1;

	for(i = 0;i < Size;i++)
	{
		while(iic_write(1,ByteAddr+i,Data[i]) == 0)
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
		printk(KERN_ERR "[IIC_Write]: unable to allocate memory for EDID.\n");
		return -1; 
	}

   // print_mem(1,buf,Size + 1,"hdmi_w");


	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		printk(KERN_ERR "[IIC_Write]: can't get i2c adapter\n");

		return -1; 
	}

	if (i2c_transfer(adapter, msgs, 1) != 1)
		return -1; 

	kfree(buf);

	return 0;
}

SMBUS_STATUS IIC_Read(unsigned char IICAddr, unsigned char ByteAddr, unsigned char *Data, unsigned int Size)
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
	//result = TLGI2C_ReadReg_EP932M(IICAddr, ByteAddr, Data, Size);

/*
	if(result != 0)
	{
		DBG_printf(("EP932M IIC_Read error : 0x%02X, 0x%02X, 0x%02X, %d\r\n",(int)IICAddr,(int)ByteAddr,(int)Data[0],Size));
	}
*/
	if(iic_init(1,1,IICAddr,0,1,0)== FALSE)
		return 1;

	for(i=0;i<Size;i++)
	{
		while(iic_writeread(1,ByteAddr+i,&Data[i]) == 0)
		{
			tn = get_ticks();
			if(tn > to + 500000) break;
		}
	}

	return 0;
#endif

	struct i2c_adapter *adapter;
	unsigned char *buf = kmalloc(Size, GFP_KERNEL);
/*	
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
*/

	struct i2c_msg msgs1[] = { 
		{
			.addr   = IICAddr,
			.flags  = 0,
			.len            = 1,
			.buf            = &ByteAddr,
		},{
			.addr   = IICAddr,
			.flags  = I2C_M_RD,
			.len            = Size,
			.buf            = Data,
		}
	};

    //print_mem(1,&ByteAddr,1,"hdmi_reg");

	if (!buf)
	{
		printk(KERN_ERR "[IIC_Read]: unable to allocate memory for Read.\n");
		return -1; 
	}

	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		printk(KERN_ERR "[IIC_Read]: can't get i2c adapter\n");

		return -1; 
	}

	if (i2c_transfer(adapter, msgs1, 2) != 2){
		return -1; 
    }

    //print_mem(1,Data,Size,"hdmi_r");
/*
	if (i2c_transfer(adapter, msgs2, 1) != 1)
*/		return -1; 

	kfree(buf);

	return 0;
}
