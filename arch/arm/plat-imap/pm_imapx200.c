#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <plat/pm.h>
#include <mach/imapx_gpio.h>
#include <mach/imapx_intr.h>
#include <mach/imapx_sysmgr.h>

static struct sleep_save core_save[] = {
	SAVE_ITEM(rPLL_LOCKED),
	SAVE_ITEM(rPLL_OCLKSEL),
	SAVE_ITEM(rPLL_CLKSEL),
	SAVE_ITEM(rAPLL_CFG),
	SAVE_ITEM(rDPLL_CFG),
	SAVE_ITEM(rEPLL_CFG),
	SAVE_ITEM(rDIV_CFG0),
	SAVE_ITEM(rDIV_CFG1),
	SAVE_ITEM(rDIV_CFG2),
	SAVE_ITEM(rHCLK_MASK),
	SAVE_ITEM(rPCLK_MASK),
	SAVE_ITEM(rSCLK_MASK),
	SAVE_ITEM(rCLKOUT0_CFG),

	SAVE_ITEM(rCLKOUT1_CFG),
	SAVE_ITEM(rCPUSYNC_CFG),

	SAVE_ITEM(rDIV_CFG2),
	SAVE_ITEM(rUSB_SRST),
	SAVE_ITEM(rPERSIM_CFG),
	SAVE_ITEM(rPAD_CFG),
	SAVE_ITEM(rGPU_CFG),
	SAVE_ITEM(rDIV_CFG3),
	SAVE_ITEM(rDIV_CFG4),

	SAVE_ITEM(rINTMOD),
	SAVE_ITEM(rINTMSK),
	SAVE_ITEM(rPRIORITY),
	SAVE_ITEM(rINTMOD2),
	SAVE_ITEM(rINTMSK2),
	SAVE_ITEM(rPRIORITY2),
//may be here is not complete saved register
#ifndef CONFIG_CPU_FREQ
//	SAVE_ITEM(S3C_APLL_CON),
//	SAVE_ITEM(S3C_MPLL_CON),
#endif
};

static struct sleep_save misc_save[] = {
	//save the gpio register
	SAVE_ITEM(rGPADAT),
	SAVE_ITEM(rGPACON),
	SAVE_ITEM(rGPBDAT),
	SAVE_ITEM(rGPBCON),
	SAVE_ITEM(rGPCDAT),
	SAVE_ITEM(rGPCCON),
	SAVE_ITEM(rGPDDAT),
	SAVE_ITEM(rGPDCON),
	SAVE_ITEM(rGPEDAT),
	SAVE_ITEM(rGPECON),
	SAVE_ITEM(rGPFDAT),
	SAVE_ITEM(rGPFCON),
	SAVE_ITEM(rGPGDAT),
	SAVE_ITEM(rGPGCON),
	SAVE_ITEM(rGPHDAT),
	SAVE_ITEM(rGPHCON),
	SAVE_ITEM(rGPIDAT),
	SAVE_ITEM(rGPICON),
	SAVE_ITEM(rGPJDAT),
	SAVE_ITEM(rGPJCON),
	SAVE_ITEM(rGPKDAT),
	SAVE_ITEM(rGPKCON),
	SAVE_ITEM(rGPLDAT),
	SAVE_ITEM(rGPLCON),
	SAVE_ITEM(rGPMDAT),
	SAVE_ITEM(rGPMCON),
	SAVE_ITEM(rGPNDAT),
	SAVE_ITEM(rGPNCON),
	SAVE_ITEM(rGPODAT),
	SAVE_ITEM(rGPOCON),
	SAVE_ITEM(rGPPDAT),
	SAVE_ITEM(rGPPCON),
	SAVE_ITEM(rGPQDAT),
	SAVE_ITEM(rGPQCON),

	SAVE_ITEM(rEINTCON),
	SAVE_ITEM(rEINTFLTCON0),
	SAVE_ITEM(rEINTFLTCON1),
	SAVE_ITEM(rEINTGCON),
	SAVE_ITEM(rEINTGFLTCON0),
	SAVE_ITEM(rEINTGFLTCON1),

