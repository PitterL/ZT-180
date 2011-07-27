#ifndef __IMAPX_PWM__
#define __IMAPX_PWM__

#define IMAP_TCFG0   (0x0)	//Timer 0 configuration
#define IMAP_TCFG1   (0x4 )	//Timer 1 configuration
#define IMAP_TCON    (0x8 )	//Timer control
#define IMAP_TCNTB0  (0xc )	//Timer count buffer 0
#define IMAP_TCMPB0  (0x10 )	//Timer compare buffer 0
#define IMAP_TCNTO0  (0x14 )	//Timer count observation 0
#define IMAP_TCNTB1  (0x18 )	//Timer count buffer 1
#define IMAP_TCMPB1  (0x1c )	//Timer compare buffer 1
#define IMAP_TCNTO1  (0x20 )	//Timer count observation 1
#define IMAP_TCNTB2  (0x24 )	//Timer count buffer 2
#define IMAP_TCMPB2  (0x28 )	//Timer compare buffer 2
#define IMAP_TCNTO2  (0x2c )	//Timer count observation 2
#define IMAP_TCNTB3  (0x30 )	//Timer count buffer 3
#define IMAP_TCMPB3  (0x34 )	//Timer compare buffer 3
#define IMAP_TCNTO3  (0x38 )	//Timer count observation 3
#define IMAP_TCNTB4  (0x3c )	//Timer count buffer 4
#define IMAP_TCNTO4  (0x40 )	//Timer count observation 4

/*		IMAP_TCFG0              */
/*		imap_TCFG1		*/ 
#define		IMAP_TCFG1_DMA		(0x1111<<20)

/*        	IMAP_TCON		*/
#define		IMAP_TCON_T4RL_ON	(1<<22)
#define		IMAP_TCON_T4MU_ON	(1<<21)
#define		IMAP_TCON_T4START	(1<<20)
#define		IMAP_TCON_T3RL_ON	(1<<19)
#define		IMAP_TCON_T3OI_ON	(1<<18)
#define		IMAP_TCON_T3MU_ON	(1<<17)
#define		IMAP_TCON_T3START	(1<<16)
#define		IMAP_TCON_T2RL_ON	(1<<15)
#define         IMAP_TCON_T2OI_ON       (1<<14)
#define		IMAP_TCON_T2MU_ON	(1<<13)
#define		IMAP_TCON_T2START	(1<<12)
#define		IMAP_TCON_T1RL_ON	(1<<11)
#define         IMAP_TCON_T1OI_ON       (1<<10)
#define		IMAP_TCON_T1MU_ON	(1<<9)
#define		IMAP_TCON_T1START	(1<<8)
#define		IMAP_TCON_DEADZ_ON	(1<<4)
#define		IMAP_TCON_T0RL_ON	(1<<3)
#define         IMAP_TCON_T0OI_ON       (1<<2)
#define		IMAP_TCON_T0MU_ON	(1<<1)
#define		IMAP_TCON_T0START	(1<<0)

#define		IMAP_TCNTB(channel)      (IMAP_TCNTB0 + (channel *0xC))
#define		IMAP_TCMPB(channel)	  (IMAP_TCMPB0 + (channel *0xC))   



#endif
