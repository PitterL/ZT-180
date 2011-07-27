/***************************************************************************** 
** drivers/serial/imapx200_serial.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Main file of Serial Port Driver.
**
** Author:
**     Feng Jiaxing <jiaxing_feng@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  09/17/2009  Feng Jiaxing
*****************************************************************************/ 

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/sysrq.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <asm/irq.h>

#include <mach/hardware.h>
#include <plat/regs-serial.h>
//#include <plat/fpga_test.h>
#include "imapx200_serial.h"

/* UART name and device definitions */
#define IMAPX200_SERIAL_NAME	"ttySAC"
#define IMAPX200_SERIAL_MAJOR	204
#define IMAPX200_SERIAL_MINOR	64

/* port irq numbers */
#define UART_IRQ(port) ((port)->irq)

/* macros to change one thing to another */
#define tx_enabled(port) ((port)->unused[0])
#define rx_enabled(port) ((port)->unused[1])

/* flag to ignore all characters comming in */
#define RXSTAT_DUMMY_READ (0x10000000)

static inline void uart_enable_irq (struct uart_port *port, uint val)
{
	uint mask;

	mask = rd_regl(port, IMAPX200_IER);
	mask |= val;
	wr_regl(port, IMAPX200_IER, mask);
}

static inline void uart_disable_irq (struct uart_port *port, uint val)
{
	uint mask;

	mask = rd_regl(port, IMAPX200_IER);
	mask &= ~val;
	wr_regl(port, IMAPX200_IER, mask);
}

static inline struct imapx200_uart_port *to_ourport(struct uart_port *port)
{
	return container_of(port, struct imapx200_uart_port, port);
}

/* translate a port to the device name */
static inline const char *imapx200_serial_portname(struct uart_port *port)
{
	return to_platform_device(port->dev)->name;
}

static int imapx200_serial_txempty_nofifo(struct uart_port *port)
{
	return ((rd_regl(port, IMAPX200_LSR) & IMAPX200_LSR_THRE_TX_HOLD_EMPTY) ? 1 : 0);
}

static void imapx200_serial_rx_enable(struct uart_port *port)
{
	unsigned long flags;
	unsigned int fcr;
	int count = 10000;

	spin_lock_irqsave(&port->lock, flags);

	while (--count && !imapx200_serial_txempty_nofifo(port))
		udelay(100);

	fcr |= IMAPX200_FCR_RFIFOR_RX_FIFO_RESET | IMAPX200_FCR_FIFOE_FIFO_ENABLE;
	wr_regl(port, IMAPX200_FCR, fcr);

	rx_enabled(port) = 1;
	
	spin_unlock_irqrestore(&port->lock, flags);
}

static void imapx200_serial_rx_disable(struct uart_port *port)
{
	unsigned long flags;

	spin_lock_irqsave(&port->lock, flags);

	rx_enabled(port) = 0;
	
	spin_unlock_irqrestore(&port->lock, flags);
}

static void imapx200_serial_stop_tx(struct uart_port *port)
{
	struct imapx200_uart_port *ourport = to_ourport(port);

	if (tx_enabled(port))
	{
		uart_disable_irq(port, UART_THRE_INT);
		uart_disable_irq(port, UART_TX_INT);
		tx_enabled(port) = 0;
		if (port->flags & UPF_CONS_FLOW)
			imapx200_serial_rx_enable(port);
	}
}

static void imapx200_serial_start_tx(struct uart_port *port)
{
	struct imapx200_uart_port *ourport = to_ourport(port);
	if (!tx_enabled(port)) 
	{
		if (port->flags & UPF_CONS_FLOW)
			imapx200_serial_rx_disable(port);

		uart_enable_irq(port, UART_TX_INT);
		uart_enable_irq(port, UART_THRE_INT);

		tx_enabled(port) = 1;
	}
}

static void imapx200_serial_stop_rx(struct uart_port *port)
{
	struct imapx200_uart_port *ourport = to_ourport(port);

	if (rx_enabled(port))
	{
		dbg("imapx200_serial_stop_rx: port=%p\n", port);
		disable_irq_nosync(ourport->rx_irq);
#if 1
		uart_disable_irq(port, UART_RX_INT);
		uart_disable_irq(port, UART_ERR_INT);
#endif	
		rx_enabled(port) = 0;
	}
}

static void imapx200_serial_enable_ms(struct uart_port *port)
{

}

static inline struct imapx200_uart_info *imapx200_port_to_info(struct uart_port *port)
{
	return to_ourport(port)->info;
}

static inline struct imapx200_uartcfg *imapx200_port_to_cfg(struct uart_port *port)
{
	if (port->dev == NULL)
		return NULL;

	return (struct imapx200_uartcfg *)port->dev->platform_data;
}

static int imapx200_serial_rx_fifocnt(struct imapx200_uart_port *ourport,
	unsigned long usr, unsigned long rfl)
{
	struct imapx200_uart_info *info = ourport->info;

	if (usr & info->rx_fifofull)
		return info->fifosize;

	return (rfl & info->rx_fifomask) >> info->rx_fifoshift;
}

