#ifndef __IMAPX_KEYBOARD__
#define __IMAPX_KEYBOARD__

#define rKBCON	 					 (0x00 )     //KB Control Register
#define rKBCKD	 					 (0x04 )     //KB Clock Divider Register
#define rKBDCNT	 					 (0x08 )     //Debouncing Filter Counter
#define rKBINT	 					 (0x0C )     //Interrupt Status Register
#define rKBCOLD	 					 (0x10 )     //Key Column Data Register
#define rKBCOEN	 					 (0x14 )     //Column Output Enable Register
#define rKBRPTC	 					 (0x18 )     //repeat scan period Register
#define rKBROWD0	 				 (0x1C )     //Row Data Register 0
#define rKBROWD1	 				 (0x20 )     //Row Data Register 1
#define rKBROWD2	 				 (0x24 )     //Row Data Register 2
#define rKBROWD3	 				 (0x28 )     //Row Data Register 3
#define rKBROWD4	 				 (0x2C )     //Row Data Register 4
  
#define KBCON_MSCAN				(1 << 8)
#define KBCON_RPTEN					(1 << 7)
#define KBCON_SCANST				(1 << 6)
#define KBCON_CLKSEL				(1 << 5)
#define KBCON_FCEN					(1 << 4)
#define KBCON_DFEN					(1 << 3)
#define KBCON_DRDYINTEN			(1 << 2)
#define KBCON_PINTEN				(1 << 1)
#define KBCON_FINTEN				(1 << 0)


#define KBDCNT_DRDYINT				(1 <<16)

#define KBCOEN_COLNUM				(17 << 20)
#define KBCOEN_COLOEN				( 0 )

#endif
