/******************************************************************************\

          (c) Copyright Explore Semiconductor, Inc. Limited 2005 
                           ALL RIGHTS RESERVED 
 
--------------------------------------------------------------------------------

 Please review the terms of the license agreement before using this file.
 If you are not an authorized user, please destroy this source code file  
 and notify Explore Semiconductor Inc. immediately that you inadvertently 
 received an unauthorized copy.  

--------------------------------------------------------------------------------

  File        :  EP932Controller.c 

  Description :  EP932Controller program 
                 Control SFR directory and use HCI functions 

  Codeing     :  Shihken

  2008.09.04  :  1) Set the Version Number to 0.27 Beta 27
                 2) Fix the problem that EP932E send YUV format in DVI mode.

  2008.09.27  :  1) Set the Version Number to 0.28 Beta 28
                 2) Fix the problem that some Registers are not initialized.
                 3) Set Link_Status to No-ACK when there is no connection.

  2008.11.10  :  1) Set the Version Number to 0.31 Beta 31
                 2) Fix the interlace polarity problem in Embedded Sync Mode.

  2008.12.08  :  1) Set the Version Number to 0.32 Beta 32
                 2) Fix the Flat bits assignment in Layout 0.
                 3) Improve the IIC Slave speed and support EPF021.

  2008.12.19  :  1) Set the Version Number to 0.33 Beta 33
                 2) Fix the Front Porch timing in Embedded Sync Mode with Mux Mode.

  2009.02.05  :  1) Set the Version Number to 0.34 Beta 34
                 2) Improve the EDID gathering algorithm.

  2009.03.19  :  1) Set the Version Number to 0.35 Beta 35
                 2) Set CS Audio Length to default.

  2009.05.18  :  1) Set the Version Number to 0.36 Beta 36
                 2) Use A_MUTE and CTS_M instead of disabling all audio relatived packet.

\******************************************************************************/


#include "EP932api.h"
#include "Edid.h"
#include "DDC_If.h"
#include "EP932Controller.h"
#include "EP932SettingsData.h"


//
// Defines
//

//#define Enable_HDCP

#define AV_STABLE_TIME            1000

#define CmpInTolarence(Value, Compared, Tolarence) ((Value >= (Compared - Tolarence)) && (Value <= (Compared + Tolarence)))

#ifndef min
#define min(a,b) (a<b)? a:b
#endif

typedef enum {
	TXS_Search_EDID,
	TXS_Wait_Upstream,
	TXS_Stream,
	TXS_HDCP
} TX_STATE;

// HDCP Key  
unsigned char HDCP_Key[64][8];

//
// Global State and Flags
//
EP932C_REGISTER_MAP EP932C_Registers;

/*
// System flags
bdata unsigned char EP932C_Flags[2];

//sbit Event_HDMI_Int = EP932C_Flags[0]^0;
// ...

sbit is_Cap_HDMI = EP932C_Flags[0]^3;
sbit is_Cap_YCC444 = EP932C_Flags[0]^4;
sbit is_Cap_YCC422 = EP932C_Flags[0]^5;
sbit is_Connected = EP932C_Flags[0]^6;
sbit is_ReceiverSense = EP932C_Flags[0]^7;

sbit is_TimingChanging = EP932C_Flags[1]^0;
sbit is_VideoChanging = EP932C_Flags[1]^1;
sbit is_AudioChanging = EP932C_Flags[1]^2;
sbit is_HDCP_Info_BKSV_Rdy = EP932C_Flags[1]^3;
sbit is_Hot_Plug = EP932C_Flags[1]^4;
*/

unsigned char is_Cap_HDMI;
unsigned char is_Cap_YCC444;
unsigned char is_Cap_YCC422;
unsigned char is_Connected;
unsigned char is_ReceiverSense;

unsigned char is_TimingChanging;
unsigned char is_VideoChanging;
unsigned char is_AudioChanging;
unsigned char is_HDCP_Info_BKSV_Rdy;
unsigned char is_Hot_Plug;

//
// Global Data
//

// Temp Data
unsigned char ChkSum, VC_Temp, ConnectionState;
//int i;

unsigned char HTPLG_NOW = 0, HTPLG_LAST = 0;


// System Data
TX_STATE TX_State;
unsigned int HTP_TimeCount, VideoChg_TimeCount, AudioChg_TimeCount, ReadEDID_TimeCount;
unsigned char Process_Dispatch_ID;

unsigned char HP_ChangeCount, RSEN_ChangeCount, Backup_Analog_Test_Control;

VDO_PARAMS Video_Params;
ADO_PARAMS Audio_Params;
unsigned char Gamut_Packet_Header_Backup[3];

// Register
PEP932C_REGISTER_MAP pEP932C_Registers;

//
// Private Functions
//

void EP932Controller_Reset(void);

void TXS_RollBack_Wait_Upstream(void);
void TXS_RollBack_Stream(void);
void TXS_RollBack_HDCP(void);

// Hardware
void ReadInterruptFlags(void);
//void On_HDMI_Int();

EP932C_CALLBACK EP932C_GenerateInt;

//--------------------------------------------------------------------------------------------------------------------

void EP932Controller_Initial(PEP932C_REGISTER_MAP pEP932C_RegMap, EP932C_CALLBACK IntCall)
{
	// Save the Logical Hardware Assignment
	pEP932C_Registers = pEP932C_RegMap;
	EP932C_GenerateInt = IntCall;

	// EP932 Reset Control
	EP_EP932M_Reset();
	EP932_EnableHPInit();
	
	// Initial IIC	   
	EP932_If_Initial();

	// Reset Variables
	// bit
	is_Cap_HDMI = 0;
	is_Cap_YCC444 = is_Cap_YCC422 = 0;
	is_Connected = 0;
	is_VideoChanging = 0;
	is_AudioChanging = 0;
	// data
	TX_State = TXS_Search_EDID;
	HTP_TimeCount = 0;
	Process_Dispatch_ID = 0;
	VideoChg_TimeCount = 0;
	AudioChg_TimeCount = 0;
	ReadEDID_TimeCount = 0;
	HP_ChangeCount = 0;
	RSEN_ChangeCount = 0; 
	memset(Gamut_Packet_Header_Backup, 0, 3);

	// Reset all EP932C registers
	memset(pEP932C_Registers, 0, sizeof(EP932C_REGISTER_MAP));
	pEP932C_Registers->Video_Interface[0] = 0x80;
	pEP932C_Registers->Power_Control = EP932E_Power_Control__PD_HDMI;
	pEP932C_Registers->Audio_Interface = 0x10; // 2 Channel audio

	// Update Version Registers
	pEP932C_Registers->VendorID = 0x177A;
	pEP932C_Registers->DeviceID = 0x0932;
	pEP932C_Registers->Version_Major = VERSION_MAJOR;
	pEP932C_Registers->Version_Minor = VERSION_MINOR;
	DBG_printf(("Version %d.%d\r\n", (int)VERSION_MAJOR, (int)VERSION_MINOR ));
	// Initial HDCP Info
	memset(pEP932C_Registers->HDCP_AKSV, 0x00, sizeof(pEP932C_Registers->HDCP_AKSV));
	memset(pEP932C_Registers->HDCP_BKSV, 0x00, sizeof(pEP932C_Registers->HDCP_BKSV));
	
	// Update Configuration Registers
	EP932_Reg_Read(EP932_Configuration, DDC_Data, 1);
	pEP932C_Registers->Configuration = DDC_Data[0];

	// Set Revocation List address
	HDCP_Extract_BKSV_BCAPS3(pEP932C_Registers->HDCP_BKSV);
	HDCP_Extract_FIFO((unsigned char*)pEP932C_Registers->HDCP_KSV_FIFO, sizeof(pEP932C_Registers->HDCP_KSV_FIFO));
	HDCP_Stop();

	// Reset EP932 Control Program
	EP932Controller_Reset();
}

