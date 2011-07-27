/***************************************************************************** 
 * ** linux/drivers/i2c/busses/i2c-imapx200.c 
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: Main file of I2C Bus driver.
 * **
 * ** Author:
 * **     Alex Zhang   <alex.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.1  01/08/2010  Alex Zhang   
 * *****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>

#include <asm/irq.h>
#include <asm/io.h>

#include <plat/imapx.h>

#define IMAP_IIC_DEBUG

#define IMAPX200_IIC_BUS_NUM		2

#ifdef IMAP_IIC_DEBUG
#define printk(format, arg...)		\
	printk(KERN_INFO format, ##arg)	
#else
#define printk	do { \
				} while (0)	
			
#endif

enum imapx200_i2c_state
{
	IIC_CUR_IDLE,
	IIC_CUR_READ,
	IIC_CUR_WRITE
};

static g_err = 0;
static struct mutex i2c1_mutex;

struct imapx200_i2c {
	spinlock_t		lock;
	wait_queue_head_t	wait;
	unsigned int		suspended:1;

	struct i2c_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;

	unsigned short		CurReadCount;
	unsigned short 		CurWriteCount;
	unsigned short 		CurCmdCount;
	unsigned char		*pCurWrite;
	unsigned char		*pCurRead;

	unsigned int		tx_setup;
	unsigned int		irq;
	unsigned int 		abort;

	enum imapx200_i2c_state	state;
	unsigned long		clkrate;
	unsigned int		dirty_bit:1;
	unsigned int		MasterArbitration:1;

	void __iomem		*base_reg;
	struct clk		*clk;
	struct device		*dev;
	struct resource		*ioarea;
	struct i2c_adapter	adap;
};

static void imapx200_i2c1_recover()
{
	int scl0,sda0 ,i,ret;
	
	scl0 = gpio_get_value(IMAPX200_GPC(2));
	sda0 = gpio_get_value(IMAPX200_GPC(3));

	if( 1 == sda0 ) {
		printk("imapx200_i2c1_recover i2c1 : sda is high ,return\r\n");
		return;
	}

	for( i == 0; i < 10; i++ ) {
		// config scl0 to output
		// gpio_direction_output(IMAPX200_GPC(2),1);
		// change to gpio
		imapx200_gpio_cfgpin(IMAPX200_GPC(2),1);
	
		gpio_set_value(IMAPX200_GPC(2),0);
		udelay(10);
		gpio_set_value(IMAPX200_GPC(2),1);
		udelay(10);
		gpio_set_value(IMAPX200_GPC(2),0);
		udelay(10);
		gpio_set_value(IMAPX200_GPC(2),1);
		udelay(10);

		// change to scl0
		imapx200_gpio_cfgpin(IMAPX200_GPC(2),2);
		udelay(2000); // msleep(1);
		sda0 = gpio_get_value(IMAPX200_GPC(3));
		if( 1 == sda0 ) {
			printk("imapx200_i2c1_recover i2c1 is successed!\r\n");
			break;
		}
	}	
}

static int imapx200_enable_errinterrupt(struct imapx200_i2c *i2c,int enable)
{
    u32 val;

	val = __raw_readl(i2c->base_reg + rIC_INTR_MASK);
	
    if( enable ) {
        val |= INT_TX_ABORT|INT_TX_OVER|INT_RX_OVER|INT_RX_UNDER;
    }else{
        val &= ~(INT_TX_ABORT|INT_TX_OVER|INT_RX_OVER|INT_RX_UNDER);
    }
	
	__raw_writel(val,i2c->base_reg + rIC_INTR_MASK);

    return 1;

}
static int imapx200_enable_xmitinterrupt(struct imapx200_i2c *i2c,int enable)
{
    u32 val;

	val = __raw_readl(i2c->base_reg + rIC_INTR_MASK);
	
    if( enable ) {
        val |= INT_STOP_DET;
    }else{
        val &= ~(INT_STOP_DET);
    }
	
	__raw_writel(val,i2c->base_reg + rIC_INTR_MASK);

    return 1;

}

static int imapx200_enable_recvinterrupt(struct imapx200_i2c *i2c,int enable)
{
    u32 val;

	val = __raw_readl(i2c->base_reg + rIC_INTR_MASK);
	
    if( enable ) {
        val |= INT_RX_DONE|INT_RX_FULL;
    }else{
        val &= ~(INT_RX_DONE|INT_RX_FULL);
    }
	
	__raw_writel(val,i2c->base_reg + rIC_INTR_MASK);

    return 1;	
}


/*imapx200_init_register
 *
 *initilize registers for setting up iic bus
 */