static irqreturn_t imapx200_serial_rx_chars(int irq, void *dev_id)
{
	struct imapx200_uart_port *ourport = dev_id;
	struct uart_port *port = &ourport->port;
	struct tty_struct *tty = port->state->port.tty;
	unsigned int ch, flag;
	unsigned int usr, rfl, lsr, fcr;
	int max_count = 64;

	while (max_count-- > 0)
	{
		usr = rd_regl(port, IMAPX200_USR);
		rfl = rd_regl(port, IMAPX200_RFL);
		if (imapx200_serial_rx_fifocnt(ourport, usr, rfl) == 0)
			break;

		lsr = rd_regl(port, IMAPX200_LSR);
		ch = rd_regl(port, IMAPX200_RBR);

		if (port->flags & UPF_CONS_FLOW)
		{
			int txe = imapx200_serial_txempty_nofifo(port);

			if (rx_enabled(port))
			{
				if (!txe)
				{
					rx_enabled(port) = 0;
					continue;
				}
			}
			else
			{
				if (txe)
				{
					fcr |= IMAPX200_FCR_RFIFOR_RX_FIFO_RESET | IMAPX200_FCR_FIFOE_FIFO_ENABLE;
					wr_regl(port, IMAPX200_FCR, fcr);
					rx_enabled(port) = 1;
					goto out;
				}
				continue;
			}
		}

		/* insert the character into the buffer */
		flag = TTY_NORMAL;
		port->icount.rx++;

		if (unlikely(lsr & IMAPX200_LSR_ANY))
		{
			dbg("rxerr: port ch=0x%02x, rxs=0x%08x\n", ch, lsr);
			
			/* check for break */
			if (lsr & IMAPX200_LSR_BI_Break_INT)
			{
				dbg("break!\n");
				port->icount.brk++;
				if (uart_handle_break(port))
				    goto ignore_char;
			}

			if (lsr & IMAPX200_LSR_FE_FRAME_ERR)
				port->icount.frame++;
			if (lsr & IMAPX200_LSR_OE_OVERRUN_ERR)
				port->icount.overrun++;

			lsr &= port->read_status_mask;

			if (lsr & IMAPX200_LSR_BI_Break_INT)
				flag = TTY_BREAK;
			else if (lsr & IMAPX200_LSR_PE_PARITY_ERR)
				flag = TTY_PARITY;
			else if (lsr & (IMAPX200_LSR_FE_FRAME_ERR | IMAPX200_LSR_OE_OVERRUN_ERR))
				flag = TTY_FRAME;
		}

		if (uart_handle_sysrq_char(port, ch))
			goto ignore_char;

		uart_insert_char(port, lsr, IMAPX200_LSR_OE_OVERRUN_ERR, ch, flag);

 ignore_char:
		continue;
	}
	tty_flip_buffer_push(tty);

 out:
	return IRQ_HANDLED;
}

static irqreturn_t imapx200_serial_tx_chars(int irq, void *id)
{
	struct imapx200_uart_port *ourport = id;
	struct uart_port *port = &ourport->port;
	struct circ_buf *xmit = &port->state->xmit;
	int count = 256;

	if (port->x_char)
	{
		wr_regb(port, IMAPX200_THR, port->x_char);
		port->icount.tx++;
		port->x_char = 0;
		goto out;
	}

	/* if there isnt anything more to transmit, or the uart is now
	 * stopped, disable the uart and exit
	*/
	if (uart_circ_empty(xmit) || uart_tx_stopped(port))
	{
		imapx200_serial_stop_tx(port);
		goto out;
	}

	/* try and drain the buffer... */
	while (!uart_circ_empty(xmit) && count-- > 0)
	{
		if((rd_regl(port, IMAPX200_IIR) & IMAPX200_IIR_FIFOSE_MASK) == IMAPX200_IIR_FIFOSE_ENABLE)
		{
			if (rd_regl(port, IMAPX200_USR) & ourport->info->tx_fifonotfull)
			{
				wr_regb(port, IMAPX200_THR, xmit->buf[xmit->tail]);
				xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
				port->icount.tx++;
			}
		}
		else
		{
			if (rd_regl(port, IMAPX200_LSR) & IMAPX200_LSR_THRE_TX_HOLD_EMPTY)
			{
				wr_regb(port, IMAPX200_THR, xmit->buf[xmit->tail]);
				xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
				port->icount.tx++;
			}

		}
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(port);

	if (uart_circ_empty(xmit))
		imapx200_serial_stop_tx(port);

 out:
	return IRQ_HANDLED;
}

static unsigned int imapx200_serial_tx_empty(struct uart_port *port)
{
	struct imapx200_uart_info *info = imapx200_port_to_info(port);
	unsigned long usr, iir;
	int ret = 0;

	iir = rd_regl(port, IMAPX200_IIR);
	if ((iir & IMAPX200_IIR_FIFOSE_MASK) == IMAPX200_IIR_FIFOSE_ENABLE)
	{
		/* fifo mode - check ammount of data in fifo registers... */
		usr = rd_regl(port, IMAPX200_USR);
		ret = (usr & info->tx_fifoempty) ? 1 : 0;
	}
	else
	{
		/* in non-fifo mode, we go and use the tx buffer empty */
		ret = imapx200_serial_txempty_nofifo(port);
	}
	
	return ret;
}

/* no modem control lines */
static unsigned int imapx200_serial_get_mctrl(struct uart_port *port)
{
	unsigned int csr = rd_regb(port, IMAPX200_CSR);

	if (csr & IMAPX200_CSR_CTS_ASSERT)
		return TIOCM_CAR | TIOCM_DSR | TIOCM_CTS;
	else
		return TIOCM_CAR | TIOCM_DSR;
}

static void imapx200_serial_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	/* todo - possibly remove AFC and do manual CTS */
}

static void imapx200_serial_break_ctl(struct uart_port *port, int break_state)
{
	unsigned long flags;
	unsigned int lcr;

	spin_lock_irqsave(&port->lock, flags);

	lcr = rd_regl(port, IMAPX200_LCR);

	if (break_state)
		lcr |= IMAPX200_LCR_Break_ENABLE;
	else
		lcr &= ~IMAPX200_LCR_Break_ENABLE;

	wr_regl(port, IMAPX200_LCR, lcr);

	spin_unlock_irqrestore(&port->lock, flags);
}