void EP932Controller_Reset(void)
{
#if defined(Enable_HDCP)
	SMBUS_STATUS status = SMBUS_STATUS_Success;
#endif
	// Reset Hardware
	DBG_printf(("Reset EP932\r\n"));

	EP_EP932M_Reset();
    pEP932C_Registers->System_Configuration = 0;
    pEP932C_Registers->System_Status = 0;
    
	EP932_EnableHPInit();
	
	// Initial Variables
	EP932_Reg_Set_Bit(EP932_Pixel_Repetition_Control, EP932_Pixel_Repetition_Control__OSCSEL);

#if defined(Enable_HDCP)

	// Read HDCP Key for EEPROM
	status = HDMI_Tx_Get_Key((unsigned char *)HDCP_Key);
	//DBG_printf(("Read HDCP Key = 0x%02X\r\n",(int)status));
	HDCP_Fake(0);
	pEP932C_Registers->System_Status &= ~EP932E_System_Status__KEY_FAIL;

	// Check HDCP key and up load the key
	if(status) {
		// Do not upload the default Key!
		pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
		pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
		DBG_printf(("No HDCP Key\r\n"));
	}
	else {
		// Check HDCP key and up load the key
		ChkSum = 0;
		for(i=0; i<328; ++i) {
			ChkSum += *((unsigned char *)HDCP_Key+i);
		}	
		DBG_printf(("HDCP Key Check Sum 0x%02X\r\n", (int)ChkSum ));
		if(HDCP_Key[3][7] != 0x50 || HDCP_Key[12][7] != 0x01 || ChkSum != 0x00) {// || HDCP_Key[40][0] != 0xA5) {
			HDCP_Fake(1);
			pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
			DBG_printf(("Check Key failed!\r\n"));
			pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
			//DBG_printf(("Disable HDCP \r\n"));
		}
		else {
			// Upload the key 0-39
			for(i=0; i<40; ++i) {
				DDC_Data[0] = (unsigned char)i;
				status |= EP932_Reg_Write(EP932_Key_Add, DDC_Data, 1);
				memcpy(DDC_Data,&HDCP_Key[i][0],7);
				status |= EP932_Reg_Write(EP932_Key_Data, DDC_Data, 7);
			}
			// Read and check	
			for(i=0; i<40; ++i) {
				DDC_Data[0] = (unsigned char)i;
				status |= EP932_Reg_Write(EP932_Key_Add, DDC_Data, 1);
				status |= EP932_Reg_Read(EP932_Key_Data, DDC_Data, 7);
				if((memcmp(DDC_Data,&HDCP_Key[i][0],7) != 0) || status) {
					// Test failed
					HDCP_Fake(1);
					pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
					DBG_printf(("Check Key failed!\r\n"));
					pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
					//DBG_printf(("Disable HDCP \r\n"));
					break;
				}
			}
			// Upload final KSV 40
			DDC_Data[0] = 40;
			status |= EP932_Reg_Write(EP932_Key_Add, DDC_Data, 1);
			memcpy(DDC_Data,&HDCP_Key[40][0],7);
			status |= EP932_Reg_Write(EP932_Key_Data, DDC_Data, 7);
			// Read back and check
	    	if(!HDMI_Tx_read_AKSV(pEP932C_Registers->HDCP_AKSV)) {
				// Test failed
				HDCP_Fake(1);
				pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
				DBG_printf(("Check KSV failed!\r\n"));
				pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS;
				//DBG_printf(("Disable HDCP \r\n"));
			}
		}	
	}

#else

	pEP932C_Registers->System_Status |= EP932E_System_Status__KEY_FAIL;
	pEP932C_Registers->System_Configuration |= EP932E_System_Configuration__HDCP_DIS|EP932E_System_Configuration__FORCE_HDMI_CAP;

#endif 

	// EP932 Interface Reset
	EP932_If_Reset();

	// Internal Variable Reset
	// bit
	//Event_HDMI_Int = 0;
	is_ReceiverSense = 0;

	// data
	Backup_Analog_Test_Control = 0;
	if(TX_State > TXS_Search_EDID) {
		DBG_printf(("\r\nState Transist: Reset -> [TXS_Wait_Upstream]\r\n"));
		TX_State = TXS_Wait_Upstream;
	}

	DBG_printf(("EP932Controller_Reset finish\r\n"));
}

void EP932Controller_Timer(void)
{
	++HTP_TimeCount;
	if(is_VideoChanging) ++VideoChg_TimeCount;
	if(is_AudioChanging) ++AudioChg_TimeCount;
	if(TX_State == TXS_HDCP) HDCP_Timer();
	++ReadEDID_TimeCount;
}

unsigned int EP932_HotPlugMonitorInt(void)
{
	unsigned char temp;
	is_Hot_Plug = HDMI_Tx_HTPLG();

	EP932_Reg_Read(EP932_General_Control_2, &temp, 1);

	if(temp & 0x1)
		return is_Hot_Plug;	
	else 
		return -1;
}

unsigned int EP932_HotPlugMonitor(void)
{
	is_Hot_Plug = HDMI_Tx_HTPLG();
	return is_Hot_Plug;

}

