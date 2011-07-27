#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <mach/hardware.h>

#include <asm/irq.h>
#include <mach/imap_addr.h>
#include <plat/pm.h>
//#include <plat/regs-serial.h>
#include <mach/imapx_uart.h>
#include <mach/imapx_gpio.h>
/**********************************************************/
 #define UARTRBR_LCR_DLAB				7			
#define UARTRBR_LCR_Break				6			
#define UARTRBR_LCR_StickParity			5			
#define UARTRBR_LCR_EPS				4			
#define UARTRBR_LCR_PEN				3			
#define UARTRBR_LCR_STOP				2			
#define UARTRBR_LCR_DLS_CLS			0
#define UARTRBR_IER_PTIME				7
#define UARTRBR_IER_EDSSI				3
#define UARTRBR_IER_ELSI				2
#define UARTRBR_IER_ETBEI				1
#define UARTRBR_IER_ERBFI				0
#define UARTRBR_IIR_MODEM				0				// modem status
#define UARTRBR_IIR_NOPENDING			1				// no interrupt pending
#define UARTRBR_IIR_THR_RDY			2				// THR empty
#define UARTRBR_IIR_RDATA_OK			4				// received data available
#define UARTRBR_IIR_RLINE_OK			6				// receiver line status
#define UARTRBR_IIR_BUSY_DETECT		7				// busy detect
#define UARTRBR_IIR_CHAR_TIMEOUT		12				// character timeout
#define rUARTRBR_IIR_RXFULL_TRI		6				// RCVR Trigger
#define rUARTRBR_IIR_TXEMPTY_TRI		4				// TX Empty Trigger.
#define rUARTRBR_IIR_DMA_MODE			3				// DMA Mode
#define rUARTRBR_IIR_TFIFO_RST			2				// XMIT FIFO Reset
#define rUARTRBR_IIR_RFIFO_RST			1				// RCVR FIFO Reset
#define rUARTRBR_IIR_FIFO_EN			0				// FIFO Enable
#define rUARTRBR_IIR_IID				0
#define UARTRBR_LSR_RFE				7
#define UARTRBR_LSR_TEMT				6
#define UARTRBR_LSR_THRE				5
#define UARTRBR_LSR_BI					4
#define UARTRBR_LSR_FE					3
#define UARTRBR_LSR_PE					2
#define UARTRBR_LSR_OE					1
#define UARTRBR_LSR_DR					0
#define UARTRBR_CLKSEL_COM				0
#define rUARTRBR_CLKSEL_PCLK			0		//PCLK clok
#define rUARTRBR_CLKSEL_FCLK			1		//FPGA doesn't work
#define rUARTRBR_CLKSEL_EXCLK			2		//external clok
#define EXT_FREQ 			66000000
#define UART_CLK_SEL		rUARTRBR_CLKSEL_PCLK		
 /**********************************************************/

/* for external use */
#define IMAP_PMDBG(fmt...) printk(KERN_DEBUG fmt)
unsigned long imapx200_pm_flags;

/* Debug code:
 *
 * This code supports debug output to the low level UARTs for use on
 * resume before the console layer is available.
*/

#define imapx200_pm_debug_init() do { } while(0)
/* Save the UART configurations if we are configured for debug. */

unsigned char pm_uart_udivslot;
static void imapx200_pm_arch_prepare_irqs(void)
{
	//__raw_writel(__raw_readl(S3C64XX_EINT0PEND), S3C64XX_EINT0PEND);
}
static void imapx200_pm_arch_stop_clocks(void)
{
}
static void imapx200_pm_arch_show_resume_irqs(void)
{
//	while(1);
}



