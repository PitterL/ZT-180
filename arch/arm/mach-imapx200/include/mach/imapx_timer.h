#ifndef __IMAPX_TIMER__
#define __IMAPX_TIMER__

#define rTimer0LoadCount      (IMAP_VA_TIMER+0x00 )     //timer0 Load Count register
#define rTimer1LoadCount      (IMAP_VA_TIMER+0x14 )     //timer1 Load Count register
#define rTimer0CurrentValue   (IMAP_VA_TIMER+0x04 )     //timer0 Current Value register
#define rTimer1CurrentValue   (IMAP_VA_TIMER+0x18 )     //timer1 Current Value register
#define rTimer0ControlReg     (IMAP_VA_TIMER+0x08 )     //timer0 Control register
#define rTimer1ControlReg     (IMAP_VA_TIMER+0x1C )     //timer1 Control register
#define rTimer0EOI            (IMAP_VA_TIMER+0x0C )     //timer0 End-of-Interrupt register
#define rTimer1EOI            (IMAP_VA_TIMER+0x20 )     //timer1 End-of-Interrupt register
#define rTimer0IntStatus      (IMAP_VA_TIMER+0x10 )     //timer0 Interrupt Status register
#define rTimer1IntStatus      (IMAP_VA_TIMER+0x24 )     //timer1 Interrupt Status register
#define rTimersIntStatus      (IMAP_VA_TIMER+0xA0 )     //Timers Interrupt Status register
#define rTimersEOI            (IMAP_VA_TIMER+0xA4 )     //Timers End-of-Interrupt register
#define rTimersRawIntStatus   (IMAP_VA_TIMER+0xA8 )     //Timers Raw Interrupt Status register
#define rTIMERS_COMP_VERSION  (IMAP_VA_TIMER+0xAC )     //Timers Component Version

#define IMAP_TCR_TIMER_EN          (1<<0)
#define IMAP_TCR_TIMER_MD          (1<<1)
#define IMAP_TCR_TIMER_INTMASK     (1<<2)

#endif