unsigned char EP932Controller_Task(void)
{
    //DBG_printf(("EP932 EP932Controller_Task \r\n"));

	// Read Interrupt Flag and updat the internal information
	ReadInterruptFlags();

	// Polling Hot-Plug every 80ms
	if(HTP_TimeCount > 80/EP932C_TIMER_PERIOD) {
		HTP_TimeCount = 0;
		
		ConnectionState = HDMI_Tx_HTPLG();

		HTPLG_NOW = ConnectionState;
		if(HTPLG_LAST != HTPLG_NOW)
		{
			HTPLG_LAST = HTPLG_NOW;
			if(HTPLG_NOW == 0)
			{
				DBG_printf(("Without HotPlug\r\n"));	
				EP_HDMI_DumpMessage();				
			}
			else
			{
				DBG_printf(("Detect HotPlug \r\n"));	
			}
		}
		
		is_Hot_Plug = (ConnectionState == 1)? 1:0;
		if(is_Connected != ((ConnectionState)?1:0) ) {
			if(HP_ChangeCount++ >= 1) { // Accept continuous 1 error = 1*80 ms = 80 ms (Skip when low period < 80 ms)
				HP_ChangeCount = 0;

				is_Connected = ((ConnectionState)?1:0);
			}
		}
		else {
			HP_ChangeCount = 0;
		}
		if(is_Hot_Plug) {
			pEP932C_Registers->System_Status |= EP932E_System_Status__HTPLG;
		}
		else {
			pEP932C_Registers->System_Status &= ~EP932E_System_Status__HTPLG;
		}

		is_ReceiverSense = HDMI_Tx_RSEN(); // Only valid when TX is powered on

		if(TX_State > TXS_Wait_Upstream) { // Powered Up and have Input
			
			// Update RSEN
			if(is_ReceiverSense) {
				pEP932C_Registers->System_Status |= EP932E_System_Status__RSEN;
			}
			else {
				pEP932C_Registers->System_Status &= ~EP932E_System_Status__RSEN;
			}
			RSEN_ChangeCount = 0;

			// Read HSO VSO POL information
			EP932_Reg_Read(EP932_General_Control_4, DDC_Data, 1);
			Video_Params.HVPol = 0;//DDC_Data[0] & (EP932_DE_Control__VSO_POL | EP932_DE_Control__HSO_POL);	
		}
		else {
			if(RSEN_ChangeCount++ >= 8) { // Accept continuous 8 error = 8*80 ms = 640 ms (Skip when low period < 640 ms)
				RSEN_ChangeCount = 0;

				pEP932C_Registers->System_Status &= ~EP932E_System_Status__RSEN;
			}
		}
	}

	//
	// Update EP932 Registers according to the System Process
	//
	//DBG_printf(("TX_State=%d \n",TX_State));
	switch(TX_State) {
		case TXS_Search_EDID:
			if(is_Connected) {
				if(ReadEDID_TimeCount > 200/EP932C_TIMER_PERIOD) {
					unsigned char EDID_DDC_Status = 0;

					// Confirm Hot-Plug (time-out after 1s)
					if(!is_Hot_Plug) {
						if(ReadEDID_TimeCount <= 1000/EP932C_TIMER_PERIOD) break;
						DBG_printf(("WARNING: EDID detected without Hot-Plug for 1s\r\n"));
					}

					// Read EDID
					DBG_printf(("\r\nState Transist: Read EDID -> [TXS_Wait_Upstream] 0x%x\r\n",pEP932C_Registers->System_Configuration));
					memset(pEP932C_Registers->Readed_EDID, 0xFF, 256);
                    if(!(pEP932C_Registers->System_Configuration & EP932E_System_Configuration__FORCE_HDMI_CAP)){
    					EDID_DDC_Status = Downstream_Rx_read_EDID(pEP932C_Registers->Readed_EDID);
    					
    					if(EDID_DDC_Status) {
    						//if(EDID_DDC_Status == EDID_STATUS_NoAct) {
    						if(EDID_DDC_Status != EDID_STATUS_ChecksumError) {
    							DBG_printf(("WARNING: EDID read failed 0x%02X\r\n", (int)EDID_DDC_Status));
    							if(ReadEDID_TimeCount <= 500/EP932C_TIMER_PERIOD) break;
    						}
    					}
                    }
					ReadEDID_TimeCount = 0;
 
					// Set Output
					if(pEP932C_Registers->System_Configuration & EP932E_System_Configuration__FORCE_HDMI_CAP) {
						is_Cap_HDMI = 1;
					}
					else {
						is_Cap_HDMI = EDID_GetHDMICap(pEP932C_Registers->Readed_EDID);
					}
					if(is_Cap_HDMI) {			
						DBG_printf(("Support HDMI"));

						// Default Capability
						is_Cap_YCC444 =	is_Cap_YCC422 = 0;
						pEP932C_Registers->EDID_ASFreq = 0x07;
						pEP932C_Registers->EDID_AChannel = 1;

						pEP932C_Registers->EDID_VideoDataAddr = 0x00;
						pEP932C_Registers->EDID_AudioDataAddr = 0x00;
						pEP932C_Registers->EDID_SpeakerDataAddr = 0x00;
						pEP932C_Registers->EDID_VendorDataAddr = 0x00;

						if(!EDID_DDC_Status) {

							if(pEP932C_Registers->Readed_EDID[131] & 0x20) {	// Support YCC444
								is_Cap_YCC444 = 1;
								DBG_printf((" YCC444"));
							}
							if(pEP932C_Registers->Readed_EDID[131] & 0x10) {	// Support YCC422
								is_Cap_YCC422 = 1;
								DBG_printf((" YCC422"));
							}
							DBG_printf(("\r\n"));
							pEP932C_Registers->EDID_ASFreq = EDID_GetPCMFreqCap(pEP932C_Registers->Readed_EDID);
							DBG_printf(("EDID ASFreq = 0x%02X\r\n",(int)pEP932C_Registers->EDID_ASFreq));

							pEP932C_Registers->EDID_AChannel = EDID_GetPCMChannelCap(pEP932C_Registers->Readed_EDID);
							DBG_printf(("EDID AChannel = 0x%02X\r\n",(int)pEP932C_Registers->EDID_AChannel));

							pEP932C_Registers->EDID_VideoDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x40);
							pEP932C_Registers->EDID_AudioDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x20);
							pEP932C_Registers->EDID_SpeakerDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x80);
							pEP932C_Registers->EDID_VendorDataAddr = EDID_GetDataBlockAddr(pEP932C_Registers->Readed_EDID, 0x60);
						}
					}
					else {
						DBG_printf(("Support DVI RGB only\r\n"));
						is_Cap_YCC444 =	is_Cap_YCC422 = 0;
						pEP932C_Registers->EDID_ASFreq = pEP932C_Registers->EDID_AChannel = 0;
					}

					if(is_Cap_HDMI)
						pEP932C_Registers->EDID_Status = EDID_DDC_Status | EP932E_EDID_Status__HDMI;
					else
						pEP932C_Registers->EDID_Status = EDID_DDC_Status;
					DBG_printf(("Support Max Audio Channel %d\r\n", (int)pEP932C_Registers->EDID_AChannel+1));
					DBG_printf(("Support Audio Freq 0x%02X\r\n", (int)pEP932C_Registers->EDID_ASFreq));

					// Report EDID Change
					pEP932C_Registers->Interrupt_Flags |= EP932E_Interrupt_Flags__EDID_CHG;
					if(EP932C_GenerateInt && (pEP932C_Registers->Interrupt_Enable & EP932E_Interrupt_Enable__EDID_CHG) ) EP932C_GenerateInt();
	
					TX_State = TXS_Wait_Upstream;
				}
			}
			else {	
				pEP932C_Registers->EDID_Status = EDID_STATUS_NoAct;
				ReadEDID_TimeCount = 0;
			}
			break;
			
		case TXS_Wait_Upstream:

			if(!is_Connected) {

				TXS_RollBack_Wait_Upstream();
				TX_State = TXS_Search_EDID;
			}
			else if(!(pEP932C_Registers->Power_Control & (EP932E_Power_Control__PD_HDMI | EP932E_Power_Control__PD_TOT)) ) {
				DBG_printf(("\r\nState Transist: Power Up -> [TXS_Stream]\r\n"));							

				// Power Up
				HDMI_Tx_Power_Up();

				TX_State = TXS_Stream;
			}
			else {
				// Check Force HDMI bit
				if(!is_Cap_HDMI) {
					if(pEP932C_Registers->System_Configuration & EP932E_System_Configuration__FORCE_HDMI_CAP) {
						TXS_RollBack_Wait_Upstream();
						TX_State = TXS_Search_EDID;
					}
				}
			}
			break;

		case TXS_Stream:

