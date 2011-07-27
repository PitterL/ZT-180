/* Reimported by warits on 20100630 */

#ifndef __IMAP_OTG_H__
#define __IMAP_OTG_H__

#define USB_DEV		(0x4000)
#define USB_LK		(0x8000)

/* OTG USB_egisters */
#define USB_ACR       (USB_DEV + 0x00)  //  Application control Register
#define USB_MDAR      (USB_DEV + 0x04)  //  Master Destination Address Register 
#define USB_UDCR      (USB_DEV + 0x08)  //  Usb and Device Control Register
#define USB_FNCR      (USB_DEV + 0x0C)  //  Frame Number Control Register
#define USB_FHHR      (USB_DEV + 0x10)  //  Flush,handshake and halt Bit Register
#define USB_PRIR      (USB_DEV + 0x14)  //  Post Request Information Register
#define USB_STR0      (USB_DEV + 0x18)  //  Setup Transaction0 Register
#define USB_STR1      (USB_DEV + 0x1C)  //  Setup Transaction1 Register
#define USB_BFCR      (USB_DEV + 0x20)  //  Buffer Flush Control Register
#define USB_TBCR0     (USB_DEV + 0x30)  //  TxBuffer0 ControlRegister
#define USB_TBCR1     (USB_DEV + 0x34)  //  TxBuffer1 ControlRegister
#define USB_TBCR2     (USB_DEV + 0x38)  //  TxBuffer2 ControlRegister
#define USB_TBCR3     (USB_DEV + 0x3C)  //  TxBuffer3 ControlRegister
#define USB_IER       (USB_DEV + 0x50)  //  Interrupt Enable Register
#define USB_IDR       (USB_DEV + 0x54)  //  Interrupt Disable Register
#define USB_ISR       (USB_DEV + 0x58)  //  Interrupt Status Register
#define USB_CCR       (USB_DEV + 0x70)  //  Current Configuration Register
#define USB_PIR0      (USB_DEV + 0x74)  //  Physical Interface Register0
#define USB_PIR1      (USB_DEV + 0x78)  //  Physical Interface Register1
#define USB_EDR0      (USB_DEV + 0x80)  //  EndPoint Descriptor Register0
#define USB_EDR1      (USB_DEV + 0x84)  //  EndPoint Descriptor Register1
#define USB_EDR2      (USB_DEV + 0x88)  //  EndPoint Descriptor Register2
#define USB_EDR3      (USB_DEV + 0x8C)  //  EndPoint Descriptor Register3
#define USB_EDR4      (USB_DEV + 0x90)  //  EndPoint Descriptor Register4
#define USB_EDR5      (USB_DEV + 0x94)  //  EndPoint Descriptor Register5
#define USB_EDR6      (USB_DEV + 0x98)  //  EndPoint Descriptor Register6
//#define USB_EDR(x)    (USB_DEV + 0x80 + ((x) << 2))  //  EndPoint Descriptor Register6
#define USB_EDRx      (USB_DEV + 0x80)


/* OTG link USB_egisters */
#define USB_BCWR      (USB_LK + 0x00)  //  OTG Link Write Register
#define USB_BCSR      (USB_LK + 0x10)  //  OTG Link Status Register
#define USB_BCISR     (USB_LK + 0x20)  //  Interrupt Status Register
#define USB_BCIER     (USB_LK + 0x30)  //  Interrupt Enable Register
#define USB_BCIDR     (USB_LK + 0x40)  //  Interrupt Disable Register
#define USB_IPCR      (USB_LK + 0x50)  //  IP Control Register

/* event bits */

#define USB_Connect			(1 << 0)
#define USB_Disconnect		(1 << 1)
#define USB_Reset			(1 << 2)
#define USB_Suspend			(1 << 3)
#define USB_Resume			(1 << 4)
#define USB_SOF				(1 << 5)

#define USB_DMA_Done		(1 << 8)
#define USB_DMA_Error		(1 << 9)
#define USB_Sync_Frame		(1 << 10)
#define USB_SF1				(1 << 11)
#define USB_SF2				(1 << 12)
#define USB_SF3				(1 << 13)
#define USB_Set_Interface	(1 << 14)

//#define USB_EP0				(0xf << 16)
#define USB_EP0_Setup		(1 << 16)
#define USB_EP0_OUT			(1 << 17)
#define USB_EP0_IN			(1 << 18)
#define USB_EP0_Query		(1 << 19)

#define USB_PEP				(0x3f << 24)
#define USB_PEP_Tran(x)		(1 << (23 + (x)))

