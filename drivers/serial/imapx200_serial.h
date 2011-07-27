/***************************************************************************** 
** drivers/serial/imapx200_serial.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Head file of Serial Port Driver.
**
** Author:
**     Feng Jiaxing <jiaxing_feng@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  09/14/2009  Feng Jiaxing
*****************************************************************************/ 

#ifndef __IMAPX200_SERAIL_H
#define __IMAPX200_SERAIL_H

struct imapx200_uart_info {
	char			*name;
	unsigned int	type;
	unsigned int	fifosize;
	unsigned long	rx_fifomask;
	unsigned long	rx_fifoshift;
	unsigned long	rx_fifofull;
	unsigned long rx_fifonotempty;
	unsigned long	tx_fifomask;
	unsigned long	tx_fifoshift;
	unsigned long tx_fifoempty;
	unsigned long	tx_fifonotfull;	

	/* clock source control */
	int (*get_clksrc)(struct uart_port *, struct imapx200_uart_clksrc *clk);
	int (*set_clksrc)(struct uart_port *, struct imapx200_uart_clksrc *clk);

	/* uart controls */
	int (*reset_port)(struct uart_port *, struct imapx200_uartcfg *);
};

struct imapx200_uart_port {
	unsigned char rx_claimed;
	unsigned char tx_claimed;

	unsigned int                    rx_irq;
        unsigned int                    tx_irq;
	struct imapx200_uart_info *info;
	struct imapx200_uart_clksrc *clksrc;
	struct clk *clk;
	struct clk *baudclk;
	struct uart_port port;
};

/* Conversion Functions */
#define imapx200_dev_to_port(__dev)		(struct uart_port *)dev_get_drvdata(__dev)
#define imapx200_dev_to_cfg(__dev)		(struct imapx200_uartcfg *)((__dev)->platform_data)

/* Register Access Controls */
#define portaddr(port, reg) ((port)->membase + (reg))

#define rd_regb(port, reg) (__raw_readb(portaddr(port, reg)))
#define rd_regl(port, reg) (__raw_readl(portaddr(port, reg)))

#define wr_regb(port, reg, val) __raw_writeb(val, portaddr(port, reg))
#define wr_regl(port, reg, val) __raw_writel(val, portaddr(port, reg))


/* Functions */
//extern int imapx200_serial_probe(struct platform_device *dev, struct imapx200_uart_info *uart);
//extern int imapx200_serial_remove(struct platform_device *dev);
//extern int imapx200_serial_initconsole(struct platform_driver *drv, struct imapx200_uart_info *uart);
//extern int imapx200_serial_init(struct platform_driver *drv, struct imapx200_uart_info *info);

#ifdef CONFIG_SERIAL_IMAP_CONSOLE

#define imapx200_console_init(__drv, __inf)			\
static int __init imapx200_serial_console_init(void)			\
{													\
	return imapx200_serial_initconsole(__drv, __inf);	\
}													\
													\
console_initcall(imapx200_serial_console_init)

#else
#define imapx200_console_init(drv, inf)	extern void no_console(void)
#endif

#ifdef CONFIG_SERIAL_IMAP_DEBUG

extern void printascii(const char *);

static void dbg(const char *fmt, ...)
{
	va_list va;
	char buff[256];

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	printascii(buff);
}

#else
#define dbg(x...)	do { } while (0)
#endif

#endif