#if defined(Enable_HDCP)
			if(!is_HDCP_Info_BKSV_Rdy && is_ReceiverSense && is_Hot_Plug) {
				// Get HDCP Info
		    	if(!Downstream_Rx_read_BKSV(pEP932C_Registers->HDCP_BKSV)) {
					pEP932C_Registers->HDCP_Status = EP932E_HDCP_Status__BKSV;
				}
				pEP932C_Registers->HDCP_BCAPS3[0] = Downstream_Rx_BCAPS();
				is_HDCP_Info_BKSV_Rdy = 1;
			}
#endif			

			if(!is_Connected) {

				TXS_RollBack_Stream();
				TXS_RollBack_Wait_Upstream();
				TX_State = TXS_Search_EDID;
			}
			else if(pEP932C_Registers->Power_Control & (EP932E_Power_Control__PD_HDMI | EP932E_Power_Control__PD_TOT) ) {
				pEP932C_Registers->Power_Control |= EP932E_Power_Control__PD_HDMI;

				TXS_RollBack_Stream();
				TX_State = TXS_Wait_Upstream;
			}
			
#if defined(Enable_HDCP)
			else if(!((pEP932C_Registers->System_Configuration & EP932E_System_Configuration__HDCP_DIS) || is_VideoChanging) && is_ReceiverSense) {
				// Enable mute for transmiter video and audio
				HDMI_Tx_Mute_Enable();

				DBG_printf(("\r\nState Transist: Start HDCP -> [TXS_HDCP]\r\n"));
				TX_State = TXS_HDCP;
			}
#endif			

			break;
			
#if defined(Enable_HDCP)
		case TXS_HDCP:
		
			if(!is_Connected || !is_Hot_Plug) {

				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				TXS_RollBack_Wait_Upstream();
				TX_State = TXS_Search_EDID;
			}
			else if(pEP932C_Registers->Power_Control & (EP932E_Power_Control__PD_HDMI | EP932E_Power_Control__PD_TOT) ) {
				pEP932C_Registers->Power_Control |= EP932E_Power_Control__PD_HDMI;

				TXS_RollBack_HDCP();
				TXS_RollBack_Stream();
				TX_State = TXS_Wait_Upstream;
			}
			else if((pEP932C_Registers->System_Configuration & EP932E_System_Configuration__HDCP_DIS) || is_VideoChanging) {
	
				TXS_RollBack_HDCP();
				TX_State = TXS_Stream;
			}
			else {
				pEP932C_Registers->HDCP_State = HDCP_Authentication_Task(is_ReceiverSense && is_Hot_Plug);
				pEP932C_Registers->HDCP_Status = HDCP_Get_Status();
			}
			break;