static int imapx200_i2c_init_register(struct imapx200_i2c *i2c)
{
	int retry = 3;
	unsigned int addr = i2c->msg->addr;

	if(i2c->dirty_bit == 1)
	{
		//printk("[IMAPX200_IIC] Initialization registers\n");
		
		__raw_writel(~IMAPX200_IIC_ENABLE, i2c->base_reg + rIC_ENABLE);

		while(1)
		{
			if(!(__raw_readl(i2c->base_reg + rIC_ENABLE_STATUS) & IMAPX200_IIC_ENABLE))
			{
				break;
			}
				
			retry--;

			if(retry <= 0)
			{
				dev_err(i2c->dev, "[IMAPX200_IIC] IIC device cannot be enable\n");
				return -1;
			}
			
			//msleep(100);
			//udelay(3000);
		}

		__raw_writel(0,i2c->base_reg + rIC_CON);
		__raw_writel(addr, i2c->base_reg + rIC_TAR);
#ifndef	CONFIG_TOUCHSCREEN_NAS	
		__raw_writel(30, i2c->base_reg + rIC_SS_SCL_HCNT); // 100
		__raw_writel(30, i2c->base_reg + rIC_SS_SCL_LCNT); // 100
		__raw_writel(0, i2c->base_reg + rIC_RX_TL);
		__raw_writel(0, i2c->base_reg + rIC_TX_TL);
		__raw_writel((15 & 0xff), i2c->base_reg + rIC_SDA_CFG0); // 0x20
#else
		__raw_writel(100, i2c->base_reg + rIC_SS_SCL_HCNT); // 100
		__raw_writel(100, i2c->base_reg + rIC_SS_SCL_LCNT); // 100
		__raw_writel(0, i2c->base_reg + rIC_RX_TL);
		__raw_writel(0, i2c->base_reg + rIC_TX_TL);
		__raw_writel((50 & 0xff), i2c->base_reg + rIC_SDA_CFG0); // 15 25
#endif		
		__raw_writel((IMAPX200_STANDARD_SPEED | IMAPX200_MATER_MODE | IMAPX200_SLAVE_DISABLE | IMAPX200_RESTART_ENABLE), 
					i2c->base_reg + rIC_CON);
		__raw_writel(0, i2c->base_reg + rIC_INTR_MASK);
		__raw_writel(IMAPX200_IIC_ENABLE, i2c->base_reg + rIC_ENABLE);

		imapx200_enable_errinterrupt(i2c,1);		

		i2c->dirty_bit = 0;

		i2c->state = IIC_CUR_IDLE;
		//printk("[IMAPX200_IIC] Initialization registers is OK\n");

	}

	return 0;
}

static void imapx200_i2c_read_in(struct imapx200_i2c *i2c)
{
	int  txFifoEmptyNum, readCmdNum;	
	int i;

	while(__raw_readl(i2c->base_reg + rIC_RXFLR))
	{
		__raw_readl(i2c->base_reg + rIC_DATA_CMD);	//read dummy data
	}

	__raw_readl(i2c->base_reg + rIC_CLR_INTR); // clear interrupt


	//txFifoEmptyNum = IMAPX200_IIC_TX_FIFO_DEPTH - __raw_readl(i2c->base_reg + rIC_TXFLR);
	//readCmdNum = i2c->CurCmdCount > txFifoEmptyNum ? txFifoEmptyNum : i2c->CurCmdCount;
	readCmdNum = i2c->CurReadCount-i2c->CurCmdCount;
	if( readCmdNum > 16 )
		readCmdNum = 16;
	
	__raw_writel(readCmdNum-1 , i2c->base_reg + rIC_RX_TL);

	imapx200_enable_recvinterrupt(i2c,1);
	for(i = readCmdNum; i > 0; i--)
	{   
		__raw_writel(IMAPX200_IIC_READ_CMD, i2c->base_reg + rIC_DATA_CMD);
	} 

	//__raw_writel(__raw_readl(i2c->base_reg + rIC_INTR_MASK) | IMAPX200_IIC_RX_FULL, i2c->base_reg + rIC_INTR_MASK);

}