	SAVE_ITEM(rEINTG1MASK),
	SAVE_ITEM(rEINTG2MASK),
	SAVE_ITEM(rEINTG3MASK),
	SAVE_ITEM(rEINTG4MASK),
	SAVE_ITEM(rEINTG5MASK),
	SAVE_ITEM(rEINTG6MASK),

	SAVE_ITEM(rEINTG1PEND),
	SAVE_ITEM(rEINTG2PEND),
	SAVE_ITEM(rEINTG3PEND),
	SAVE_ITEM(rEINTG4PEND),
	SAVE_ITEM(rEINTG5PEND),
	SAVE_ITEM(rEINTG6PEND),

};

void imapx200_pm_configure_extint(void)
{
	/*************************************
	__raw_writel(imapx200_irqwake_eintmask, imapx200_EINT_MASK);
	*************************************/
}

void imapx200_pm_restore_core(void)
{
	unsigned long tmp;
	tmp = 0xffffffff;
	imapx200_pm_do_restore(misc_save, ARRAY_SIZE(misc_save));
	__raw_writel(tmp, rSRCPND);
	__raw_writel(tmp, rSRCPND2);
	__raw_writel(tmp, rINTPND);
	__raw_writel(tmp, rINTPND2);
	imapx200_pm_do_restore_core(core_save, ARRAY_SIZE(core_save));
}

void imapx200_pm_save_core(void)
{
	imapx200_pm_do_save(core_save, ARRAY_SIZE(core_save));
	imapx200_pm_do_save(misc_save, ARRAY_SIZE(misc_save));
}

void imapx200_cpu_suspend(void)
{
	unsigned long tmp;

	/* issue the standby signal into the pm unit. Note, we
	 * issue a write-buffer drain just in case */
	tmp = 0xffffffff;
	__raw_writel(tmp, rINTMSK);
	__raw_writel(tmp, rINTMSK2);
	__raw_writel(tmp, rSRCPND);
	__raw_writel(tmp, rSRCPND2);
	__raw_writel(tmp, rINTPND);
	__raw_writel(tmp, rINTPND2);
	tmp = 0;
	__raw_writel(0x0, rRTC_INT_CFG);
	__raw_writel(0xff, rWP_MASK);
	__raw_writel(0xff, rRST_ST);
	__raw_writel(0x03, rGPOW_CFG);
	asm("b 1f\n\t"
	    ".align 5\n\t"
	    "1:\n\t"
	    "mcr p15, 0, %0, c8, c5, 0\n\t"		/* invalidate i-TLB */
	    "mcr p15, 0, %0, c8, c6, 0\n\t"		/* invalidate d-TLB */
	    "mcr p15, 0, %0, c7, c10, 0\n\t"      /* clean dcache */
	    "mcr p15, 0, %0, c7, c10, 5\n\t"	/* Data Memory Barrier Operation */
	    "mcr p15, 0, %0, c7, c10, 4\n\t"	/* data synchronization barrier operation */
	    "mcr p15, 0, %0, c7, c6, 0\n\t"       /* invalidate dcache */
	    "mcr p15, 0, %0, c7, c5, 0\n\t"	      /* invalidate icache */
	    "mcr p15, 0, %0, c7, c0, 4" :: "r" (tmp));

	/* we should never get past here */

	panic("sleep resumed to originator?");
}

static void imapx200_pm_prepare(void)
{
	/* store address of resume. */
	__raw_writel(virt_to_phys(imapx200_cpu_resume), rINFO0);
	/* ensure previous wakeup state is cleared before sleeping */
//	__raw_writel(__raw_readl(imapx200_WAKEUP_STAT), imapx200_WAKEUP_STAT);
}

int imapx200_pm_init(void)
{
	pm_cpu_prep = imapx200_pm_prepare;
	pm_cpu_sleep = imapx200_cpu_suspend;
	pm_uart_udivslot = 1;
	return 0;
}

arch_initcall(imapx200_pm_init);