#endif

	}

	//
	// Update EP932 Registers for any time
	//

	// Mute Control
	if( (pEP932C_Registers->System_Configuration & EP932E_System_Configuration__AUDIO_DIS) || (TX_State < TXS_Stream) || is_VideoChanging  || is_AudioChanging ) {
		HDMI_Tx_AMute_Enable();	
	}
	else {
		HDMI_Tx_AMute_Disable();
	}
	
	if( (pEP932C_Registers->System_Configuration & EP932E_System_Configuration__VIDEO_DIS) || (TX_State < TXS_Stream) || is_VideoChanging ) {
		HDMI_Tx_VMute_Enable();		
	}
	else {
		HDMI_Tx_VMute_Disable();
	}

	// HDMI Mode
	if(!is_Cap_HDMI || (pEP932C_Registers->System_Configuration & EP932E_System_Configuration__HDMI_DIS) ) {
		HDMI_Tx_DVI();		// Set to DVI mode (The Info Frame and Audio Packets would not be send)
	}
	else {
		HDMI_Tx_HDMI();	// Set to HDMI mode
	}

	++Process_Dispatch_ID;
	if(Process_Dispatch_ID > 2) Process_Dispatch_ID = 0;

	switch(Process_Dispatch_ID) {

		case 0:
			//
			// Update Video Params
			//
		
			// Video Interface
			Video_Params.Interface = pEP932C_Registers->Video_Interface[0];
		
			// Video Timing
			if(pEP932C_Registers->Video_Input_Format[0]) { 
				// Manul set the Video Timing
				if(pEP932C_Registers->Video_Input_Format[0] < 128) {
					Video_Params.VideoSettingIndex = pEP932C_Registers->Video_Input_Format[0];
				}
				else {
					Video_Params.VideoSettingIndex = pEP932C_Registers->Video_Input_Format[0] - (128 - EP932_VDO_Settings_IT_Start);
				}
			} 
		
			// Select Sync Mode
			Video_Params.SyncMode = (pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__SYNC) >> 2;
		
			// Select Color Space
			switch(pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__COLOR) {
				default:
				case EP932E_Video_Interface_Setting_1__COLOR__Auto:
					switch(Video_Params.VideoSettingIndex) {
						case  4: case  5: case 16: case 19: case 20: case 31: case 32: 
						case 33: case 34: case 39: case 40: case 41: case 46: case 47:		// HD Timing
							Video_Params.ColorSpace = COLORSPACE_709;
							break;
		
						default:
							if(Video_Params.VideoSettingIndex && Video_Params.VideoSettingIndex < EP932_VDO_Settings_IT_Start) { // SD Timing
								Video_Params.ColorSpace = COLORSPACE_601;
							}
							else {															// IT Timing
								Video_Params.ColorSpace = COLORSPACE_709;
							}
					}
					break;
				case EP932E_Video_Interface_Setting_1__COLOR__601:
					Video_Params.ColorSpace = COLORSPACE_601;
					break;
				case EP932E_Video_Interface_Setting_1__COLOR__709:
					Video_Params.ColorSpace = COLORSPACE_709;
					break;
			}
		
			// Set Input Format
			switch(pEP932C_Registers->Video_Interface[1] & EP932E_Video_Interface_Setting_1__VIN_FMT) {
				default:
				case EP932E_Video_Interface_Setting_1__VIN_FMT__RGB:
					Video_Params.FormatIn = COLORFORMAT_RGB;
					Video_Params.FormatOut = COLORFORMAT_RGB;
					break;
				case EP932E_Video_Interface_Setting_1__VIN_FMT__YCC444:
					Video_Params.FormatIn = COLORFORMAT_YCC444;
					if(is_Cap_YCC444) {
						Video_Params.FormatOut = COLORFORMAT_YCC444;
					}
					else if(is_Cap_YCC422) {
						Video_Params.FormatOut = COLORFORMAT_YCC422;
					}
					else {
						Video_Params.FormatOut = COLORFORMAT_RGB;
					}
					break;
				case EP932E_Video_Interface_Setting_1__VIN_FMT__YCC422:
					Video_Params.FormatIn = COLORFORMAT_YCC422;
					if(is_Cap_YCC444) {
						Video_Params.FormatOut = COLORFORMAT_YCC444;
					}
					else if(is_Cap_YCC422) {
						Video_Params.FormatOut = COLORFORMAT_YCC422;
					}
					else {
						Video_Params.FormatOut = COLORFORMAT_RGB;
					}
					break;
			}

	//add by eric.lu
	
			// Set Output Format
			switch(pEP932C_Registers->Video_Output_Format) {
				default:
				case 0:		// Auto, don't need change setting.
					break;
					
				case 1:		// Force to YUV444 output format
					Video_Params.FormatOut = COLORFORMAT_YCC444;
					break;
					
				case 2:		// Force to YUV422 output format
					Video_Params.FormatOut = COLORFORMAT_YCC422;
					break;
					
				case 3:		// Force to RGB444 output format
					Video_Params.FormatOut = COLORFORMAT_RGB;
					break;
			}
			
	// end of add

			// DVI mode settings overwrite
			if(!is_Cap_HDMI || (pEP932C_Registers->System_Configuration & EP932E_System_Configuration__HDMI_DIS) ) {
				Video_Params.FormatOut = COLORFORMAT_RGB;
			}
		
			// AFAR
			Video_Params.AFARate = ((pEP932C_Registers->Video_Input_Format[1] & EP932E_Video_Input_Format_1__AFAR) >> 4) | 0x08;

		// add by eric.lu
			// SCAN			
			Video_Params.SCAN = (pEP932C_Registers->Video_Input_Format[1] & EP932E_Video_Input_Format_1__SCAN);
		// end of add
		
			// Video Change
			if(memcmp(&Video_Params, &pEP932C_Registers->Video_Params_Backup, sizeof(VDO_PARAMS)) != 0) {
				if(memcmp(&Video_Params, &pEP932C_Registers->Video_Params_Backup, 6) != 0) {
					is_TimingChanging = 1;
				}
//				DBG_printf(("Video_Params new: interface 0x%02X, Vindex 0x%02X, HV 0x%02X, mode 0x%02X, Fin 0x%02X, Fout 0x%02X, color 0x%02X, AFAR 0x%02X\r\n",(int)Video_Params.Interface, (int)Video_Params.VideoSettingIndex, (int)Video_Params.HVPol ,(int)Video_Params.SyncMode, (int)Video_Params.FormatIn, (int)Video_Params.FormatOut, (int)Video_Params.ColorSpace, (int)Video_Params.AFARate));
//				DBG_printf(("Video_Params old: interface 0x%02X, Vindex 0x%02X, HV 0x%02X, mode 0x%02X, Fin 0x%02X, Fout 0x%02X, color 0x%02X, AFAR 0x%02X\r\n",(int)pEP932C_Registers->Video_Params_Backup.Interface, (int)pEP932C_Registers->Video_Params_Backup.VideoSettingIndex, (int)pEP932C_Registers->Video_Params_Backup.HVPol ,(int)pEP932C_Registers->Video_Params_Backup.SyncMode, (int)pEP932C_Registers->Video_Params_Backup.FormatIn, (int)pEP932C_Registers->Video_Params_Backup.FormatOut, (int)pEP932C_Registers->Video_Params_Backup.ColorSpace, (int)pEP932C_Registers->Video_Params_Backup.AFARate));
				
				pEP932C_Registers->Video_Params_Backup = Video_Params;
				
				VideoChg_TimeCount = 0;
				is_VideoChanging = 1;
			}
		
			// Video Change Debouncing
			if(is_VideoChanging) {				
				if(VideoChg_TimeCount > AV_STABLE_TIME/EP932C_TIMER_PERIOD) {

					DBG_printf(("### VideoChanging \r\n"));

					if(is_TimingChanging) 
						EP932Controller_Reset();

					HDMI_Tx_Video_Config(&Video_Params);

					if(is_TimingChanging) {
						if(!is_AudioChanging) 
							HDMI_Tx_Audio_Config(&Audio_Params);
					}

					is_TimingChanging = 0;
					is_VideoChanging = 0;
					VideoChg_TimeCount = 0;
					
					// Report Video Change
					pEP932C_Registers->Interrupt_Flags |= EP932E_Interrupt_Flags__VIDEO_CHG;
					if(EP932C_GenerateInt && (pEP932C_Registers->Interrupt_Enable & EP932E_Interrupt_Enable__VIDEO_CHG) ) EP932C_GenerateInt();
				}
			}
			break;

		case 1:
			//
			// Update Audio Params
			//
			Audio_Params.Interface = pEP932C_Registers->Audio_Interface & 0x0F; // IIS, WS_M, WS_POL, SCK_POL
			Audio_Params.VideoSettingIndex = Video_Params.VideoSettingIndex;

			// Update Audio Channel Number
			if(EP932_VDO_Settings[Video_Params.VideoSettingIndex].Pix_Freq_Type <= PIX_FREQ_27027KHz) {
				Audio_Params.ChannelNumber = 1;
			}
			else {
				Audio_Params.ChannelNumber = min(((pEP932C_Registers->Audio_Interface & 0x70) >> 4), pEP932C_Registers->EDID_AChannel);
			}

			// Update VFS
			if(Audio_Params.VideoSettingIndex < EP932_VDO_Settings_IT_Start) {
				// Pixel Clock Type shift (59.94/60)
				Audio_Params.VFS = (pEP932C_Registers->Video_Input_Format[1] & EP932E_Video_Input_Format_1__VIF)? 1:0;
			}
			else {
				Audio_Params.VFS = 0;
			}
			Audio_Params.NoCopyRight = (pEP932C_Registers->Audio_Input_Format & EP932E_Audio_Input_Format__NoCopyRight)?1:0;
		
			// Write Frequency info (Use ADO_FREQ or Auto)
			switch( pEP932C_Registers->Audio_Input_Format & EP932E_Audio_Input_Format__ADO_FREQ ) {
		
				case EP932E_Audio_Input_Format__ADO_FREQ__32000Hz:
					Audio_Params.InputFrequency = ADSFREQ_32000Hz;
					// Disable Down Sample
					Audio_Params.ADSRate = 0;
					break;
		
				default:
				case EP932E_Audio_Input_Format__ADO_FREQ__44100Hz:
					Audio_Params.InputFrequency = ADSFREQ_44100Hz;
					// Disable Down Sample
					Audio_Params.ADSRate = 0;
					break;
		
				case EP932E_Audio_Input_Format__ADO_FREQ__48000Hz:
					Audio_Params.InputFrequency = ADSFREQ_48000Hz;
					// Disable Down Sample
					Audio_Params.ADSRate = 0;
					break;
		
				case EP932E_Audio_Input_Format__ADO_FREQ__88200Hz:
					Audio_Params.InputFrequency = ADSFREQ_88200Hz;
					if(pEP932C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
						// Disable Down Sample
						Audio_Params.ADSRate = 0;
					}
					else {
						// Enable Down Sample 1/2
						Audio_Params.ADSRate = 1;
					}
					break;
		
				case EP932E_Audio_Input_Format__ADO_FREQ__96000Hz:
					Audio_Params.InputFrequency = ADSFREQ_96000Hz;
					if(pEP932C_Registers->EDID_ASFreq & 0x10) { // 96kHz
						// Disable Down Sample
						Audio_Params.ADSRate = 0;
					}
					else {
						if(pEP932C_Registers->EDID_ASFreq & 0x04) { // 48kHz
							// Enable Down Sample 1/2
							Audio_Params.ADSRate = 1;
						}
						else {
							// Enable Down Sample 1/3
							Audio_Params.ADSRate = 2;
						}
					}
					break;
		
				case EP932E_Audio_Input_Format__ADO_FREQ__176400Hz:
					Audio_Params.InputFrequency = ADSFREQ_176400Hz;
					if(pEP932C_Registers->EDID_ASFreq & 0x20) { // 176kHz
						// Disable Down Sample
						Audio_Params.ADSRate = 0;
					}
					else {
						if(pEP932C_Registers->EDID_ASFreq & 0x08) { // 88.2kHz
							// Enable Down Sample 1/2
							Audio_Params.ADSRate = 1;
						}
						else {
							// Enable Down Sample 1/4
							Audio_Params.ADSRate = 3;
						}
					}
					break;
		
				case EP932E_Audio_Input_Format__ADO_FREQ__192000Hz:
					Audio_Params.InputFrequency = ADSFREQ_192000Hz;
					if(pEP932C_Registers->EDID_ASFreq & 0x40) { // 192kHz
						// Disable Down Sample
						Audio_Params.ADSRate = 0;
					}
					else {
						if(pEP932C_Registers->EDID_ASFreq & 0x10) { // 96kHz
							// Enable Down Sample 1/2
							Audio_Params.ADSRate = 1;
						}
						else {
							// Enable Down Sample 1/4
							Audio_Params.ADSRate = 3;
						}
					}
					break;
			}
		
			// Audio Change
			if(memcmp(&Audio_Params, &pEP932C_Registers->Audio_Params_Backup, sizeof(ADO_PARAMS)) != 0) {
				pEP932C_Registers->Audio_Params_Backup = Audio_Params;
		
				AudioChg_TimeCount = 0;
				is_AudioChanging = 1;
			}

			// Audio Change Debouncing
			if(is_AudioChanging) {
				if(AudioChg_TimeCount > AV_STABLE_TIME/EP932C_TIMER_PERIOD) {
					HDMI_Tx_Audio_Config(&Audio_Params);
					is_AudioChanging = 0;
					AudioChg_TimeCount = 0;
					
					// Report Audio Change
					pEP932C_Registers->Interrupt_Flags |= EP932E_Interrupt_Flags__AUDIO_CHG;
					if(EP932C_GenerateInt && (pEP932C_Registers->Interrupt_Enable & EP932E_Interrupt_Enable__AUDIO_CHG) ) EP932C_GenerateInt();
				}
			}
			break;

		case 2:

			// Update TREG
			if(pEP932C_Registers->Analog_Test_Control != Backup_Analog_Test_Control) {
				Backup_Analog_Test_Control = pEP932C_Registers->Analog_Test_Control;
		
				if(pEP932C_Registers->Analog_Test_Control & 0x01) {
					EP932_Reg_Set_Bit(EP932_Color_Space_Control, 0x01);
				}
				else {
					EP932_Reg_Clear_Bit(EP932_Color_Space_Control, 0x01);
				}
				if(pEP932C_Registers->Analog_Test_Control & 0x02) {
					EP932_Reg_Set_Bit(EP932_Color_Space_Control, 0x02);
				}
				else {
					EP932_Reg_Clear_Bit(EP932_Color_Space_Control, 0x02);
				}
			}
			break;
	}

	// Return the status
	if(pEP932C_Registers->Power_Control & (EP932E_Power_Control__PD_HDMI | EP932E_Power_Control__PD_TOT)) {
		return EP932C_TASK_Idle;
	}
	else {
		return EP932C_TASK_Pending;
	}
}