/*imapx200_i2c_read
 *
 *iic bus data read 
 */
static void imapx200_i2c_read(struct imapx200_i2c *i2c,
				struct i2c_msg *msg)
{
	int  txFifoEmptyNum, readCmdNum;	
	int i;

	i2c->CurReadCount = msg->len;
	i2c->CurCmdCount = 0;
	i2c->pCurRead = msg->buf;

	imapx200_i2c_read_in(i2c);

}

/*imapx200_i2c_write
 *
 *iic bus data write
 */
static int imapx200_i2c_write(struct imapx200_i2c *i2c,
				struct i2c_msg *msg)
{
	int val,written = 0,i;
	int freespace;
	int txFifoEmptyNum, writeNum; 
	unsigned char byte;

	__raw_readl(i2c->base_reg + rIC_CLR_INTR); // clear interrupt
	
	i2c->CurWriteCount = msg->len;
	i2c->pCurWrite	= msg->buf;

	txFifoEmptyNum = IMAPX200_IIC_TX_FIFO_DEPTH - (__raw_readl(i2c->base_reg + rIC_TXFLR));
	writeNum = i2c->CurWriteCount > txFifoEmptyNum ? txFifoEmptyNum : i2c->CurWriteCount;

	imapx200_enable_xmitinterrupt(i2c,1);
	for(i = 0; i < writeNum; i++)
	{
		byte = *i2c->pCurWrite++;
		__raw_writel((byte & 0xff), i2c->base_reg + rIC_DATA_CMD);
		i2c->CurWriteCount--;
	}
	
	return 0;
}

/* imapx200_i2c_message_start
 *
 * put the start of a message onto the bus
*/

static void imapx200_i2c_message_start(struct imapx200_i2c *i2c,
				      struct i2c_msg *msg)
{


	//if(i2c->state == IIC_CUR_IDLE)
	{
		if(i2c->dirty_bit == 1)
		{
			imapx200_i2c_init_register(i2c);
		}
		if(msg->flags & I2C_M_RD)
		{
			i2c->state = IIC_CUR_READ;
			imapx200_i2c_read(i2c,msg);
		}
		else
		{
			i2c->state = IIC_CUR_WRITE;
			imapx200_i2c_write(i2c,msg);
		}
	}
	/* delay here to ensure the data byte has gotten onto the bus
	 * before the transaction is started */

	ndelay(i2c->tx_setup);
}

/* imapx200_i2c_set_master
 *
 * get the i2c bus for a master transaction
*/

static int imapx200_i2c_set_master(struct imapx200_i2c *i2c)
{
	unsigned long iicstat;
	int timeout = 400;

	while (timeout-- > 0) {
		iicstat = readl(i2c->base_reg + rIC_TX_ABRT_SOURCE);

		if (!(iicstat & IMAPX200_IIC_LOST_ARBITRATION))
		{
			//printk("imapx200_i2c_set_master : MasterArbitration\r\n");
			i2c->MasterArbitration = 1;
			return 0;
		}
		//msleep(1);
		udelay(1000);
	}
	
	i2c->MasterArbitration = 0;
	imapx200_i2c_init_register(i2c);	
	return -ETIMEDOUT;
}

/* imapx200_i2c_doxfer
 *
 * this starts an i2c transfer
*/
static int i2c_addr_old = 0;
static int imapx200_i2c_doxfer(struct imapx200_i2c *i2c,
			      struct i2c_msg *msgs, int num)
{
	unsigned long timeout;
	int i;
	int ret;

	if (i2c->suspended)
		return -EIO;
	
	ret = imapx200_i2c_set_master(i2c);
	if (ret != 0) {
		dev_err(i2c->dev, "cannot get bus (error %d)\n", ret);
		ret = -EAGAIN;
		goto out;
	}

	//spin_lock_irq(&i2c->lock);

	// if the i2c addr is diffirent from before , then set dirty to 1
	if(i2c_addr_old !=  msgs->addr ) {
		i2c->dirty_bit = 1;
	}
	