static void imapx200_serial_shutdown(struct uart_port *port)
{
	struct imapx200_uart_port *ourport = to_ourport(port);
	if (ourport->tx_claimed || ourport->rx_claimed)
	{
		free_irq(UART_IRQ(port), ourport);

		ourport->tx_claimed = 0;
		ourport->rx_claimed = 0;
		tx_enabled(port) = 0;
		rx_enabled(port) = 0;
	}
}

static irqreturn_t imapx200_serial_interrupt(int irq, void *dev_id)
{
	struct imapx200_uart_port *ourport = dev_id;
	struct uart_port *port = &ourport->port;
	unsigned long int_id,line_sts;
	irqreturn_t ret = IRQ_HANDLED;

	int_id = rd_regl(port, IMAPX200_IIR) & IMAPX200_IIR_IID_MASK;

	switch (int_id)
	{
		case IMAPX200_IIR_IID_NO_INT:
			break;
		case IMAPX200_IIR_IID_TX:
			ret = imapx200_serial_tx_chars(irq, dev_id);
			break;
		case IMAPX200_IIR_IID_RX:
			ret = imapx200_serial_rx_chars(irq, dev_id);
			break;
		case IMAPX200_IIR_IID_LINE_STATUS:
		case IMAPX200_IIR_IID_BUSY_DETECT:
		case IMAPX200_IIR_IID_TIMEOUT:
		    line_sts=rd_regl(port, IMAPX200_LSR);
		default:
			break;
	}

	return ret;
}

static int imapx200_serial_startup(struct uart_port *port)
{
	struct imapx200_uart_port *ourport = to_ourport(port);
	int ret;
	unsigned long flags;
	
	dbg("imapx200_serial_startup: port=%p (%08lx,%p)\n",
		port->mapbase, port->membase);

//	local_irq_save(flags);

	rx_enabled(port) = 1;
//	tx_enabled(port) = 1;

	uart_enable_irq(port, UART_RX_INT);
	uart_enable_irq(port, UART_ERR_INT);
//	uart_enable_irq(port, UART_THRE_INT);
	ret = request_irq(UART_IRQ(port), imapx200_serial_interrupt, IRQF_DISABLED,
	imapx200_serial_portname(port), ourport);
	if (ret != 0)
	{
		printk(KERN_ERR "cannot get irq %d\n", UART_IRQ(port));
		goto err;
	}

	ourport->tx_claimed = 1;
	ourport->rx_claimed = 1;

	dbg("imapx200_serial_startup ok\n");

	/* the port reset code should have done the correct
	 * register setup for the port controls */
//	local_irq_restore(flags);
	return ret;

 err:
	imapx200_serial_shutdown(port);
	
	return ret;
}

/* power power management control */

static void imapx200_serial_pm(struct uart_port *port, unsigned int level, unsigned int old)
{
	struct imapx200_uart_port *ourport = to_ourport(port);

	switch (level)
	{
		case 3:
			if (!IS_ERR(ourport->baudclk) && (ourport->baudclk != NULL))
				clk_disable(ourport->baudclk);

			clk_disable(ourport->clk);
			break;
		case 0:
			clk_enable(ourport->clk);

			if (!IS_ERR(ourport->baudclk) && (ourport->baudclk != NULL))
				clk_enable(ourport->baudclk);

			break;
		default:
			printk(KERN_ERR "imapx200_serial: unknown pm %d\n", level);
	}
}

/* baud rate calculation
 *
 * The UARTs on the S3C2410/S3C2440 can take their clocks from a number
 * of different sources, including the peripheral clock ("pclk") and an
 * external clock ("uclk"). The S3C2440 also adds the core clock ("fclk")
 * with a programmable extra divisor.
 *
 * The following code goes through the clock sources, and calculates the
 * baud clocks (and the resultant actual baud rates) and then tries to
 * pick the closest one and select that.
 *
*/
#define MAX_CLKS (8)

static struct imapx200_uart_clksrc tmp_clksrc = {
	.name		= "pclk",
	.min_baud	= 0,
	.max_baud	= 0,
};

struct baud_calc {
	struct imapx200_uart_clksrc *clksrc;
	unsigned int calc;
	unsigned int divisor;
	struct clk *src;
};

static int imapx200_serial_calcbaud(struct baud_calc *calc, struct uart_port *port,
	struct imapx200_uart_clksrc *clksrc, unsigned int baud)
{
	unsigned long rate;
	unsigned long v0, v1, v2;
	
	calc->src = clk_get(port->dev, clksrc->name);
	if (calc->src == NULL || IS_ERR(calc->src))
		return 0;

	rate = clk_get_rate(calc->src);

	calc->clksrc = clksrc;

	v1 = rate / (16 * baud);
	v2 = rate % (16 * baud);
	if (v2 >= ((16 * baud) / 2))
		v0 = v1 + 1;
	else
		v0 = v1;

	calc->divisor = v0;
 
	calc->calc = (rate / (calc->divisor * 16));
 	
	printk(KERN_DEBUG  "FOUND calc %d, divisor %d\n",
		calc->calc, calc->divisor);

	return 1;
}

