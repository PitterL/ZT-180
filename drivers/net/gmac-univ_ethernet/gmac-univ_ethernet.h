#ifndef __GMAC_H__
#define __GMAC_H__

/* select any TX/RX mode as you like */
#define GMAC_POLL			0 
#define GMAC_NORM_IRQ			0
#define GMAC_TASK_IRQ			1
/* TWOTXFRAMES mode cannot be used yet */
#define TWOTXFRAMES			0

#define RANDOMMAC			0

#define DEBUG_LEVEL			0

/*
 *  ETHERNET DMA REGISTER PARAM
 */
//BUS MODE
#define AAL			(0x1<<25)
#define FBPL		(0x1<<24)
#define USP			(0x1<<23)
#define RXPBL		(0x8<<17)
#define FB			(0x1<<16)
#define PR			(0x0<<14)
#define PBL			(0x8<<8)
#define DSL			(0x4<<2)
#define DA			(0x1<<1)
#define SWR			(0x1<<0)

//Operation mode register
#define DT			(0x1<<26)
#define RSF			(0x1<<25)
#define DFF			(0x1<<24)
#define TSF			(0x1<<21)
#define FTF			(0x1<<20)
#define TTC			(0x1<<14)
#define ST			(0x1<<13)
#define RFD			(0x1<<11)
#define RFA			(0x3<<9)
#define EFC			(0x1<<8)
#define FEF			(0x1<<7)
#define FUF			(0x1<<6)
#define RTC			(0x3<<3)
#define OSF			(0x1<<2)
#define SR			(0x1<<1)

//DMA status register
#define TTI			(0x1<<29)
#define GPI			(0x1<<28)
#define GMI			(0x1<<27)
#define GLI			(0x1<<26)
#define TSSTOP		(0x0<<20)
#define TSFET		(0x1<<20)
#define TSWAIT		(0x2<<20)
#define TSTRAN		(0x3<<20)
#define TSSPEND		(0x6<<20)
#define TSCLOSE		(0x7<<20)
#define RSSTOP		(0x0<<17)
#define RSFET		(0x1<<17)
#define RSWAIT		(0x3<<17)
#define RSSPEND		(0x4<<17)
#define RSCLOSE		(0x5<<17)
#define RSTRAN		(0x7<<17)
#define NORINTE		(0x1<<16)
#define ABNORINTE	(0x1<<15)
#define ERI			(0x1<<14)
#define FBI			(0x1<<13)
#define ETI			(0x1<<10)
#define RWT			(0x1<<9)
#define RPS			(0x1<<8)
#define RU			(0x1<<7)
#define RI			(0x1<<6)
#define UNF			(0x1<<5)
#define OVF			(0x1<<4)
#define TJT			(0x1<<3)
#define TU			(0x1<<2)
#define TPS			(0x1<<1)
#define TI			(0x1<<0)

//DMA interrupt enable register
#define NIE			(0x1<<16)
#define AIE			(0x1<<15)
#define ERE			(0x1<<14)
#define FBE			(0x1<<13)
#define ETE			(0x1<<10)
#define RWE			(0x1<<9)
#define RSE			(0x1<<8)
#define RUE			(0x1<<7)
#define RIE			(0x1<<6)
#define UNE			(0x1<<5)
#define OVE			(0x1<<4)
#define TJE			(0x1<<3)
#define TUE			(0x1<<2)
#define TSE			(0x1<<1)
#define TIE			(0x1<<0)

/*
 * ETHERNET MAC REGISTER PARAM
 */

//MAC configuration register
#define TC			(0x1<<24)
#define WD			(0x1<<23)
#define JD			(0x0<<22)
#define BE			(0x1<<21)
#define JE			(0x1<<20)
//#define IFG		(0x4<<17)  //100: 64bit times
#define IFG			(0x7<<17)  //111: 40bit times
#define DCRS		(0x0<<16)
#define PS			(0x1<<15) //MII (10/100 Mbps)
#define FES			(0x1<<14)
#define DO			(0x0<<13)
#define LM			(0x0<<12) //not use loopback mode
#define DM			(0x1<<11)
#define IPC			(0x0<<10)
#define DR			(0x0<<9)
#define LUD			(0x0<<8)
#define ACS			(0x1<<7)
#define BL			(0x1<<5)
#define DC			(0x0<<4)
#define TE			(0x1<<3)
#define RE			(0x1<<2)