void TXS_RollBack_Wait_Upstream(void)
{
	DBG_printf(("\r\nState Rollback: Reset EDID -> [TXS_Search_EDID]\r\n"));

	// Reset EDID
	memset(pEP932C_Registers->Readed_EDID, 0xFF, 256);

	// Report EDID Change
	pEP932C_Registers->Interrupt_Flags |= EP932E_Interrupt_Flags__EDID_CHG;
	if(EP932C_GenerateInt && (pEP932C_Registers->Interrupt_Enable & EP932E_Interrupt_Enable__EDID_CHG) ) EP932C_GenerateInt();
	ReadEDID_TimeCount = 0;
}

void TXS_RollBack_Stream(void)
{
	DBG_printf(("\r\nState Rollback: Power Down -> [TXS_Wait_Upstream]\r\n"));

	// Power Down
	HDMI_Tx_Power_Down();

	// Reset HDCP Info
	memset(pEP932C_Registers->HDCP_BKSV, 0x00, sizeof(pEP932C_Registers->HDCP_BKSV));
	is_HDCP_Info_BKSV_Rdy = 0;
}

void TXS_RollBack_HDCP(void)
{
	DBG_printf(("\r\nState Rollback: Stop HDCP -> [TXS_Stream]\r\n"));

	HDCP_Stop();
	pEP932C_Registers->HDCP_Status = 0;
	pEP932C_Registers->HDCP_State = 0;
}