static unsigned int imapx200_serial_getclk(struct uart_port *port,
	struct imapx200_uart_clksrc **clksrc, struct clk **clk,
	unsigned int baud)
{
	struct imapx200_uartcfg *cfg = imapx200_port_to_cfg(port);
	struct imapx200_uart_clksrc *clkp;
	struct baud_calc res[MAX_CLKS];
	struct baud_calc *resptr, *best, *sptr;
	int i;

	clkp = cfg->clocks;
	best = NULL;
	if (cfg->clocks_size < 2)
	{
		if (cfg->clocks_size == 0)
			clkp = &tmp_clksrc;		

		imapx200_serial_calcbaud(res, port, clkp, baud);
		best = res;
		resptr = best + 1;
	}
	else
	{
		resptr = res;

		for (i = 0; i < cfg->clocks_size; i++, clkp++)
		{
			if (imapx200_serial_calcbaud(resptr, port, clkp, baud))
				resptr++;
		}
	}

	/* ok, we now need to select the best clock we found */
	if (!best)
	{
		unsigned int deviation = (1<<30)|((1<<30)-1);
		int calc_deviation;

		for (sptr = res; sptr < resptr; sptr++)
		{
			calc_deviation = baud - sptr->calc;
			if (calc_deviation < 0)
				calc_deviation = -calc_deviation;

			if (calc_deviation < deviation)
			{
				best = sptr;
				deviation = calc_deviation;
			}
		}
	}

	/* store results to pass back */
	*clksrc = best->clksrc;
	*clk = best->src;

	return best->divisor;
}

/*
 ***********************************************************************
 * -Function:
 *    imapx200_serial_setsource( INPUT1 *port, INPUT2 *clk, OUTPUT1 none)
 *
 * -Description:
 *    This function implement special features. The process is,
 *        1. Select clock source for input serial port;
 *
 * -Input Param
 *    *port	: struct uart_port
 *    *clk		: struct imapx200_uart_clksrc
 *
 * -Output Param
 *    none
 *                
 * -Return
 *    0		: correct
 *    others	: error
 *
 * -Others
 *    none
 ***********************************************************************
 */
static int imapx200_serial_setsource(struct uart_port *port, struct imapx200_uart_clksrc *clk)
{
	unsigned long cksr = rd_regl(port, IMAPX200_CKSR);

	cksr &= ~IMAPX200_CKSR_CKSEL_MASK;

	if (strcmp(clk->name, "xext") == 0)
		cksr |= IMAPX200_CKSR_CKSEL_UEXTCLK;
	else if (strcmp(clk->name, "pclk") == 0)
		cksr |= IMAPX200_CKSR_CKSEL_PCLK;
	else
	{
		printk(KERN_ERR "imapx200 unknown clock source %s\n", clk->name);
		return -EINVAL;
	}

	wr_regl(port, IMAPX200_CKSR, cksr);

	return 0;
}

static void imapx200_serial_set_termios(struct uart_port *port,
	struct ktermios *termios, struct ktermios *old)
{
	struct imapx200_uartcfg *cfg = imapx200_port_to_cfg(port);
	struct imapx200_uart_port *ourport = to_ourport(port);
	struct imapx200_uart_clksrc *clksrc = NULL;
	struct clk *clk = NULL;
	unsigned long flags;
	unsigned int baud, divisor = 0;
	unsigned int lcr = 0, mcr = 0;

	/*
	 * We don't support modem control lines.
	 */
	termios->c_cflag &= ~(HUPCL | CMSPAR);
	termios->c_cflag |= CLOCAL;

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old, 0, 115200*8);

	if ((baud == 38400) && ((port->flags & UPF_SPD_MASK) == UPF_SPD_CUST))
		divisor = port->custom_divisor;
	else
		divisor = imapx200_serial_getclk(port, &clksrc, &clk, baud);

	/* check to see if we need  to change clock source */
	if (ourport->clksrc != clksrc || ourport->baudclk != clk)
	{
		imapx200_serial_setsource(port, clksrc);
	
		if (ourport->baudclk != NULL && !IS_ERR(ourport->baudclk))
		{
			clk_disable(ourport->baudclk);
			ourport->baudclk  = NULL;
		}

		clk_enable(clk);

		ourport->clksrc = clksrc;
		ourport->baudclk = clk;
	}

	switch (termios->c_cflag & CSIZE)
	{
		case CS5:
			dbg("config: 5bits/char\n");
			lcr |= IMAPX200_LCR_DLS_5BIT;
			break;
		case CS6:
			dbg("config: 6bits/char\n");
			lcr |= IMAPX200_LCR_DLS_6BIT;
			break;
		case CS7:
			dbg("config: 7bits/char\n");
			lcr |= IMAPX200_LCR_DLS_7BIT;
			break;
		case CS8:
		default:
			dbg("config: 8bits/char\n");
			lcr |= IMAPX200_LCR_DLS_8BIT;
			break;
	}

	/* preserve original lcon IR settings */
	mcr |= (cfg->mcr & IMAPX200_MCR_SIRE_IRDA_ENABLE);

	if (termios->c_cflag & CSTOPB)
		lcr |= IMAPX200_LCR_STOP_1POINT5_2_STOP_BIT;

	if (termios->c_cflag & CRTSCTS) 
		mcr |= IMAPX200_MCR_AFCE_AFC_ENABLE;
	else
		mcr &= ~IMAPX200_MCR_AFCE_AFC_ENABLE;

	if (termios->c_cflag & PARENB)
	{
		lcr |= IMAPX200_LCR_PEN_PARITY_ENABLE;
		if (termios->c_cflag & PARODD)
			lcr &= ~IMAPX200_LCR_EPS_EVEN_PARITY;
		else
			lcr |= IMAPX200_LCR_EPS_EVEN_PARITY;
	}
	else
	{
		lcr &= ~IMAPX200_LCR_PEN_PARITY_ENABLE;
	}

	spin_lock_irqsave(&port->lock, flags);

	dbg("setting lcr to %08x, divisor to %d\n", lcr, divisor);

	wr_regl(port, IMAPX200_LCR, (lcr | IMAPX200_LCR_DLAB_ENABLE));
	wr_regl(port, IMAPX200_DLL, (divisor & 0xff));
	wr_regl(port, IMAPX200_DLH, ((divisor & 0xff00) >> 8));
	wr_regl(port, IMAPX200_LCR, (lcr & ~IMAPX200_LCR_DLAB_ENABLE));
	wr_regl(port, IMAPX200_LCR, lcr);
	wr_regl(port, IMAPX200_MCR, mcr);

	dbg("uart: lcr = 0x%08x, mcr = 0x%08x\n", rd_regl(port, IMAPX200_LCR),
		rd_regl(port, IMAPX200_MCR));

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	/*
	 * Which character status flags are we interested in?
	 */
	port->read_status_mask = IMAPX200_LSR_OE_MASK;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= IMAPX200_LSR_FE_MASK | IMAPX200_LSR_PE_MASK;

	/*
	 * Which character status flags should we ignore?
	 */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= IMAPX200_LSR_OE_MASK;
	if (termios->c_iflag & IGNBRK && termios->c_iflag & IGNPAR)
		port->ignore_status_mask |= IMAPX200_LSR_FE_MASK;

	/*
	 * Ignore all characters if CREAD is not set.
	 */
	if ((termios->c_cflag & CREAD) == 0)
		port->ignore_status_mask |= RXSTAT_DUMMY_READ;

	spin_unlock_irqrestore(&port->lock, flags);
}