static void imapx200_pm_save_uarts(void) { }
static void imapx200_pm_restore_uarts(void) { 
	int v0, v1, v2;
	int bdrate = 115200;
	int rUARTRBR1_LCR;
	int rUARTRBR1_THR_DLL_IMAP;
	int rUARTRBR1_IER_DLH;
	int rUARTRBR1_IIR_FCR;
	int rUARTRBR_CLKSEL;
	__raw_writel(((__raw_readl(rGPACON) &(~0xf)) | 0xa), rGPACON);
	__raw_writel(((__raw_readl(rGPHCON) &(~0xf0)) | 0x50), rGPHCON);
	__raw_writel(((__raw_readl(rGPBDAT) &(~0xf)) | 0x9), rGPBDAT);

	rUARTRBR_CLKSEL = __raw_readl(rUART0_CLKSEL);
	rUARTRBR_CLKSEL &= ~((0x3 << UARTRBR_CLKSEL_COM));
	rUARTRBR_CLKSEL |= ((rUARTRBR_CLKSEL_PCLK << UARTRBR_CLKSEL_COM));
    	__raw_writel(rUARTRBR_CLKSEL, rUART0_CLKSEL);

	v1 = EXT_FREQ / (16 * bdrate);
	v2 = EXT_FREQ % (16 * bdrate);

	if (v2 >= ((16 * bdrate) / 2))
		v0 = (v1 + 1);
	else
		v0 = v1;
	
	//disable interrupt
//	rUARTRBR1_IER_DLH = __raw_readl(rUART0_IER_DLH);
//	rUARTRBR1_IER_DLH &= ~(1 << UARTRBR_IER_EDSSI)
//						&~(1 << UARTRBR_IER_ELSI)
//						&~(1 << UARTRBR_IER_ETBEI)
//						&~(1 << UARTRBR_IER_ERBFI);
//	__raw_writel(rUARTRBR1_IER_DLH, rUART0_IER_DLH);

	//disable FIFO
//	rUARTRBR1_IIR_FCR = 0;
//	rUARTRBR1_IIR_FCR &= ~(1 << rUARTRBR_IIR_FIFO_EN);
//	__raw_writel(rUARTRBR1_IIR_FCR, rUART0_IIR_FCR);

	//config baundrate
	rUARTRBR1_LCR = __raw_readl(rUART0_LCR);
	rUARTRBR1_LCR |= (1 << UARTRBR_LCR_DLAB);
	__raw_writel(rUARTRBR1_LCR, rUART0_LCR);	//DLAB=1
    	
	rUARTRBR1_THR_DLL_IMAP = v0 & 0x000000FF;
	__raw_writel(rUARTRBR1_THR_DLL_IMAP, rUARTRBR0_THR_DLL);
	rUARTRBR1_IER_DLH = (v0 & 0x0000FF00) >> 8;
	__raw_writel(rUARTRBR1_IER_DLH, rUART0_IER_DLH);
		
	rUARTRBR1_LCR &= ~(1 << UARTRBR_LCR_DLAB);
	__raw_writel(rUARTRBR1_LCR, rUART0_LCR);	//DLAB=0


	//DATA-Bit
	rUARTRBR1_LCR &= ~(3 << UARTRBR_LCR_DLS_CLS);
	rUARTRBR1_LCR |= (3 << UARTRBR_LCR_DLS_CLS);
	//STOP-Bit
	rUARTRBR1_LCR &= ~(1 << UARTRBR_LCR_STOP);
	//PEN-Bit
	rUARTRBR1_LCR &= ~(1 << UARTRBR_LCR_PEN);		
	//PARITY-Bit
	rUARTRBR1_LCR &= ~(1 << UARTRBR_LCR_EPS);
	__raw_writel(rUARTRBR1_LCR, rUART0_LCR);
	
}

/* The IRQ ext-int code goes here, it is too small to currently bother
 * with its own file. */

unsigned long imapx200_irqwake_intmask	= 0xffffffffL;
unsigned long imapx200_irqwake_eintmask	= 0xffffffffL;

int imapx200_irqext_wake(unsigned int irqno, unsigned int state)
{
		return 0;
}

/* helper functions to save and restore register state */

/**
 * imapx200_pm_do_save() - save a set of registers for restoration on resume.
 * @ptr: Pointer to an array of registers.
 * @count: Size of the ptr array.
 *
 * Run through the list of registers given, saving their contents in the
 * array for later restoration when we wakeup.
 */
void imapx200_pm_do_save(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
		ptr->val = __raw_readl(ptr->reg);
		IMAP_PMDBG("saved %p value %08lx\n", ptr->reg, ptr->val);
	}
}

/**
 * imapx200_pm_do_restore() - restore register values from the save list.
 * @ptr: Pointer to an array of registers.
 * @count: Size of the ptr array.
 *
 * Restore the register values saved from imapx200_pm_do_save().
 *
 * Note, we do not use IMAP_PMDBG() in here, as the system may not have
 * restore the UARTs state yet
*/

void imapx200_pm_do_restore(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
	//	IMAP_PMDBG("restore %p (restore %08lx, was %08x)\n",
	//	       ptr->reg, ptr->val, __raw_readl(ptr->reg));

		__raw_writel(ptr->val, ptr->reg);
	}
}

/**
 * imapx200_pm_do_restore_core() - early restore register values from save list.
 *
 * This is similar to imapx200_pm_do_restore() except we try and minimise the
 * side effects of the function in case registers that hardware might need
 * to work has been restored.
 *
 * WARNING: Do not put any debug in here that may effect memory or use
 * peripherals, as things may be changing!
*/