	i2c->msg_idx = 0;
	i2c->msg_num = num;
	i2c->abort=0;
	for(i = 0; i < num; i++)
	{
		i2c->msg     = &msgs[i];
		i2c->msg_ptr = 0;
		//printk("imapx200_i2c_message_start,num=%d\r\n",num);
		imapx200_i2c_message_start(i2c, &msgs[i]);
		
	}
	//spin_unlock_irq(&i2c->lock);
	
	//timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, HZ * 5);
	
	timeout = wait_event_interruptible_timeout(i2c->wait, i2c->msg_num == 0, HZ);
	//printk("imapx200_i2c_message_start ,timeout=%d\r\n",timeout);
	ret = i2c->msg_idx;
	i2c_addr_old = msgs->addr;
	/* having these next two as dev_err() makes life very
	 * noisy when doing an i2cdetect */
	
	if( 1 == i2c->abort ) {
		printk("i2c1 : recover\r\n");
		imapx200_i2c1_recover();
		i2c->abort=0;
		i2c->dirty_bit = 1;
		if(i2c->msg)
			imapx200_i2c_init_register(i2c);
		i2c->dirty_bit = 0;
		return 0;
	}
	
	if (timeout == 0) {
		printk("timeout\n");
		imapx200_i2c1_recover();
		i2c->abort=0;
		i2c->dirty_bit = 1;
		if(i2c->msg)
			imapx200_i2c_init_register(i2c);
		i2c->dirty_bit = 0;		
	}
	else if (ret != num)
		printk("incomplete xfer (%d)\n", ret);

	/* ensure the stop has been through the bus */

 out:
	return ret;
}

/* imapx200_i2c_xfer
*
* first port of call from the i2c bus code when an message needs
* transferring across the i2c bus.
*/

static int imapx200_i2c_xfer(struct i2c_adapter *adap,
		       struct i2c_msg *msgs, int num)
{
       struct imapx200_i2c *i2c = (struct imapx200_i2c *)adap->algo_data;
       int retry;
       int ret;

       for (retry = 0; retry < adap->retries; retry++) {
			
			//mutex_lock(&i2c1_mutex);
	       ret = imapx200_i2c_doxfer(i2c, msgs, num);
			//mutex_unlock(&i2c1_mutex);
	       if (ret != -EAGAIN)
		       return ret;

	       dev_dbg(i2c->dev, "Retrying transmission (%d)\n", retry);

	       udelay(100);
       }
		
       return -EREMOTEIO;
}
/* declare our i2c functionality */
static u32 imapx200_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}

/* i2c bus registration info */

static const struct i2c_algorithm imapx200_i2c_algorithm = {
	.master_xfer		= imapx200_i2c_xfer,
	.functionality		= imapx200_i2c_func,
};


//extern u8 cp2007_buf[2];