/* UDCR bits */
#define USB_MTMS_			(1)
#define USB_MTMS_MSK		(0x7)
#define USB_DSI				(1 << 18)

/* FHHR bits */
//#define USB_PEP_HALT(x)		(1 << (16 + (x)))



//ACR 0x0
#define bsACR_PRE_BUFFER_SELECT     0
#define bwACR_PRE_BUFFER_SELECT     1
#define bsACR_AUTO_ACK              1
#define bwACR_AUTO_ACK              1
#define bsACR_FORCE_PHY_SUSPEND     2
#define bwACR_FORCE_PHY_SUSPEND     1
#define bsACR_DMA_ENABLE            4
#define bwACR_DMA_ENABLE            2
#define ACR_DMA_ENABLE_RX 1
#define ACR_DMA_ENABLE_TX 2
#define bsACR_DATA_TOKEN            6
#define bwACR_DATA_TOKEN            2
#define bsACR_TX_BUFFER_SELECT      8
#define bwACR_TX_BUFFER_SELECT      3
#define bsACR_DMA_REQ_ERROR         11
#define bwACR_DMA_REQ_ERROR         1
#define bsACR_DMA_DATA_LEN          16
#define bwACR_DMA_DATA_LEN          11

/* ACR bits */
/*#define USB_IN_Prebuffer	(1 << 0)
#define USB_QueryACK		(1 << 1)
#define USB_DMA_RxEn		(1 << 4)
#define USB_DMA_TxEn		(1 << 5)
#define USB_TxBuffer(x)		((x) << 8)
#define USB_ReqLength(x)	((x) << 16)
#define USB_ReqError		(1 << 11)*/

//UDCR 0x4008
#define UDC_CORE_ENABLE             (1<<0)     
#define UDC_TEST_MODE_SELECT_SHIFT  1
#define UDC_LTE                     (1<<4)
#define UDC_SOFT_CONNECT            (1<<5)
#define UDC_SOFT_DISCONNECT         (1<<6)
#define UDC_SELF_POWERD             (1<<8)
#define UDC_REMOTE_WAKEUP_SUPPORT   (1<<9)
#define UDC_HNP_SUPPORT             (1<<10)
#define UDC_SYNC_FRAME_SUPPORT      (1<<16)
#define UDC_IPG_DISABLE             (1<<17)
#define UDC_DEVICE_SPEED            (1<<18)  //1 in high speed 0 in low speed
#define UDC_SET_TEST_MODE_SHIFT     24
#define UDC_TEST_MODE_ENABLE        (1<<27)
#define UDC_TEST_MODE_STATUS_ENABLE (1<<28)

//FNCR 0x400c
#define FNC_CURRENT_FRAME_NUMBER_SHIFT  0
#define FNC_SYNC_FRAME_NUMBER_SHIFT     16
#define FNC_USB_SUSPEND_ENABLE_SHIFT    27
#define FNC_USB_FULL_SPEED_ONLY_SHIFT   28
#define FNC_USB_FULL_SPEED_RESET_SHIFT  29
#define FNC_USB_HIGH_SPEED_RESET_SHIFT  30

//FHHR 0x4010
#define bsFHHR_HANDSHAKE     0
#define bwFHHR_HANDSHAKE     2
#define FHHR_HANDSHAKE_ACK       1
#define FHHR_HANDSHAKE_STALL     2
#define FHHR_HANDSHAKE_NO_RES    3
#define bsFHHR_EPx_HALT     16

//PRIR 0x4014
#define bsPRIR_BYTE_CNT     0
#define bwPRIR_BYTE_CNT     12
#define bsPRIR_EP           16
#define bwPRIR_EP           8
#define bsPRIR_ISO_TOKEN    24
#define bwPRIR_ISO_TOKEN    2
#define bsPRIR_ISO_HIGH     26
#define bwPRIR_ISO_HIGH     1


//BFCR 0x4020
#define BFCR_FLUSH_ALL      (1<<0)
#define BFCR_FLUSH_RXB      (1<<1)
#define BFCR_FLUSH_TXB      (1<<2)
#define BFCR_FLUSH_TXB0     (1<<4)
#define BFCR_FLUSH_TXB1     (1<<5)
#define BFCR_FLUSH_TXB2     (1<<6)
#define BFCR_FLUSH_TXB3     (1<<7)