static const char *imapx200_serial_type(struct uart_port *port)
{
	switch (port->type)
	{
		case PORT_IMAPX200:
			return "IMAPX200";
		default:
			return NULL;
	}
}

#define MAP_SIZE (0x200)

static void imapx200_serial_release_port(struct uart_port *port)
{
	release_mem_region(port->mapbase, MAP_SIZE);
}

static int imapx200_serial_request_port(struct uart_port *port)
{
	const char *name = imapx200_serial_portname(port);
	
	return request_mem_region(port->mapbase, MAP_SIZE, name) ? 0 : -EBUSY;
}

static void imapx200_serial_config_port(struct uart_port *port, int flags)
{
	struct imapx200_uart_info *info = imapx200_port_to_info(port);

	if ((flags & UART_CONFIG_TYPE) && (imapx200_serial_request_port(port) == 0))
		port->type = info->type;
}

/*
 * verify the new serial_struct (for TIOCSSERIAL).
 */
static int imapx200_serial_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	struct imapx200_uart_info *info = imapx200_port_to_info(port);

	if (ser->type != PORT_UNKNOWN && ser->type != info->type)
		return -EINVAL;

	return 0;
}

static ssize_t imapx200_serial_show_clksrc(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct uart_port *port = imapx200_dev_to_port(dev);
	struct imapx200_uart_port *ourport = to_ourport(port);

	return snprintf(buf, PAGE_SIZE, "* %s\n", ourport->clksrc->name);
}

static DEVICE_ATTR(clock_source, S_IRUGO, imapx200_serial_show_clksrc, NULL);

#ifdef CONFIG_SERIAL_IMAP_CONSOLE
static struct console imapx200_serial_console;
#define IMAP_SERIAL_CONSOLE &imapx200_serial_console
#else
#define IMAP_SERIAL_CONSOLE NULL
#endif

static struct uart_ops imapx200_serial_ops = {
	.pm			= imapx200_serial_pm,
	.tx_empty	= imapx200_serial_tx_empty,
	.get_mctrl	= imapx200_serial_get_mctrl,
	.set_mctrl	= imapx200_serial_set_mctrl,
	.stop_tx		= imapx200_serial_stop_tx,
	.start_tx		= imapx200_serial_start_tx,
	.stop_rx		= imapx200_serial_stop_rx,
	.enable_ms	= imapx200_serial_enable_ms,
	.break_ctl	= imapx200_serial_break_ctl,
	.startup		= imapx200_serial_startup,
	.shutdown	= imapx200_serial_shutdown,
	.set_termios	= imapx200_serial_set_termios,
	.type		= imapx200_serial_type,
	.release_port	= imapx200_serial_release_port,
	.request_port	= imapx200_serial_request_port,
	.config_port	= imapx200_serial_config_port,
	.verify_port	= imapx200_serial_verify_port,
};

static struct uart_driver imapx200_uart_drv = {
	.owner		= THIS_MODULE,
	.dev_name	= IMAPX200_SERIAL_NAME,
	.nr			= 4,
	.cons		= IMAP_SERIAL_CONSOLE,
	.driver_name	= "imapx200_serial",
	.major		= IMAPX200_SERIAL_MAJOR,
	.minor		= IMAPX200_SERIAL_MINOR,
};

static struct imapx200_uart_port imapx200_serial_ports[NR_PORTS] = {
	[0] = {
		.port = {
			.lock		= __SPIN_LOCK_UNLOCKED(imapx200_serial_ports[0].port.lock),
			.iotype	= UPIO_MEM,
			.irq		= IRQ_UART0,
			.uartclk	= 0,
			.fifosize	= 64,
			.ops		= &imapx200_serial_ops,
			.flags	= UPF_BOOT_AUTOCONF,
			.line		= 0,
		}
	},
	[1] = {
		.port = {
			.lock		= __SPIN_LOCK_UNLOCKED(imapx200_serial_ports[1].port.lock),
			.iotype	= UPIO_MEM,
			.irq		= IRQ_UART1,
			.uartclk	= 0,
			.fifosize	= 64,
			.ops		= &imapx200_serial_ops,
			.flags	= UPF_BOOT_AUTOCONF,
			.line		= 1,
		}
	},
	[2] = {
		.port = {
			.lock		= __SPIN_LOCK_UNLOCKED(imapx200_serial_ports[2].port.lock),
			.iotype	= UPIO_MEM,
			.irq		= IRQ_UART2,
			.uartclk	= 0,
			.fifosize	= 64,
			.ops		= &imapx200_serial_ops,
			.flags	= UPF_BOOT_AUTOCONF,
			.line		= 2,
		}
	},
#if 1
	[3] = {
		.port = {
			.lock		= __SPIN_LOCK_UNLOCKED(imapx200_serial_ports[3].port.lock),
			.iotype	= UPIO_MEM,
			.irq		= IRQ_UART3,
			.uartclk	= 0,
			.fifosize	= 64,
			.ops		= &imapx200_serial_ops,
			.flags	= UPF_BOOT_AUTOCONF,
			.line		= 3,
		}
	}
#endif
};