//MAC Frame filter
#define RA			(0x1<<31)
#define HPF			(0x1<<10)
#define SAF			(0x1<<9)
#define SAIF		(0x1<<8)
#define PCF			(0x3<<6)

#define DBF			(0x1<<5)
#define PM			(0x1<<4)
#define DAIF		(0x1<<3)
#define HMC			(0x1<<2)
#define HUC			(0x1<<1)
#define PRM			(0x1<<0)

//FLOW Control register
#define PT			(0x200<<16)
#define TFE			(0x1<<2)
#define	RFE			(0x1<<1)

#define DELAY		10000
#define PHYBUSY		0x1

#define SIZE_OF_REG_SPA		(0x2000)

#define GMAC_MULTICAST_LIST	64
#if TWOTXFRAMES
#define MAXTXFRAMES		2
#else
#define MAXTXFRAMES		1
#endif

#define NO_OF_TXDES			5
#define NO_OF_RXDES			100

#define MIN_SIZE			(0x40)
#define MAX_MTU				(0x600)

#define SINGLE_TXDES_SIZE	(16)
#define SINGLE_RXDES_SIZE	(16)

#define SINGLE_TXBUF_SIZE	MAX_MTU	
#define SINGLE_RXBUF_SIZE	MAX_MTU

#define SIZE_OF_TXDES		(SINGLE_TXDES_SIZE*NO_OF_TXDES)
#define SIZE_OF_TXBUF		(SINGLE_TXBUF_SIZE*NO_OF_TXDES)
#define SIZE_OF_RXDES		(SINGLE_RXDES_SIZE*NO_OF_RXDES)
#define SIZE_OF_RXBUF		(SINGLE_RXBUF_SIZE*NO_OF_RXDES)

//TXDES0
#define TXOWN				(0x1<<31)
#define TXES				(0x1<<15)
#define TXJT 				(0x1<<14)
#define TXFF				(0x1<<13)
//#define TXLC				(0x1<<11)
#define TXNC				(0x1<<10)
#define TXLC				(0x1<<9)
#define TXEC				(0x1<<8)
#define TXED				(0x1<<2)
#define TXUE				(0x1<<1)

//TXDES1
#define IC					(0x1<<31)
#define TXLS				(0x1<<30)
#define FS					(0x1<<29)
#define CIC					(0x3<<27) 
#define DC_T				(0x0<<26)
#define TER					(0x1<<25)
#define TCH					(0x1<<24) //not second address but next descriptor address
#define DP					(0x0<<23)
#define TTSE				(0x1<<22)
#define TBS2				((SINGLE_TXBUF_SIZE&0x7ff)<<11)
#define TBS1				((SINGLE_TXBUF_SIZE&0x7ff)<<0)

//RXDES0
#define RXOWN				(0x1<<31)
#define RXLS				(0x1<<8)
#define RXFS				(0x1<<9)
#define RXES				(0x1<<15)
#define RXDE				(0x1<<14)
#define RXOE				(0x1<<11)
#define RXGF				(0x1<<7)
#define RXLC				(0x1<<6)
#define RXWT				(0x1<<4)
#define RXRE				(0x1<<3)
#define RXCRCE				(0x1<<1)

//RXDES1
#define DINT				(0x0<<31)
#define RER					(0x1<<25)
#define RCH					(0x1<<24)
#define RBS2				((SINGLE_RXBUF_SIZE&0x7ff)<<11)
#define RBS1				((SINGLE_RXBUF_SIZE&0x7ff)<<0)

#define NORMAL_SPEED		0x0
#define FAST_SPEED			0x1

#endif  /* __GMAC_H__ */