//IER 0x4050
//IDR 0x4054
//ISR 0x4058
#define IRQ_CONNECT     (1<<0)
#define IRQ_DISCONNECT  (1<<1)
#define IRQ_RESET       (1<<2)
#define IRQ_SUSPEND     (1<<3)
#define IRQ_RESUME      (1<<4)
#define IRQ_SOF         (1<<5)
#define IRQ_DMA_DONE    (1<<8)
#define IRQ_DMA_ERROR   (1<<9)
#define IRQ_SYNC_FRAME  (1<<10)
#define IRQ_SET_FEATURE_A_HNP_ENABLE   (1<<11)
#define IRQ_SET_FEATURE_A_HNP_SUPPORT  (1<<12)
#define IRQ_SET_FEATURE_A_ALT_HNP_SUFFORT  (1<<13)
#define IRQ_SET_INTERFACE   (1<<14)
#define IRQ_CONTROL_SETUP   (1<<16)
#define IRQ_CONTROL_OUT     (1<<17)
#define IRQ_CONTROL_IN      (1<<18)
#define IRQ_CONTROL_QUERY   (1<<19)
#define IRQ_EPx_SHIFT       24
#define IRQ_EP1             (1<<24)
#define IRQ_EP2             (1<<25)
#define IRQ_EP3             (1<<26)
#define IRQ_EP4             (1<<27)
#define IRQ_EP5             (1<<28)
#define IRQ_EP6             (1<<29)

//CCR   0x4070
#define bsCCR_ADDRESS   0
#define bwCCR_ADDRESS   7
#define bsCCR_CONFIGURE 8
#define bwCCR_CONFIGURE 4

//PIR0  0x4074
#define PIR_INTERFACE0_ACTIVE_ALT_SHIFT 0
#define PIR_INTERFACE0_NUMBER_SHIFT 4
#define PIR_CONFIGURE0_NUMBER_SHIFT 8
#define PIR_INTERFACE0_MAXALT_SHIFT 12
#define PIR_INTERFACE1_ACTIVE_ALT_SHIFT 16
#define PIR_INTERFACE1_NUMBER_SHIFT 20
#define PIR_CONFIGURE1_NUMBER_SHIFT 24
#define PIR_INTERFACE1_MAXALT_SHIFT 28

//PIR1  0x4078
#define PIR_INTERFACE2_ACTIVE_ALT_SHIFT 0
#define PIR_INTERFACE2_NUMBER_SHIFT 4
#define PIR_CONFIGURE2_NUMBER_SHIFT 8
#define PIR_INTERFACE2_MAXALT_SHIFT 12
#define PIR_INTERFACE3_ACTIVE_ALT_SHIFT 16
#define PIR_INTERFACE3_NUMBER_SHIFT 20
#define PIR_CONFIGURE3_NUMBER_SHIFT 24
#define PIR_INTERFACE3_MAXALT_SHIFT 28


//EDR 0~6  0x4080~0x4098
#define bsEDR_TYPE  0
#define bwEDR_TYPE  3
#define EDR_TYPE_CONTROL        0
#define EDR_TYPE_BULK           1
#define EDR_TYPE_INTERRUPT      2
#define EDR_TYPE_ISO            4
#define EDR_TYPE_ISO_1          5
#define EDR_TYPE_ISO_2          6
#define EDR_TYPE_ISO_3          7
#define bsEDR_DIR   3
#define bwEDR_DIR   1
#define EDR_DIR_OUT             0
#define EDR_DIR_IN              1
#define bsEDR_NO   4
#define bwEDR_NO   4
#define bsEDR_ALT_INTERFACE 8
#define bwEDR_ALT_INTERFACE 4
#define bsEDR_INTERFACE 12
#define bwEDR_INTERFACE 4
#define bsEDR_MAX_PKT 16
#define bwEDR_MAX_PKT 11
#define bsEDR_BUFFER_SELECT 28
#define bwEDR_BUFFER_SELECT 3
#define bsEDR_HALT_STS 31
#define bwEDR_HALT_STS 1

/* EDR bits */
/*
#define USB_EP_Type(x)		((x) << 0)
#define USB_EP_DirIn		(1 << 3)
#define USB_LogicNo(x)		((x) << 4)
#define USB_EP_Alt(x)		((x) << 8)
#define USB_EP_Phy(x)		((x) << 12)
#define USB_MaxPacket(x)	((x) << 16)
#define USB_InBuf(x)		((x) << 28)
#define USB_EP_HALT			(1 << 31)*/

/* BCSR */
#define BCSR_B_STATUS_SHIFT 4

#define USB_B_Valid			(1 << 13)
#define USB_DRDB_CS			(3 << 4)

/* BCWR */
#define USB_B_Connect		(1 << 11)

#endif /* __IMAP_OTG_H__ */