static int imapx200_serial_resetport(struct uart_port *port, struct imapx200_uartcfg *cfg)
{
	dbg("imapx200_serial_resetport: port=%p (%08lx), cfg=%p\n", port, port->mapbase, cfg);

	wr_regl(port, IMAPX200_LCR,  cfg->lcr);
	wr_regl(port, IMAPX200_MCR,  cfg->mcr);


	/* reset tx and rx fifos */

//	printk(KERN_INFO "cfg->fcr is %d\n",cfg->fcr);
	wr_regl(port, IMAPX200_FCR, (cfg->fcr | IMAPX200_FCR_RFIFOR_RX_FIFO_RESET |
		IMAPX200_FCR_XFIFOR_TX_FIFO_RESET));
	wr_regl(port, IMAPX200_FCR, cfg->fcr);
#if defined(FPGA_TEST)
	wr_regl(port, IMAPX200_CKSR,  IMAPX200_CKSR_CKSEL_UEXTCLK);
#else
	wr_regl(port, IMAPX200_CKSR,  IMAPX200_CKSR_CKSEL_PCLK);
#endif
	return 0;
}

/*
 ***********************************************************************
 * -Function:
 *    imapx200_serial_init_port( INPUT1 *  pInput1, INPUT2 *pInput2, OUTPUT1 *pOutput1)
 *
 * -Description:
 *    This function implement special features. The process is,
 *        1. Initialise a single serial port from the platform device given
 *
 * -Input Param
 *    *info		: struct imapx200_uart_info
 *    *platdev	: struct platform_device
 *
 * -Output Param
 *    *ourport	: struct imapx200_uart_port
 *                
 * -Return
 *    0		: correct
 *    others	: error
 *
 * -Others
 *    None. 
 ***********************************************************************
 */
static int imapx200_serial_init_port(struct imapx200_uart_port *ourport,
	struct imapx200_uart_info *info, struct platform_device *platdev)
{
	struct uart_port *port = &ourport->port;
	struct imapx200_uartcfg *cfg;
	struct resource *res;
	int ret;

	dbg("imapx200_serial_init_port: port=%p, platdev=%p\n", port, platdev);

	if (platdev == NULL)
		return -ENODEV;

	cfg = imapx200_dev_to_cfg(&platdev->dev);

	if (port->mapbase != 0)
		return 0;

	if (cfg->hwport > 3)
		return -EINVAL;

	/* setup info for port */
	port->dev = &platdev->dev;
	ourport->info = info;

	/* copy the info in from provided structure */
	ourport->port.fifosize = info->fifosize;

	dbg("imapx200_serial_init_port: %p (hw %d)...\n", port, cfg->hwport);

	port->uartclk = 1;

	if (cfg->uart_flags & UPF_CONS_FLOW)
	{
		dbg("imapx200_serial_init_port: enabling flow control\n");
		port->flags |= UPF_CONS_FLOW;
	}

	/* sort our the physical and virtual addresses for each UART */
	res = platform_get_resource(platdev, IORESOURCE_MEM, 0);
	if (res == NULL)
	{
		printk(KERN_ERR "failed to find memory resource for uart\n");
		return -EINVAL;
	}

	dbg("resource %p (%lx..%lx)\n", res, res->start, res->end);

	port->mapbase = res->start;
	port->membase = IMAP_VA_UART + (res->start - IMAPX200_PA_UART0);
	ret = platform_get_irq(platdev, 0);
#if 0
	if (ret < 0)
		port->irq = 0;
	else
		port->irq = ret;
#endif
	if (ret < 0)
                port->irq = 0;
        else {
                port->irq = ret;
                ourport->rx_irq = ret;
                ourport->tx_irq = ret + 1;
        }

        ret = platform_get_irq(platdev, 1);
        
	if (ret > 0)
                ourport->tx_irq = ret;

#if 1
	ourport->clk	= clk_get(&platdev->dev, "UART0");

	dbg("port: map=%08x, mem=%08x, irq=%d (%d,%d), clock=%ld\n",
	    port->mapbase, port->membase, port->irq, ourport->rx_irq, ourport->tx_irq, port->uartclk);
#endif
	/* reset the fifos (and setup the uart) */
	imapx200_serial_resetport(port, cfg);
	
	return 0;
}

/*
 ***********************************************************************
 * -Function:
 *    imapx200_serial_getsource( INPUT1 *port, OUTPUT1 *clk)
 *
 * -Description:
 *    This function implement special features. The process is,
 *        1. Get clock source of input serial port;
 *
 * -Input Param
 *    *port	: struct uart_port
 *
 * -Output Param
 *    *clk		: struct imapx200_uart_clksrc
 *                
 * -Return
 *    0		: correct
 *
 * -Others
 *    none
 ***********************************************************************
 */