//----------------------------------------------------------------------------------------------------------------------

void ReadInterruptFlags(void) 
{
	//DBG_printf(("EP932 ReadInterruptFlags \r\n"));
	EP932_Reg_Read(EP932_General_Control_2, DDC_Data, 1);

	if(DDC_Data[0] & EP932_General_Control_2__RIF) {
		HDCP_Ext_Ri_Trigger();
		// Clear the interrupt flag
		DDC_Data[0] = EP932_General_Control_2__RIF;
		EP932_Reg_Write(EP932_General_Control_2, DDC_Data, 1);
	}
	/*
	// Clear the interrupt flag
	DDC_Data[0] = EP932_General_Control_2__RIF;
	EP932_Reg_Write(EP932_General_Control_2, DDC_Data, 1);
	*/
}

//----------------------------------------------------------------------------------------------------------------------

void  EP_HDMI_DumpMessage(void)
{
	unsigned short Temp_USHORT;
	unsigned char temp_R[2];
	unsigned char reg_addr;

	// System Status
	DBG_printf(("\r\n\r\n======= Dump EP932E information =======\r\n"));

	DBG_printf(("\r\n[EDID Data]"));
	for(Temp_USHORT = 0; Temp_USHORT < 256; ++Temp_USHORT) {
		if(Temp_USHORT%16 == 0) DBG_printf(("\r\n"));
		if(Temp_USHORT%8 == 0) DBG_printf((" "));
		DBG_printf(("0x%02X,", (int)EP932C_Registers.Readed_EDID[Temp_USHORT] ));
	}
	DBG_printf(("\r\n"));

	DBG_printf(("\r\n[Revision & Configuration]\r\n"));
	DBG_printf(("VendorID=0x%04X, ", EP932C_Registers.VendorID ));
	DBG_printf(("DeviceID=0x%04X, ", EP932C_Registers.DeviceID ));
	DBG_printf(("Version=%d.%d, CFG=0x%02X\r\n", (int)EP932C_Registers.Version_Major, (int)EP932C_Registers.Version_Minor, (int)EP932C_Registers.Configuration ));

	DBG_printf(("\r\n[Interrupt Flags]\r\n"));
	DBG_printf(("EDID_CHG=%d, ", (int)((EP932C_Registers.Interrupt_Flags & EP932E_Interrupt_Flags__EDID_CHG)?1:0) ));
	DBG_printf(("VIDEO_CHG=%d, ", (int)((EP932C_Registers.Interrupt_Flags & EP932E_Interrupt_Flags__VIDEO_CHG)?1:0) ));
	DBG_printf(("AUDIO_CHG=%d\r\n", (int)((EP932C_Registers.Interrupt_Flags & EP932E_Interrupt_Flags__AUDIO_CHG)?1:0) ));

	DBG_printf(("\r\n[System Status]\r\n"));
	DBG_printf(("RSEN=%d, ", (int)((EP932C_Registers.System_Status & EP932E_System_Status__RSEN)?1:0) ));
	DBG_printf(("HTPLG=%d, ", (int)((EP932C_Registers.System_Status & EP932E_System_Status__HTPLG)?1:0) ));
	DBG_printf(("KEY_FAIL=%d, ", (int)((EP932C_Registers.System_Status & EP932E_System_Status__KEY_FAIL)?1:0) ));
	DBG_printf(("DEF_KEY=%d\r\n", (int)((EP932C_Registers.System_Status & EP932E_System_Status__DEF_KEY)?1:0) ));

	DBG_printf(("\r\n[EDID Status]\r\n"));
	DBG_printf(("EDID_HDMI=%d, ", (int)((EP932C_Registers.EDID_Status & EP932E_EDID_Status__HDMI)?1:0) ));
	DBG_printf(("DDC_STATUS=%d\r\n", (int)(EP932C_Registers.EDID_Status & 0x0F) ));
	DBG_printf(("VIDEO_DATA_ADDR=0x%02X, ", (int)EP932C_Registers.EDID_VideoDataAddr ));
	DBG_printf(("AUDIO_DATA_ADDR=0x%02X, ", (int)EP932C_Registers.EDID_AudioDataAddr ));
	DBG_printf(("SPEAKER_DATA_ADDR=0x%02X, ", (int)EP932C_Registers.EDID_SpeakerDataAddr ));
	DBG_printf(("VENDOR_DATA_ADDR=0x%02X\r\n", (int)EP932C_Registers.EDID_VendorDataAddr ));
	DBG_printf(("ASFREQ=0x%02X, ", (int)EP932C_Registers.EDID_ASFreq ));
	DBG_printf(("ACHANNEL=%d\r\n", (int)EP932C_Registers.EDID_AChannel ));

	DBG_printf(("\r\n[Video Status]\r\n"));
	DBG_printf(("Interface=0x%02X, ", (int)EP932C_Registers.Video_Params_Backup.Interface ));
	DBG_printf(("VideoSettingIndex=%d, ", (int)EP932C_Registers.Video_Params_Backup.VideoSettingIndex ));
	DBG_printf(("HVPol=%d, ", (int)EP932C_Registers.Video_Params_Backup.HVPol ));
	DBG_printf(("SyncMode=%d, ", (int)EP932C_Registers.Video_Params_Backup.SyncMode ));
	DBG_printf(("FormatIn=%d, ", (int)EP932C_Registers.Video_Params_Backup.FormatIn ));
	DBG_printf(("FormatOut=%d, ", (int)EP932C_Registers.Video_Params_Backup.FormatOut ));
	DBG_printf(("ColorSpace=%d, ", (int)EP932C_Registers.Video_Params_Backup.ColorSpace ));
	DBG_printf(("AFARate=%d\r\n", (int)EP932C_Registers.Video_Params_Backup.AFARate ));

	DBG_printf(("\r\n[Audio Status]\r\n"));
	DBG_printf(("Interface=0x%02X, ", (int)EP932C_Registers.Audio_Params_Backup.Interface ));
	DBG_printf(("VideoSettingIndex=%d, ", (int)EP932C_Registers.Audio_Params_Backup.VideoSettingIndex ));
	DBG_printf(("ChannelNumber=%d, ", (int)EP932C_Registers.Audio_Params_Backup.ChannelNumber ));
	DBG_printf(("ADSRate=%d, ", (int)EP932C_Registers.Audio_Params_Backup.ADSRate ));
	DBG_printf(("InputFrequency=%d, ", (int)EP932C_Registers.Audio_Params_Backup.InputFrequency ));
	DBG_printf(("VFS=%d, ", (int)EP932C_Registers.Audio_Params_Backup.VFS ));
	DBG_printf(("NoCopyRight=%d\r\n", (int)EP932C_Registers.Audio_Params_Backup.NoCopyRight ));

	DBG_printf(("\r\n[Power Control]\r\n"));
	DBG_printf(("PD_HDMI=%d, ", (int)((EP932C_Registers.Power_Control & EP932E_Power_Control__PD_HDMI)?1:0) ));
	DBG_printf(("PD_TOT=%d\r\n", (int)((EP932C_Registers.Power_Control & EP932E_Power_Control__PD_TOT)?1:0) ));

	DBG_printf(("\r\n[System Configuration]\r\n"));
	DBG_printf(("HDCP_DIS=%d, ", (int)((EP932C_Registers.System_Configuration & EP932E_System_Configuration__HDCP_DIS)?1:0) ));
	DBG_printf(("HDMI_DIS=%d, ", (int)((EP932C_Registers.System_Configuration & EP932E_System_Configuration__HDMI_DIS)?1:0) ));
	DBG_printf(("AUDIO_DIS=%d, ", (int)((EP932C_Registers.System_Configuration & EP932E_System_Configuration__AUDIO_DIS)?1:0) ));
	DBG_printf(("VIDEO_DIS=%d\r\n", (int)((EP932C_Registers.System_Configuration & EP932E_System_Configuration__VIDEO_DIS)?1:0) ));

	DBG_printf(("\r\n[Interrupt Enable]\r\n"));
	DBG_printf(("EDID_CHG=%d, ", (int)((EP932C_Registers.Interrupt_Enable & EP932E_Interrupt_Enable__EDID_CHG)?1:0) ));
	DBG_printf(("VS_PERIOD_CHG=%d, ", (int)((EP932C_Registers.Interrupt_Enable & EP932E_Interrupt_Enable__VIDEO_CHG)?1:0) ));
	DBG_printf(("AS_FREQ_CHG=%d\r\n", (int)((EP932C_Registers.Interrupt_Enable & EP932E_Interrupt_Enable__AUDIO_CHG)?1:0) ));

	DBG_printf(("\r\n[Video Interface 0]\r\n"));
	DBG_printf(("DK=%d, ", (int)((EP932C_Registers.Video_Interface[0] & EP932E_Video_Interface_Setting_0__DK)?1:0) ));
	DBG_printf(("DKEN=%d, ", (int)((EP932C_Registers.Video_Interface[0] & EP932E_Video_Interface_Setting_0__DKEN)?1:0) ));
	DBG_printf(("DSEL=%d, ", (int)((EP932C_Registers.Video_Interface[0] & EP932E_Video_Interface_Setting_0__DSEL)?1:0) ));
	DBG_printf(("BSEL=%d, ", (int)((EP932C_Registers.Video_Interface[0] & EP932E_Video_Interface_Setting_0__BSEL)?1:0) ));
	DBG_printf(("EDGE=%d, ", (int)((EP932C_Registers.Video_Interface[0] & EP932E_Video_Interface_Setting_0__EDGE)?1:0) ));
	DBG_printf(("FMT12=%d\r\n", (int)((EP932C_Registers.Video_Interface[0] & EP932E_Video_Interface_Setting_0__FMT12)?1:0) ));

	DBG_printf(("\r\n[Video Interface 1]\r\n"));
	DBG_printf(("COLOR=%d, ", (int)((EP932C_Registers.Video_Interface[1] & EP932E_Video_Interface_Setting_1__COLOR)>>4) ));
	DBG_printf(("SYNC=%d, ", (int)((EP932C_Registers.Video_Interface[1] & EP932E_Video_Interface_Setting_1__SYNC)>>2) ));
	DBG_printf(("VIN_FMT=%d\r\n", (int)((EP932C_Registers.Video_Interface[1] & EP932E_Video_Interface_Setting_1__VIN_FMT)>>0) ));

	DBG_printf(("\r\n[Audio Interface]\r\n"));
	DBG_printf(("CHANNEL=%d, ", (int) (EP932C_Registers.Audio_Interface & EP932E_Audio_Interface_Setting__CHANNEL)>>4 ));
	DBG_printf(("IIS=%d, ", (int)((EP932C_Registers.Audio_Interface & EP932E_Audio_Interface_Setting__IIS)?1:0) ));
	DBG_printf(("WS_M=%d, ", (int)((EP932C_Registers.Audio_Interface & EP932E_Audio_Interface_Setting__WS_M)?1:0) ));
	DBG_printf(("WS_POL=%d, ", (int)((EP932C_Registers.Audio_Interface & EP932E_Audio_Interface_Setting__WS_POL)?1:0) ));
	DBG_printf(("SCK_POL=%d\r\n", (int)((EP932C_Registers.Audio_Interface & EP932E_Audio_Interface_Setting__SCK_POL)?1:0) ));	

	DBG_printf(("\r\n[Video Input Format 0]\r\n"));
	DBG_printf(("VIC=%d\r\n", (int)EP932C_Registers.Video_Input_Format[0] ));	

	DBG_printf(("\r\n[Video Input Format 1]\r\n"));
	DBG_printf(("AFAR_VIF=0x%02X\r\n", (int)EP932C_Registers.Video_Input_Format[1] ));	


	DBG_printf(("\r\n[EP932 Register value]"));
	for(reg_addr = 0; reg_addr<=0x88; reg_addr++)
	{
		EP932_Reg_Read(reg_addr, temp_R, 1);
		if(reg_addr%8 == 0)DBG_printf(("\r\n"));
		DBG_printf(("[%02X]%02X, ",(int)reg_addr,(int)temp_R[0]));
	}
	DBG_printf(("\r\n"));
}