/* imapx200_i2c_irq
 *
 * top level IRQ servicing routine
*/
static irqreturn_t imapx200_i2c_irq(int irqno, void *dev_id)
{
	struct imapx200_i2c *i2c = dev_id;
	unsigned char byte;
	int txFifoEmptyNum, writeNum;
	int rx_level, readNum;
	int i = 0;
	u32 status,mask,irq;
	unsigned long flags;
	
	//spin_lock_irqsave(&i2c->lock,flags);
		

	status = __raw_readl(i2c->base_reg + rIC_RAW_INTR_STAT);
	mask   = __raw_readl(i2c->base_reg + rIC_INTR_MASK);

	irq = status & mask;
	//imapx200_enable_recvinterrupt(i2c,0);
	//imapx200_enable_xmitinterrupt(i2c,0);
	
	if(irq & ( INT_RX_UNDER | INT_RX_OVER ) ) {
		__raw_readl(i2c->base_reg + rIC_CLR_INTR); //clr all status
		printk("imapx200_i2c_irq i2c1 : irq = 0x%x,status = 0x%x\r\n",irq,status);
		__raw_readl(i2c->base_reg + rIC_CLR_RX_UNDER);	
		__raw_readl(i2c->base_reg + rIC_CLR_RX_OVER);
		i2c->abort=1;
		
		wake_up_interruptible(&i2c->wait);
	}
	else if( irq & ( INT_TX_OVER | INT_TX_ABORT ) ) {
		unsigned int abort = __raw_readl(i2c->base_reg + rIC_TX_ABRT_SOURCE);
		printk("imapx200_i2c_irq i2c1 : irq = 0x%x,status = 0x%x ,abort is 0x%x\r\n",irq,status,abort);
		__raw_readl(i2c->base_reg + rIC_CLR_INTR); //clr all status
		
		__raw_readl(i2c->base_reg + rIC_CLR_TX_OVER);	
		__raw_readl(i2c->base_reg + rIC_CLR_TX_ABRT);
		i2c->abort=1;
		
		/*if(abort & IMAPX200_IIC_LOST_ARBITRATION){
			i2c->MasterArbitration = 0;
			
			__raw_writel(~(IMAPX200_IIC_RX_FULL | IMAPX200_IIC_TX_EMPTY), i2c->base_reg + rIC_INTR_MASK);
			
			printk("[IMAPX200_IIC: ERR] IIC Bus lost arbitration\n");

			i2c->abort=1;
			i2c->dirty_bit = 1;
			wake_up_interruptible(&i2c->wait);
		}else */
		{
			txFifoEmptyNum = IMAPX200_IIC_TX_FIFO_DEPTH - (__raw_readl(i2c->base_reg + rIC_TXFLR));
			writeNum = i2c->CurWriteCount > txFifoEmptyNum ? txFifoEmptyNum : i2c->CurWriteCount;
			
			// 如果没数据的话，设abort为1，并直接返回，如果有数据继续读
			if(writeNum > 0) {
			
				for(i = 0; i < writeNum; i++)
				{
					byte = *i2c->pCurWrite++;
					__raw_writel((byte & 0xff), i2c->base_reg + rIC_DATA_CMD);
					i2c->CurWriteCount--;
				}
				
				if(i2c->CurWriteCount <= 0)
				{
					i2c->state = IIC_CUR_IDLE;
					i2c->msg_ptr = 0;
					i2c->msg = NULL;
					i2c->msg_idx++;
					i2c->msg_num--;
					imapx200_enable_xmitinterrupt(i2c,0);
					wake_up_interruptible(&i2c->wait);
				}
			}else {
				i2c->abort=1;
				printk("imapx200_i2c_irq 2 : writeNum = 0x%x\r\n",writeNum);
				i2c->state = IIC_CUR_IDLE;
				i2c->msg_ptr = 0;
				i2c->msg = NULL;
				i2c->msg_idx++;
				i2c->msg_num--;
				wake_up_interruptible(&i2c->wait);
			}
		}
		
	}
	else if( irq & ( INT_RX_FULL | INT_RX_DONE ) ) {
		__raw_readl(i2c->base_reg + rIC_CLR_INTR); //clr all status
		__raw_readl(i2c->base_reg + rIC_CLR_RX_DONE); //clr all status
		
		rx_level = __raw_readl(i2c->base_reg + rIC_RXFLR);

		readNum = rx_level;/*i2c->CurReadCount > rx_level ? rx_level : i2c->CurReadCount;*/
		
		for(i = readNum; i > 0; i--)
		{
			if(i2c->CurReadCount == i2c->CurCmdCount)
				break;
			
			byte = __raw_readl(i2c->base_reg + rIC_DATA_CMD);
			*(i2c->pCurRead) = (byte & 0xff);
			i2c->pCurRead++;
			i2c->CurCmdCount++;
		}
		//printk("1  readNum:0x%x,CurReadCount=0x%x,CurCmdCount=0x%x\r\n",readNum,i2c->CurReadCount,i2c->CurCmdCount);
		if(i2c->CurReadCount == i2c->CurCmdCount){
			i2c->state = IIC_CUR_IDLE;
			i2c->msg_ptr = 0;
			i2c->msg = NULL;
			i2c->msg_idx++;
			i2c->msg_num--;
			imapx200_enable_recvinterrupt(i2c,0);
			wake_up_interruptible(&i2c->wait);
		}else{
			imapx200_i2c_read_in(i2c);
		}
	}
	else if( irq &  INT_STOP_DET ) {
		__raw_readl(i2c->base_reg + rIC_CLR_INTR); //clr all status
		__raw_readl(i2c->base_reg + rIC_CLR_STOP_DET); //clr all status
		
		txFifoEmptyNum = IMAPX200_IIC_TX_FIFO_DEPTH - (__raw_readl(i2c->base_reg + rIC_TXFLR));
		writeNum = i2c->CurWriteCount > txFifoEmptyNum ? txFifoEmptyNum : i2c->CurWriteCount;

		 for(i = 0; i < writeNum; i++)
		 {
			byte = *i2c->pCurWrite++;
		 	__raw_writel((byte & 0xff), i2c->base_reg + rIC_DATA_CMD);
			i2c->CurWriteCount--;
		 }

		 if(i2c->CurWriteCount <= 0)
		 {
			i2c->state = IIC_CUR_IDLE;
			i2c->msg_ptr = 0;
			i2c->msg = NULL;
			i2c->msg_idx++;
			i2c->msg_num--;
			imapx200_enable_xmitinterrupt(i2c,0);
			wake_up_interruptible(&i2c->wait);
		 }
		 
	}
	else {
		__raw_readl(i2c->base_reg + rIC_CLR_INTR); //clr all status
		__raw_readl(i2c->base_reg + rIC_CLR_TX_OVER);	
		__raw_readl(i2c->base_reg + rIC_CLR_TX_ABRT);		
		__raw_readl(i2c->base_reg + rIC_CLR_RX_UNDER);	
		__raw_readl(i2c->base_reg + rIC_CLR_RX_OVER);		
		//do something other
		__raw_readl(i2c->base_reg + rIC_CLR_RX_DONE); //clr all status	
		__raw_readl(i2c->base_reg + rIC_CLR_GEN_CALL);
		__raw_readl(i2c->base_reg + rIC_CLR_START_DET);
		__raw_readl(i2c->base_reg + rIC_CLR_STOP_DET);
		__raw_readl(i2c->base_reg + rIC_CLR_ACTIVITY);
		__raw_readl(i2c->base_reg + rIC_CLR_RD_REQ);
		i2c->abort=1;
		//printk("hehehe\r\n");
		wake_up_interruptible(&i2c->wait);		
	}

	//spin_unlock_irqrestore(&i2c->lock,flags);
	return IRQ_HANDLED;
}