static int imapx200_serial_getsource(struct uart_port *port, struct imapx200_uart_clksrc *clk)
{
	unsigned long cksr = rd_regl(port, IMAPX200_CKSR);

	switch (cksr & IMAPX200_CKSR_CKSEL_MASK)
	{
		case IMAPX200_CKSR_CKSEL_PCLK:
			clk->name = "pclk";
			break;

		case IMAPX200_CKSR_CKSEL_UEXTCLK:
			clk->name = "xext";
			break;

		default:
			printk(KERN_ERR "imapx200 clock source error\n");
	}

	return 0;
}
static struct imapx200_uart_info imapx200_uart_inf = {
	.name			= "IMAP X200 UART",
	.type			= PORT_IMAPX200,
	.fifosize			= 64,
	.rx_fifomask		= IMAPX200_RFL_RX_FIFO_LEVEL_MASK,
	.rx_fifoshift		= 0,
	.rx_fifofull		= IMAPX200_USR_RFF_MASK,
	.rx_fifonotempty	= IMAPX200_USR_RFNE_MASK,
	.tx_fifomask		= IMAPX200_RFL_RX_FIFO_LEVEL_MASK,
	.tx_fifoshift		= 0,
	.tx_fifoempty		= IMAPX200_USR_TFE_MASK,
	.tx_fifonotfull		= IMAPX200_USR_TFNF_MASK,
	.get_clksrc		= imapx200_serial_getsource,
	.set_clksrc		= imapx200_serial_setsource,
	.reset_port		= imapx200_serial_resetport,
};

/* Device driver serial port probe */
static int probe_index;
static int imapx200_serial_probe(struct platform_device *dev)
{
	struct imapx200_uart_port *ourport;
	int ret;

	dbg("imapx200_serial_probe(%p) %d\n", dev, probe_index);

//	printk(KERN_INFO "imapx200_serial_probe(%p) %d\n", dev, probe_index);
	ourport = &imapx200_serial_ports[probe_index];
	probe_index++;

	dbg("%s: initialising port %p...\n", __func__, ourport);

	ret = imapx200_serial_init_port(ourport, &imapx200_uart_inf, dev);
	if (ret < 0)
		goto probe_err;

	dbg("%s: adding port\n", __func__);
	
	uart_add_one_port(&imapx200_uart_drv, &ourport->port);
	platform_set_drvdata(dev, &ourport->port);

	ret = device_create_file(&dev->dev, &dev_attr_clock_source);
	if (ret < 0)
		printk(KERN_ERR "%s: failed to add clksrc attr.\n", __func__);

	return 0;

 probe_err:
	return ret;
}

static int imapx200_serial_remove(struct platform_device *dev)
{
	struct uart_port *port = imapx200_dev_to_port(&dev->dev);

	if (port)
	{
		device_remove_file(&dev->dev, &dev_attr_clock_source);
		uart_remove_one_port(&imapx200_uart_drv, port);
	}

	return 0;
}

/* UART power management code */
#ifdef CONFIG_PM
static int imapx200_serial_suspend(struct platform_device *dev, pm_message_t state)
{
	struct uart_port *port = imapx200_dev_to_port(&dev->dev);

	if (port)
		uart_suspend_port(&imapx200_uart_drv, port);

	return 0;
}

static int imapx200_serial_resume(struct platform_device *dev)
{
	struct uart_port *port = imapx200_dev_to_port(&dev->dev);
	struct imapx200_uart_port *ourport = to_ourport(port);

	if (port)
	{
		clk_enable(ourport->clk);
		imapx200_serial_resetport(port, imapx200_port_to_cfg(port));
		clk_disable(ourport->clk);

		uart_resume_port(&imapx200_uart_drv, port);
	}

	return 0;
}
#endif

static struct platform_driver imapx200_serial_drv = {
	.probe		= imapx200_serial_probe,
	.remove		= imapx200_serial_remove,
#ifdef CONFIG_PM
	.suspend 		= imapx200_serial_suspend,
	.resume 		= imapx200_serial_resume,
#endif
	.driver		= {
		.name	= "imapx200-uart",
		.owner	= THIS_MODULE,
	},
};

static int __init imapx200_serial_init(void)
{
	int ret;

//	printk(KERN_INFO "in imapx200_serial_init\n");
	ret = uart_register_driver(&imapx200_uart_drv);
	if (ret < 0)
	{
		printk(KERN_ERR "imapx200 failed to register UART driver\n");
		return -1;
	}

	return platform_driver_register(&imapx200_serial_drv);
}

static void __exit imapx200_serial_exit(void)
{
	uart_unregister_driver(&imapx200_uart_drv);
	platform_driver_unregister(&imapx200_serial_drv);
}

module_init(imapx200_serial_init);
module_exit(imapx200_serial_exit);

/* Console code */
#ifdef CONFIG_SERIAL_IMAP_CONSOLE

static struct uart_port *cons_uart;

static int imapx200_serial_console_txrdy(struct uart_port *port, unsigned int fcr)
{
	struct imapx200_uart_info *info = imapx200_port_to_info(port);
	unsigned long usr, lsr;
	int ret = 1;

//	if (fcr & IMAPX200_FCR_FIFOE_FIFO_ENABLE)
//	{
//		/* fifo mode - check ammount of data in fifo registers... */
//		usr = rd_regl(port, IMAPX200_USR);
//		ret = (usr & info->tx_fifonotfull) ? 1 : 0;
//	}
//	else
//	{
		/* in non-fifo mode, we go and use the tx buffer empty */
		lsr = rd_regl(port, IMAPX200_LSR);
		ret = (lsr & (IMAPX200_LSR_TEMT_XMITER_EMPTY| IMAPX200_LSR_THRE_MASK)) ? 1 : 0;
//	}
	
	return ret;
}

static void imapx200_serial_console_putchar(struct uart_port *port, int ch)
{
//	unsigned int fcr = rd_regl(cons_uart, IMAPX200_FCR);
	
	while (!imapx200_serial_console_txrdy(port, 0))
		barrier();
	wr_regb(cons_uart, IMAPX200_THR, ch);
}