void imapx200_pm_do_restore_core(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++)
		__raw_writel(ptr->val, ptr->reg);
}

/* imapx2002410_pm_show_resume_irqs
 *
 * print any IRQs asserted at resume time (ie, we woke from)
*/
static void imapx200_pm_show_resume_irqs(int start, unsigned long which,
				    unsigned long mask)
{
	int i;

	which &= ~mask;

	for (i = 0; i <= 31; i++) {
		if (which & (1L<<i)) {
			IMAP_PMDBG("IRQ %d asserted at resume\n", start+i);
		}
	}
}


void (*pm_cpu_prep)(void);
void (*pm_cpu_sleep)(void);

#define any_allowed(mask, allow) (((mask) & (allow)) != (allow))

/* imapx200_pm_enter
 *
 * central control for sleep/resume process
*/

static int imapx200_pm_enter(suspend_state_t state)
{
	static unsigned long regs_save[16];

	/* ensure the debug is initialised (if enabled) */

	imapx200_pm_debug_init();

	IMAP_PMDBG("%s(%d)\n", __func__, state);

	if (pm_cpu_prep == NULL || pm_cpu_sleep == NULL) {
		printk(KERN_ERR "%s: error: no cpu sleep function\n", __func__);
		return -EINVAL;
	}

	/* check if we have anything to wake-up with... bad things seem
	 * to happen if you suspend with no wakeup (system will often
	 * require a full power-cycle)
	*/
#if 0
	if (!any_allowed(imapx200_irqwake_intmask, imapx200_irqwake_intallow) &&
	    !any_allowed(imapx200_irqwake_eintmask, imapx200_irqwake_eintallow)) {
		printk(KERN_ERR "%s: No wake-up sources!\n", __func__);
		printk(KERN_ERR "%s: Aborting sleep\n", __func__);
		return -EINVAL;
	}
#endif
	/* store the physical address of the register recovery block */

	imapx200_sleep_save_phys = virt_to_phys(regs_save);

	IMAP_PMDBG("imapx200_sleep_save_phys=0x%08lx\n", imapx200_sleep_save_phys);

	printk(KERN_INFO "imapx200_sleep_save_phys=0x%08lx\n", imapx200_sleep_save_phys);
	/* save all necessary core registers not covered by the drivers */
//	imapx200_pm_save_gpios();//finish-xmj
	imapx200_pm_save_uarts();
	imapx200_pm_save_core();

	/* set the irq configuration for wake */

	imapx200_pm_configure_extint();

	IMAP_PMDBG("sleep: irq wakeup masks: %08lx,%08lx\n",
	    imapx200_irqwake_intmask, imapx200_irqwake_eintmask);

	imapx200_pm_arch_prepare_irqs();

	/* call cpu specific preparation */

	pm_cpu_prep();

	/* flush cache back to ram */

	flush_cache_all();

	imapx200_pm_check_store();

	/* send the cpu to sleep... */

	imapx200_pm_arch_stop_clocks();

	/* imapx200_cpu_save will also act as our return point from when
	 * we resume as it saves its own register state and restores it
	 * during the resume.  */

	imapx200_cpu_save(regs_save);

	/* restore the cpu state using the kernel's cpu init code. */

	cpu_init();
	/* restore the system state */
	__raw_writel(0xff,rPOW_STB);
	udelay(300);
	while(__raw_readl(rPOW_STB) != 0x0){
	__raw_writel(0xff,rPOW_STB);
	udelay(300);

	}
	__raw_writel(0x40000000,rSRCPND);
	__raw_writel(0x40000000,rINTPND);

	imapx200_pm_restore_core();
	imapx200_pm_restore_uarts();
	return 0;
}

/* callback from assembly code */
void imapx200_pm_cb_flushcache(void)
{
	flush_cache_all();
}

static int imapx200_pm_prepare(void)
{
	/* prepare check area if configured */

	imapx200_pm_check_prepare();
	return 0;
}

static void imapx200_pm_finish(void)
{
	imapx200_pm_check_cleanup();
}

static struct platform_suspend_ops imapx200_pm_ops = {
	.enter		= imapx200_pm_enter,
	.prepare	= imapx200_pm_prepare,
	.finish		= imapx200_pm_finish,
	.valid		= suspend_valid_only_mem,
};

/* imapx200_pm_init
 *
 * Attach the power management functions. This should be called
 * from the board specific initialisation if the board supports
 * it.
*/

int __init imap_pm_init(void)
{
	printk("IMAP Power Management, Copyright 2010 Infotm\n");

	suspend_set_ops(&imapx200_pm_ops);
	return 0;
}