/* imapx200_i2c_init
 *
 * initialise the controller, set the IO lines and other SFRs
*/

static int imapx200_i2c_init(struct imapx200_i2c *i2c)
{
	int scl0,sda0 ,i,ret;
	i2c->dirty_bit = 1;
		
	// config i2c1
	imapx200_gpio_cfgpin(IMAPX200_GPC(2),2);
	imapx200_gpio_cfgpin(IMAPX200_GPC(3),2);

	return 0;
}

/* imapx200_i2c_probe
 *
 * called by the bus driver when a suitable device is found
*/

static int imapx200_i2c_probe(struct platform_device *pdev)
{
	struct imapx200_i2c *i2c;
	struct resource *res;
	int ret;

	printk(KERN_INFO "Entering 2 %s\n",__func__);

	i2c = kzalloc(sizeof(struct imapx200_i2c), GFP_KERNEL);
	if (!i2c) {
		dev_err(&pdev->dev, "no memory for state\n");
		return -ENOMEM;
	}

	mutex_init(&i2c1_mutex);
	strlcpy(i2c->adap.name, "imapx200-iic1", sizeof(i2c->adap.name));
	i2c->adap.owner   = THIS_MODULE;
	i2c->adap.algo    = &imapx200_i2c_algorithm;
	i2c->adap.retries = 2;
	i2c->adap.class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	i2c->tx_setup     = 50;

	spin_lock_init(&i2c->lock);
	init_waitqueue_head(&i2c->wait);

	/* find the clock and enable it */

	i2c->dev = &pdev->dev;
	i2c->clk = clk_get(NULL, "i2c1");
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		ret = -ENOENT;
		goto err_noclk;
	}

	dev_dbg(&pdev->dev, "clock source %p\n", i2c->clk);

	clk_enable(i2c->clk);

	/* map the registers */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "cannot find IO resource\n");
		ret = -ENOENT;
		goto err_clk;
	}

	i2c->ioarea = request_mem_region(res->start, resource_size(res),
					 pdev->name);

	if (i2c->ioarea == NULL) {
		dev_err(&pdev->dev, "cannot request IO\n");
		ret = -ENXIO;
		goto err_clk;
	}

	i2c->base_reg = ioremap(res->start, resource_size(res));

	if (i2c->base_reg == NULL) {
		dev_err(&pdev->dev, "cannot map IO\n");
		ret = -ENXIO;
		goto err_ioarea;
	}

	dev_dbg(&pdev->dev, "registers %p (%p, %p)\n",
		i2c->base_reg, i2c->ioarea, res);

	/* setup info block for the i2c core */

	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;

	/* initialise the i2c controller */
	ret = gpio_request(IMAPX200_GPC(2),"i2c scl1");
	if (ret) {
		printk("imapx200_i2c1_recover : i2c scl1 is failed\r\n");
		return;
	}
	ret = gpio_request(IMAPX200_GPC(3),"i2c sda1");
	if (ret) {
		printk("imapx200_i2c1_recover : i2c sda1 is failed\r\n");
		return;
	}
	ret = imapx200_i2c_init(i2c);
	if (ret != 0)
		goto err_iomap;

	/* find the IRQ for this unit (note, this relies on the init call to
	 * ensure no current IRQs pending
	 */

	i2c->irq = ret = platform_get_irq(pdev, 0);
	if (ret <= 0) {
		dev_err(&pdev->dev, "cannot find IRQ\n");
		goto err_iomap;
	}

	ret = request_irq(i2c->irq, imapx200_i2c_irq, IRQF_DISABLED,
			  dev_name(&pdev->dev), i2c);

	if (ret != 0) {
		dev_err(&pdev->dev, "cannot claim IRQ %d\n", i2c->irq);
		goto err_iomap;
	}

	/* Note, previous versions of the driver used i2c_add_adapter()
	 * to add the bus at any number. We now pass the bus number via
	 * the platform data, so if unset it will now default to always
	 * being bus 0.
	 */

	i2c->adap.nr = IMAPX200_IIC_BUS_NUM;

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add bus to i2c core\n");
		return ret;
	}

	platform_set_drvdata(pdev, i2c);
	