static void imapx200_serial_console_write(struct console *co, const char *s, unsigned int count)
{
	uart_console_write(cons_uart, s, count, imapx200_serial_console_putchar);
}

static void __init imapx200_serial_get_options(struct uart_port *port, int *baud, int *parity, int *bits)
{
	struct imapx200_uart_clksrc clksrc;
	struct clk *clk;
	unsigned int lcr, dll, dlh;	
	unsigned long divisor;
	unsigned long freq;

	lcr = rd_regl(port, IMAPX200_LCR);
	wr_regl(port, IMAPX200_LCR, (lcr | IMAPX200_LCR_DLAB_ENABLE));
	dll = rd_regl(port, IMAPX200_DLL) & 0xff;
	dlh = rd_regl(port, IMAPX200_DLH) & 0xff;
	wr_regl(port, IMAPX200_LCR, (lcr & ~IMAPX200_LCR_DLAB_ENABLE));
	divisor = (dlh<<8) | dll;

	dbg("imapx200_serial_get_options: port=%p\n" "registers: lcr=%08x, divisor=%08x\n",
		port, lcr, divisor);

	if ((dll != 0) || (dlh!= 0))
	{
		/* consider the serial port configured if the UART enable */
		switch (lcr & IMAPX200_LCR_DLS_MASK)
		{
			case IMAPX200_LCR_DLS_5BIT:
				*bits = 5;
				break;
			case IMAPX200_LCR_DLS_6BIT:
				*bits = 6;
				break;
			case IMAPX200_LCR_DLS_7BIT:
				*bits = 7;
				break;			
			case IMAPX200_LCR_DLS_8BIT:
			default:
				*bits = 8;
				break;
		}

		if (lcr & IMAPX200_LCR_PEN_PARITY_ENABLE)
		{
			if (lcr & IMAPX200_LCR_EPS_EVEN_PARITY)
				*parity = 'e';
			else
				*parity = 'o';
		}
		else
			*parity = 'n';

		/* calculate the baud rate */
		imapx200_serial_getsource(port, &clksrc);
		clk = clk_get(port->dev, clksrc.name);
		if ((!IS_ERR(clk)) && (clk != NULL))
			freq = clk_get_rate(clk);
		*baud = freq / (16 * divisor);
		dbg("calculated baud %d\n", *baud);
	}
}

/* imapx200_serial_init_ports
 *
 * initialise the serial ports from the machine provided initialisation data.
*/
static int imapx200_serial_init_ports(struct imapx200_uart_info *info)
{
	struct imapx200_uart_port *ptr = imapx200_serial_ports;
	struct platform_device **platdev_ptr;
	int i;

	dbg("imapx200_serial_init_ports: initialising ports...\n");

	platdev_ptr = imap_uart_devs;
#if defined (FPGA_TEST_1)
		ptr += 2;
		platdev_ptr += 2;
		imapx200_serial_init_port(ptr, info, *platdev_ptr);
#else
	for (i = 0; i < NR_PORTS; i++, ptr ++, platdev_ptr++)
	{
		imapx200_serial_init_port(ptr, info, *platdev_ptr);
	}
#endif
	return 0;
}

static int __init imapx200_serial_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = 9600;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';

	dbg("imapx200_serial_console_setup: co=%p (%d), %s\n", co, co->index, options);

	/* Is this a valid port */
	if (co->index == -1 || co->index >= NR_PORTS)
		co->index = 0;

	port = &imapx200_serial_ports[co->index].port;

	/* Is the port configured? If not, then use port0 by default */
	if (port->mapbase == 0x0)
	{
		co->index = 0;
		port = &imapx200_serial_ports[co->index].port;
	}

	cons_uart = port;

	dbg("imapx200_serial_console_setup: port=%p (%d)\n", port, co->index);

	/*
	 * Check whether an invalid uart number has been specified, and if so,
	 * search for the first available port that does have console support.
	 */
	if (options)
	{
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	}
	else
		imapx200_serial_get_options(port, &baud, &parity, &bits);

	dbg("imapx200_serial_console_setup: baud %d\n", baud);

	return uart_set_options(port, co, baud, parity, bits, flow);
}

/* imapx200_serial_initconsole
 *
 * initialise the console from one of the uart drivers
*/
static struct console imapx200_serial_console = {
	.name		= IMAPX200_SERIAL_NAME,
	.device		= uart_console_device,
	.flags		= CON_PRINTBUFFER,
#if defined (FPGA_TEST)
	.index		= 2,
#else

	.index		= -1,
#endif
	.write		= imapx200_serial_console_write,
	.setup		= imapx200_serial_console_setup
};

int imapx200_serial_initconsole(struct platform_driver *drv, struct imapx200_uart_info *info)
{
#if defined (FPGA_TEST)	
	struct platform_device *dev = imap_uart_devs[2];
#else
	struct platform_device *dev = imap_uart_devs[0];
#endif
	dbg("imapx200_serial_initconsole\n");

	/* select driver based on the cpu */
	if (dev == NULL)
	{
		printk(KERN_ERR "imapx200: no devices for console init\n");
		return 0;
	}

	imapx200_serial_console.data = &imapx200_uart_drv;
	imapx200_serial_init_ports(info);

	register_console(&imapx200_serial_console);
	
	return 0;
}

imapx200_console_init(&imapx200_serial_drv, &imapx200_uart_inf);

#endif /* CONFIG_SERIAL_IMAP_CONSOLE */

MODULE_DESCRIPTION("IMAPX200 SoC Serial port driver");
MODULE_AUTHOR("Feng Jiaxing <jiaxing_feng@infotm.com>");
MODULE_LICENSE("GPL v2");