//	printk(KERN_INFO "leaving %s\n",__func__);
	dev_info(&pdev->dev, "%s: IMAPX200 I2C adapter\n", dev_name(&i2c->adap.dev));
	return 0;

 err_irq:
	free_irq(i2c->irq, i2c);

 err_iomap:
	iounmap(i2c->base_reg);

 err_ioarea:
	release_resource(i2c->ioarea);
	kfree(i2c->ioarea);

 err_clk:
	clk_disable(i2c->clk);
	clk_put(i2c->clk);

 err_noclk:
	kfree(i2c);
	return ret;
}


/*imapx200_i2c_remove
 *
 *called when device is removed form bus
 */
  
static int imapx200_i2c_remove(struct platform_device *pdev)
{
	struct imapx200_i2c *i2c = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c->adap);
	free_irq(i2c->irq, i2c);

	clk_disable(i2c->clk);
	clk_put(i2c->clk);

	iounmap(i2c->base_reg);

	release_resource(i2c->ioarea);
	kfree(i2c->ioarea);
	kfree(i2c);

	return 0;
}

#ifdef CONFIG_PM
static int imapx200_i2c_suspend_late(struct platform_device *dev,
				    pm_message_t msg)
{
	struct imapx200_i2c *i2c = platform_get_drvdata(dev);
	i2c->suspended = 1;
	return 0;
}

static int imapx200_i2c_resume(struct platform_device *dev)
{
	struct imapx200_i2c *i2c = platform_get_drvdata(dev);

	i2c->suspended = 0;
	imapx200_i2c_init(i2c);

	return 0;
}

#else
#define imapx200_i2c_suspend_late NULL
#define imapx200_i2c_resume NULL
#endif

static struct platform_driver imapx200_i2c_driver = {
	.probe		= imapx200_i2c_probe,
	.remove		= imapx200_i2c_remove,
	.suspend	= imapx200_i2c_suspend_late,
	.resume		= imapx200_i2c_resume,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "imapx200_iic1",
	},
};

static int __init imapx200_i2c_adap_init(void)
{
	return platform_driver_register(&imapx200_i2c_driver);
}
subsys_initcall(imapx200_i2c_adap_init);

static void __exit imapx200_i2c_adap_exit(void)
{
	platform_driver_unregister(&imapx200_i2c_driver);
}
module_exit(imapx200_i2c_adap_exit);

MODULE_DESCRIPTION("IMAPX200 I2C Bus driver");
MODULE_AUTHOR("Alex Zhang, <alex.zhang@infotmic.com.cn>");
MODULE_LICENSE("GPL");
