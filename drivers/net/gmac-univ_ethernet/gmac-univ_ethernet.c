/* **************************************************************************** 
 * ** gmac-univ_ethernet.c
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** This program is free software; you can redistribute it and/or modify
 * ** it under the terms of the GNU General Public License as published by
 * ** the Free Software Foundation; either version 2 of the License, or
 * ** (at your option) any later version.
 * ** 
 * ** Description:This c file is the driver for DesignWare Cores Ethernet 
 * ** MAC Universal(GMAC-UNIV for short). 
 * **
 * ** Author:
 * **     Bob.yang<bob.yang@mail.infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.0  12/17/2009 Bob.yang
 * *****************************************************************************/

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/dma-mapping.h>

#include <asm/delay.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>

#include <plat/imapx.h>
#include <plat/mem_reserve.h>

#include "gmac-univ_ethernet.h"

/*board information definition*/
#define CARDNAME		"gmac-univ"
#define DRV_VERSION		"1.0"

/* All four parameters below is physical addresses because they are used by DMA controller */
unsigned long ETH_BASE_TXDES_PHY, ETH_BASE_TXBUF_PHY, ETH_BASE_RXDES_PHY, ETH_BASE_RXBUF_PHY;
volatile unsigned int *pt_to_last_edge_rxdesc;

#if TWOTXFRAMES
int txdes_tail, txdes_head;
#endif

/*  Transmit timeout, default 6 seconds. */
static int watchdog = 6000;
module_param(watchdog, int, 0400);
MODULE_PARM_DESC(watchdog, "transmit timeout in milliseconds");

/* gmac board infomation structure declaration */
typedef struct board_info {
	void __iomem	*io_addr;	//Register I/O base address
	struct resource *addr_req;	//resources requested

	/* These four parameters will be explained in the gmac_probe function */
	volatile unsigned int *eth_base_txdes_vir;
	volatile unsigned int *eth_base_rxdes_vir;
	volatile unsigned int *eth_base_txbuf_vir;
	volatile unsigned int *eth_base_rxbuf_vir;

	u16 tx_pkt_cnt;
	int debug_level;	

	unsigned int flags;		//used for hashtable
	unsigned int in_suspend:1;	//flag for suspending

	struct device *dev;		//parent device

	struct mutex addr_lock;		//phy access lock

	struct delayed_work phy_poll;	//used for polling work
#if GMAC_TASK_IRQ
	struct tasklet_struct my_tasklet;
#endif
	struct net_device *ndev;

	struct mii_if_info mii;		//use the universal MII layer in this driver
	uint32_t msg_enable;		//only used in ethtool_ops functions
} board_info_t;

/* debug code */
#define gmac_dbg(db, msg...) do\
{\
	if (db->debug_level > 0)\
	{\
		dev_info(db->dev, msg);\
	}\
} while (0)

static inline board_info_t *to_gmac_board(struct net_device *dev)
{
	return netdev_priv(dev);
}

/*****************************************************************************
 * ** -Function:
 * **   gmac_cre_def_tx_des(volatile unsigned int *tx_des_base_addr)
 * **
 * ** -Description:
 * **   This function create NO_OF_TXDES linked default Tx descriptors, and they form a ring
 * **
 * ** -Input Param
 * **	tx_des_base_addr: point to the physical base address of memory allocated for
 * **	Tx descriptors
 * **
 * ** -Return
 * **   NULL 
 * *****************************************************************************/
void gmac_cre_def_tx_des(volatile unsigned int *tx_des_base_addr)
{
	int i = 0;
	volatile unsigned int *pointto_cur_des;
	
	pointto_cur_des = tx_des_base_addr;

	for (i=0; i<NO_OF_TXDES; i++)
	{
		writel(0, pointto_cur_des);	//at first the Tx descriptor is not owned by DMA control
		writel(IC | CIC | DC_T | TCH | DP | TTSE | TBS2, pointto_cur_des + 1);		//Config the TxDES1
		writel(ETH_BASE_TXDES_PHY+(i+1)*SINGLE_TXDES_SIZE, pointto_cur_des + 3);		//Next Tx descriptor address
		pointto_cur_des += 4;
	}
	pointto_cur_des -= 4;
	writel(ETH_BASE_TXDES_PHY, pointto_cur_des + 3);	//Next Tx descriptor address of last descriptor is the first one 
}

/*****************************************************************************
 * ** -Function:
 * **   gmac_cre_def_rx_des(volatile unsigned int *rx_des_base_addr)
 * **
 * ** -Description:
 * **   This function create NO_OF_RXDES linked default Rx descriptors, and they form a ring
 * **
 * ** -Input Param
 * **	rx_des_base_addr: point to the physical base address of memory allocated for
 * **	Rx descriptors
 * **
 * ** -Return
 * **   NULL 
 * *****************************************************************************/
void gmac_cre_def_rx_des(volatile unsigned int *rx_des_base_addr)
{

	int i = 0;
	volatile unsigned int *pointto_cur_des;

	pointto_cur_des = rx_des_base_addr;

	for (i=0; i<NO_OF_RXDES; i++)
	{
		writel(TXOWN, pointto_cur_des);	//at first the Rx descriptor is owned by DMA control
		writel(DINT | RCH | RBS2 | RBS1, pointto_cur_des + 1);		//Config the RxDES1
		writel(ETH_BASE_RXBUF_PHY+i*RBS1, pointto_cur_des + 2);		//Physical address for Rx buffer
		writel(ETH_BASE_RXDES_PHY+(i+1)*SINGLE_RXDES_SIZE, pointto_cur_des + 3);	//Next Rx descriptor address
		pointto_cur_des += 4;
	}
	pointto_cur_des -= 4;														
	writel(ETH_BASE_RXDES_PHY, pointto_cur_des + 3);	////Next Rx descriptor address of last descriptor is the first one 
}

/*****************************************************************************
 * ** -Function:
 * **   gmac_cre_cur_tx_des(struct board_info *db, uint32_t *pdata_phyaddr, uint32_t len, 
 * **	volatile unsigned int *tx_des_base_addr)
 * **
 * ** -Description:
 * **   This function put the Tx frame into current descriptor(only one des is enough
 * **	for one frame).
 * **
 * ** -Input Param
 * **	db: point to the struct board_info
 * **	pdata_phyaddr: point to the physical address of the Tx frame
 * **	len: length of Tx frame
 * **	tx_des_base_addr: point to the physical base address of memory allocated for
 * **	Tx descriptors
 * **
 * ** -Return
 * **   NULL 
 * *****************************************************************************/
void gmac_cre_cur_tx_des(struct board_info *db, uint32_t *pdata_phyaddr, uint32_t len, volatile unsigned int *tx_des_base_addr)
{
#if TWOTXFRAMES
	volatile unsigned int *pointto_cur_des, *base;

	/* Got the current Tx descriptor address read by DMA from the CurrHostTxDescriptor register */
	pointto_cur_des = tx_des_base_addr + (readl(db->io_addr + rCurrHostTxDescriptor) - ETH_BASE_TXDES_PHY) /4;

	while (readl(pointto_cur_des) & TXOWN)
	{
		if (*(pointto_cur_des + 3) == ETH_BASE_TXDES_PHY)
		{
			pointto_cur_des -= (NO_OF_TXDES - 1) * 4;
		}
		else
		{
			pointto_cur_des += 4;
		}
	}

	base = pointto_cur_des;
	writel(IC | CIC | DC_T | TCH | DP | TTSE | TBS2, pointto_cur_des + 1);
	writel((int)*pdata_phyaddr , pointto_cur_des + 2);	//set physical address for Tx buffer into TxDES2

	*(pointto_cur_des + 1) |= FS | TXLS | len;     //the first and last segment of a frame is in one descriptor
	*base = TXOWN;
	txdes_head ++;
	if (txdes_head >= NO_OF_TXDES)
	{
		txdes_head = 0;
	}
#else
	volatile unsigned int *pointto_cur_des;

	gmac_dbg(db, "putting the Tx frame into current descriptors\n");

	/* Got the current Tx descriptor address read by DMA from the CurrHostTxDescriptor register */
	pointto_cur_des = tx_des_base_addr + (readl(db->io_addr + rCurrHostTxDescriptor) - ETH_BASE_TXDES_PHY) /4;

	writel(TXOWN, pointto_cur_des);		//this Tx descriptor is owned by DMA control now
	writel(IC | CIC | DC_T | TCH | DP | TTSE | TBS2, pointto_cur_des + 1);
	writel((int)*pdata_phyaddr , pointto_cur_des + 2);	//set physical address for Tx buffer into TxDES2

	*(pointto_cur_des + 1) |= FS | TXLS | len;     //the first and last segment of a frame is in one descriptor
#endif
}

void gmac_enable_irq(struct board_info *db)
{
#if !GMAC_POLL
	writel(NIE | RIE | TIE, db->io_addr + rInterruptEnable);
//	writel(NIE|RIE, db->io_addr + rInterruptEnable);
//	writel(NIE|TIE, db->io_addr + rInterruptEnable);
#endif
}

void gmac_disable_irq(struct board_info *db)
{
	writel(0x0, db->io_addr + rInterruptEnable);		
}

uint32_t gmac_get_irqstatus(struct board_info *db)
{	
#if !GMAC_POLL
	return (readl(db->io_addr + rStatus) & (RI | TI | NORINTE));
//	return (readl(db->io_addr + rStatus) & (RI | NORINTE));
//	return (readl(db->io_addr + rStatus) & (TI | NORINTE));
#else
	return 0;
#endif
}

void gmac_clear_irqstatus(struct board_info *db, uint32_t uValue)
{
	writel(uValue, db->io_addr + rStatus);
}

static void gmac_reset(board_info_t *db)
{
	uint32_t tmp, i = 0;
	dev_dbg(db->dev, "resetting device\n");

	/* reset device */
	tmp = readl(db->io_addr + rBUSMODE);                                            
	tmp |= SWR;
	writel(tmp, db->io_addr + rBUSMODE);         //reset all Gmac registers
	for (i=0; i<DELAY; i++) 
	{                                
		if ((readl(db->io_addr + rBUSMODE) & SWR) == 0)
		{
			break;
		}
	}                                                            
	                                                             
	if (i == DELAY)                                              
	{
		dev_info(db->dev, "phy write timed out\n");          
	}
}

static void gmac_schedule_poll(board_info_t *db)
{
#if GMAC_POLL
	schedule_delayed_work(&db->phy_poll, 1);
#else
	schedule_delayed_work(&db->phy_poll, HZ*5);
#endif
}

static int gmac_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	if (!netif_running(dev))
	{
		return -EINVAL;
	}

	return generic_mii_ioctl(&db->mii, if_mii(req), cmd, NULL);
}

static void gmac_get_drvinfo(struct net_device *dev,struct ethtool_drvinfo *info)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	strcpy(info->driver, CARDNAME);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->bus_info, to_platform_device(db->dev)->name);
}

static uint32_t gmac_get_msglevel(struct net_device *dev)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	return db->msg_enable;
}

static void gmac_set_msglevel(struct net_device *dev, uint32_t value)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	db->msg_enable = value;
}

static int gmac_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	mii_ethtool_gset(&db->mii, cmd);
	return 0;
}

static int gmac_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	return mii_ethtool_sset(&db->mii, cmd);
}

static int gmac_nway_reset(struct net_device *dev)
{
	board_info_t *db = to_gmac_board(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	return mii_nway_restart(&db->mii);
}

static uint32_t gmac_get_link(struct net_device *dev)
{
	board_info_t *db = to_gmac_board(dev);
	uint32_t ret;

	gmac_dbg(db, "entering %s\n", __func__);

	/* check link status */
	ret = mii_link_ok(&db->mii);

	return ret;
}

/* only ethtool tool can call these function */
static const struct ethtool_ops gmac_ethtool_ops = 
{
	.get_drvinfo = gmac_get_drvinfo,
	.get_settings = gmac_get_settings,
	.get_settings = gmac_set_settings,
	.get_msglevel = gmac_get_msglevel,
	.set_msglevel = gmac_set_msglevel,
	.nway_reset	= gmac_nway_reset,
	.get_link = gmac_get_link,
};

/*****************************************************************************
 * ** -Function:
 * **   gmac_rx(struct net_device *dev)
 * **
 * ** -Description:
 * **   This function implement passing all received frames to upper layer
 * **
 * ** -Input Param
 * **	dev: point to the struct net_device whose private pointer is struct board_info_t *
 * **
 * ** -Return
 * **   NULL 
 * *****************************************************************************/
static void gmac_rx(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
	struct sk_buff *skb;
	u8  *rdptr;
	volatile unsigned int *pointto_cur_des;
	uint8_t *pointto_cur_rx_pkt;		//point to the current RX frame
	u16  pkt_len;
	u32 rx_status;
	
	gmac_dbg(db, "entering %s\n", __func__);

	pointto_cur_des = pt_to_last_edge_rxdesc;

	/* find all received frames in these 50 RX descriptors*/
	while (readl(pointto_cur_des) & RXLS)			//the RXLS bit means the end of one received frame
	{
		if (!(readl(pointto_cur_des) & RXFS))
		{
			dev_dbg(db->dev, "The Rx segment is not the first one!\n");
		}

		rx_status = readl(pointto_cur_des);
		if(rx_status & RXES)
		{
			printk(KERN_INFO "Rx error!\n" );
			if (rx_status & RXDE)
			{
				printk(KERN_INFO "Descriptor Error\n");
			}
			if (rx_status & RXOE)
			{
				printk(KERN_INFO "Overflow Error\n");
			}
			if (rx_status & RXGF)
			{
				printk(KERN_INFO "Giant Frame\n");
			}
			if (rx_status & RXLC)
			{
				printk(KERN_INFO "Late Collision\n");
			}
			if (rx_status & RXWT)
			{
				printk(KERN_INFO "Watchdog Timeout\n");
			}
			if (rx_status & RXRE)
			{
				printk(KERN_INFO "Receive Error\n");
			}
			if (rx_status & RXCRCE)
			{
				printk(KERN_INFO "CRC Error\n");
			}
		}
		
		pointto_cur_rx_pkt = (uint8_t *)db->eth_base_rxbuf_vir + (pointto_cur_des - db->eth_base_rxdes_vir)/4*RBS1;
		pkt_len = (readl(pointto_cur_des) >> 16) & 0x3fff;		//the length of the frame

		/* Packet Status check */
		if (pkt_len > MAX_MTU)
		{
			dev_dbg(db->dev, "RST: RX Len:%x\n", pkt_len);
		}

		/* Now move the frame to upper layer*/
		if ((skb = dev_alloc_skb(pkt_len + 2)) != NULL)
		{
			rdptr = (u8 *) skb_put(skb, pkt_len);

			/* Read received frame from Rxbuf */
			memcpy(rdptr, pointto_cur_rx_pkt, pkt_len);

			dev->stats.rx_bytes += pkt_len;

			/* Pass to upper layer */
			skb->protocol = eth_type_trans(skb, dev);
			netif_rx(skb);
			dev->stats.rx_packets++;
		} 

		/* Now the Rx descriptor could be used by DMA again */			
		writel(RXOWN, pointto_cur_des);

		pointto_cur_des += 4;		//goto next RxDes
		if (pointto_cur_des > (db->eth_base_rxdes_vir + (NO_OF_RXDES -1)*4))
		{
			pointto_cur_des = db->eth_base_rxdes_vir;
		}
	}
	
	pt_to_last_edge_rxdesc = pointto_cur_des;
}

static void gmac_poll_work(struct work_struct *w)
{
	struct delayed_work *dw = container_of(w, struct delayed_work, work);
	board_info_t *db = container_of(dw, board_info_t, phy_poll);
	struct net_device *ndev = db->ndev;

	gmac_dbg(db, "entering %s\n", __func__);
	
	//printk("gmac_poll_work \r\n");
	
	mii_check_media(&db->mii, netif_msg_link(db), 0);

	if (netif_running(ndev))
	{
#if GMAC_POLL
		while (readl(db->io_addr + rStatus) & RI)
		{
			writel(RI, db->io_addr + rStatus);
			gmac_rx(ndev);
		}
#endif
		gmac_schedule_poll(db);
	}
}

static void gmac_release_board(struct platform_device *pdev, struct board_info *db)
{
	gmac_dbg(db, "entering %s\n", __func__);

#if GMAC_TASK_IRQ
	tasklet_kill(&db->my_tasklet);
#endif
	/* free dma resources */
	dma_free_coherent(db->dev, 0x200000, db->eth_base_txdes_vir, ETH_BASE_TXDES_PHY);
	
	iounmap(db->io_addr);

	/* release the resource*/
	release_resource(db->addr_req);
	kfree(db->addr_req);
}

/* set gmac multicast address */
static void gmac_hash_table(struct net_device *dev)
{
	board_info_t *db = (board_info_t *)netdev_priv(dev);
	struct dev_mc_list *mcptr = dev->mc_list;
	int mc_cnt = dev->mc_count;
	int i;
	uint32_t hash_val;
	u16 hash_table[4];
	uint32_t rcr = 0;

	gmac_dbg(db, "entering %s\n", __func__);

	writel(dev->dev_addr[5]<<8 | dev->dev_addr[4], db->io_addr + rMACAddr0H);			//mac high addr 
	writel(dev->dev_addr[3]<<24 | dev->dev_addr[2]<<16 | dev->dev_addr[1]<<8 | dev->dev_addr[0],
	   db->io_addr + rMACAddr0L);		//mac low addr 

	/* Clear Hash Table */
	for (i=0; i<4; i++)
		hash_table[i] = 0x0;

	/* broadcast address */
	hash_table[3] = 0x8000;

	if (dev->flags & IFF_PROMISC)
	{
		rcr |= PRM;
	}

	if (dev->flags & IFF_ALLMULTI)
	{
		rcr |= PM;
	}

	/* the multicast address in Hash Table : 64 bits */
	for (i=0; i<mc_cnt; i++, mcptr = mcptr->next)
	{
		hash_val = ether_crc_le(6, mcptr->dmi_addr) & 0x3f;
		hash_table[hash_val / 16] |= (u16) 1 << (hash_val % 16);
	}

	/* Write the hash table to MAC MD table */
	writel(hash_table[3]<<16|hash_table[2], db->io_addr + rHashTableH);
	writel(hash_table[1]<<16|hash_table[0], db->io_addr + rHashTableL);

	writel(rcr, db->io_addr + rMACFrameFilter);
}

/*
 * Read a word from phyxcer
 */
static int gmac_phy_read(struct net_device *dev, int phy_reg_unused, int reg)
{
	board_info_t *db = netdev_priv(dev);
	int ret, i = 0;

	gmac_dbg(db, "entering %s\n", __func__);

//	mutex_lock(&db->addr_lock);
	while (readl(db->io_addr + rGMIIAddr) & PHYBUSY)
	{
		;		//wait for free
	}

	/* Fill the phyxcer register into GMII Address register */
	writel((0x1<<11) | (reg<<6) | (0x1<<3) |(0x0<<1) | PHYBUSY, db->io_addr + rGMIIAddr);
	for (i=0; i<DELAY; i++)
	{
		if ((readl(db->io_addr + rGMIIAddr) & PHYBUSY) == 0)
		{
			break;
		}
	}

	if (i == DELAY)
	{
	    dev_info(db->dev, "phy read timed out\n");
	}
	ret = readl(db->io_addr + rGMIIData);

//	mutex_unlock(&db->addr_lock);
	return ret;
}

/*
 * Write a word to phyxcer
 */
static void gmac_phy_write(struct net_device *dev, int phyaddr_unused, int reg, int value)
{
	board_info_t *db = netdev_priv(dev);
	int i = 0;

	gmac_dbg(db, "entering %s\n", __func__);

//	mutex_lock(&db->addr_lock);
	while (readl(db->io_addr + rGMIIAddr) & PHYBUSY)
	{
		;		//wait for free
	}

	/* Fill the phyxcer register into GMII Address register */
	writel(value, db->io_addr + rGMIIData);
	writel((0x1<<11) | (reg<<6) | (0x1<<3) |(0x1<<1) | PHYBUSY, db->io_addr + rGMIIAddr );
	for (i=0; i<DELAY; i++) 
	{                                
		if ((readl(db->io_addr + rGMIIAddr) & PHYBUSY) == 0)
		{
			break;
		}
	}                                                            
	                                                             
	if (i==DELAY)                                              
	{
		dev_info(db->dev, "phy write timed out\n");          
	}

//	mutex_unlock(&db->addr_lock);
}

/*****************************************************************************
 * ** -Function:
 * **   gmac_init_gmac(struct net_device *dev)
 * **
 * ** -Description:
 * **   This function initilizes gmac board 
 * **
 * ** -Input Param
 * **	dev: point to the struct net_device whose private pointer is struct board_info_t *
 * **
 * ** -Return
 * **   NULL 
 * *****************************************************************************/
static void gmac_init_gmac(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
	uint32_t tmp, eth_speed;
	
	gmac_dbg(db, "entering %s\n", __func__);

//	eth_speed = NORMAL_SPEED;
	eth_speed = FAST_SPEED;
	
	if (eth_speed == NORMAL_SPEED)
	{
		/* only test on fpga */
		gmac_phy_write(dev, 0, MII_ADVERTISE, ADVERTISE_10FULL | ADVERTISE_10HALF);	// support 10M
		gmac_phy_write(dev, 0, MII_BMCR, BMCR_FULLDPLX);			//Full-duplex and autoneg unenabled 
		tmp = gmac_phy_read(dev, 0, MII_BMCR);
		gmac_dbg(db, "MII_BMCR %x\n", tmp);
		tmp = gmac_phy_read(dev, 0, 1);
		gmac_dbg(db, "MII_1 %x\n", tmp);
	}
	else
	{
		if (!(gmac_phy_read(dev, 0, MII_BMCR) & BMCR_ANENABLE))
		{
			gmac_phy_write(dev, 0, MII_BMCR, BMCR_RESET);	//phy reset
		}
	}

	gmac_hash_table(dev);

	/* config the DMA BUSMODE register */
	//writel(AAL|FBPL|USP|RXPBL|FB|PBL|DSL|DA, db->io_addr + rBUSMODE);	
	writel(AAL|FBPL|FB|PBL|DSL, db->io_addr + rBUSMODE);	

	/* config the DMA TxDescriptorListAddr and RxDescriptorListAddr register */
	writel(ETH_BASE_TXDES_PHY, db->io_addr + rTxDescriptorListAddr);
	writel(ETH_BASE_RXDES_PHY, db->io_addr + rRxDescriptorListAddr);	

	/* config the DMA OperationMode register */
	//writel(DFF | FEF | RFA | RSF, db->io_addr + rOperationMode); 			
#if TWOTXFRAMES
	writel(TTC | FEF | RTC | OSF, db->io_addr + rOperationMode); 			
#else
	writel(TTC | FEF | RTC, db->io_addr + rOperationMode); 			
#endif

	/* config the GMAC MACConfig register */
	writel(TC|WD|JD|BE|JE|IFG|DCRS|PS|FES|DO|LM|DM|IPC|DR|LUD|ACS|BL|DC|TE|RE, db->io_addr + rMACConfig);

	writel(PT | RFE | TFE, db->io_addr + rFlowControl);

	writel(0xff, db->io_addr + rStatus);	//clear all interrupt bits

	gmac_cre_def_tx_des(db->eth_base_txdes_vir);	
	gmac_cre_def_rx_des(db->eth_base_rxdes_vir);

	/* Init Driver variable */
	db->tx_pkt_cnt = 0;
	dev->trans_start = 0;

	/* enable TI and RI interrupt mask */
	gmac_enable_irq(db);

	pt_to_last_edge_rxdesc = db->eth_base_rxdes_vir;
#if TWOTXFRAMES
	txdes_tail = 0;
	txdes_head = 0;
#endif	

	/* start Tx and Rx */
	tmp= readl(db->io_addr + rOperationMode);
	tmp |= (ST | SR);
	writel(tmp, db->io_addr + rOperationMode);
}

/* Watchdog timed out. Called by the networking layer */
static void gmac_timeout(struct net_device *dev)
{
	board_info_t *db = (board_info_t *) netdev_priv(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	netif_stop_queue(dev);
	gmac_reset(db);
	gmac_init_gmac(dev);

	/*We can accept Tx frames again */
	dev->trans_start = jiffies;
	netif_wake_queue(dev);
}

static void gmac_tx_done(struct net_device *dev, board_info_t *db)
{
#if TWOTXFRAMES
	volatile unsigned int *pointto_cur_des;
	pointto_cur_des = (volatile unsigned int *)db->eth_base_txdes_vir + txdes_tail * 4;
	while ((!(*pointto_cur_des & TXOWN)) && (txdes_tail != txdes_head))
	{
		db->tx_pkt_cnt--;
		dev->stats.tx_packets++;

		/* report tx available now */
		if (!(netif_queue_stopped(dev)))
		{
			netif_wake_queue(dev);
		}

		if (*(pointto_cur_des + 3) == ETH_BASE_TXDES_PHY)  
		{
			pointto_cur_des -= (NO_OF_TXDES - 1) * 4;
		}   
		else
		{
			pointto_cur_des += 4;
		}   

		txdes_tail ++;
		if (txdes_tail >= NO_OF_TXDES)
		{
			txdes_tail = 0;
		}   
	}   
#else
	gmac_dbg(db, "entering %s\n", __func__);
	db->tx_pkt_cnt--;
	dev->stats.tx_packets++;

	/* report tx available now */
	netif_wake_queue(dev);
#endif
}

/*****************************************************************************
 * ** -Function:
 * **   gmac_start_xmit(struct sk_buff *skb, struct net_device *dev)
 * **
 * ** -Description:
 * **   This function implement sending a frame to media from the upper layer. 
 * **
 * ** -Input Param
 * **	skb: point to struct sk_buff which contain the frame needed to be sent
 * **	dev: point to the struct net_device whose private pointer is struct board_info_t *
 * **
 * ** -Return
 * **   0 means success, 1 means failure 
 * *****************************************************************************/
static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);
	uint32_t *pdate_phyaddr, date_phyaddr;
	unsigned long tmp;

	gmac_dbg(db, "entering %s\n", __func__);

	if (db->tx_pkt_cnt >= MAXTXFRAMES)
	{
		return NETDEV_TX_BUSY;
	}

	db->tx_pkt_cnt++;

	dev->stats.tx_bytes += skb->len;

	/* Move datas in skb buffer into Txbuf */
	date_phyaddr = dma_map_single(db->dev, skb->data, skb->len, DMA_TO_DEVICE);
	pdate_phyaddr = &date_phyaddr;

	gmac_cre_cur_tx_des(db, pdate_phyaddr, skb->len, db->eth_base_txdes_vir);

	gmac_dbg(db, "out of gmac_cre_cur_tx_des\n");
	/* Everytime after last Tx, TU will be set automatically. So we should clear it */
	tmp = readl(db->io_addr + rStatus);
	if (tmp & TU)
	{
		writel(TU, db->io_addr + rStatus);	
	}

	/* Issue Tx polling command */
	writel(0x1, db->io_addr + rTxPollDemand);

	dev->trans_start = jiffies;

#if GMAC_POLL
	/* Wait until DMA has moved all datas in the Txbuf to TX FIFO, ETI will be set. Then clear it */
	do
	{
		tmp = readl(db->io_addr + rStatus);
	}
	while (!(tmp & TI));

	writel(TI, db->io_addr + rStatus);

	gmac_tx_done(dev, db);
#endif
	   
	if (db->tx_pkt_cnt >= MAXTXFRAMES)
	{
		netif_stop_queue(dev);
	}
	/* free this skb */
	dev_kfree_skb(skb);
	gmac_dbg(db, "out of %s\n", __func__);

	return 0;
}

#if GMAC_TASK_IRQ
void my_tasklet_handler (unsigned long data)
{
	gmac_rx((struct net_device *)data);
}
#endif

/*****************************************************************************
 * ** -Function:
 * **   gmac_interrupt(int irq, void *dev_id)
 * **
 * ** -Description:
 * **   This function is gmac interrupt handler. It mainly responds to two interrupts.
 * **	One is the RX interrupt(RI), and the other is the TX interrupt(TI) means that gmac
 * **	has moved the TX frame in TX FIFO to PHY
 * **
 * ** -Input Param
 * **	irq: the irq number	
 * **	dev_id: point to the struct net_device whose private pointer is struct board_info_t *
 * **
 * ** -Return
 * **   IRQ_HANDLED means success, others mean failure 
 * *****************************************************************************/
static irqreturn_t gmac_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	board_info_t *db = netdev_priv(dev);
	int int_status;

	gmac_dbg(db, "entering %s\n", __func__);

	/* Disable all interrupt mask */
	//gmac_disable_irq(db);

	/* Got gmac-univ interrupt status and then clear the status*/
	int_status = gmac_get_irqstatus(db);
	gmac_clear_irqstatus(db, int_status);

	if (netif_msg_intr(db))
	{
		dev_dbg(db->dev, "interrupt status %02x\n", int_status);
	}

	/* Trnasmit Interrupt */
	if (int_status & TI)
	{
		gmac_tx_done(dev, db);
	}

	/* Received the coming frame */
	if (int_status & RI)
	{
#if GMAC_NORM_IRQ
		gmac_rx(dev);
#else
		tasklet_schedule(&db->my_tasklet);
#endif
	}

	/* Re-enable interrupt mask */
	//gmac_enable_irq(db);

	return IRQ_HANDLED;
}

/*
 * Open the interface.
 * The interface is opened whenever "ifconfig" actives it.
 */
static int gmac_open(struct net_device *dev)
{
	board_info_t *db = netdev_priv(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	if (netif_msg_ifup(db))
	{
		printk(KERN_DEBUG "enabling %s\n", dev->name);
	}

	/* request system irq for gmac irq_handle */
	if (request_irq(dev->irq, &gmac_interrupt, IRQF_DISABLED, dev->name, dev))
	{
		return -EAGAIN;
	}

	/* Initialize gmac board */
	gmac_reset(db);
	gmac_init_gmac(dev);

	mii_check_media(&db->mii, netif_msg_link(db), 1);
	netif_start_queue(dev);

	gmac_schedule_poll(db);

	return 0;
}

static void gmac_shutdown(struct net_device *dev)
{
	uint32_t tmp;
	board_info_t *db = netdev_priv(dev);

	gmac_dbg(db, "entering %s\n", __func__);

	/* reset device */
	gmac_phy_write(dev, 0, MII_BMCR, BMCR_RESET); //PHY RESET

	gmac_disable_irq(db);
	/* stop Tx and Rx */
	tmp= readl(db->io_addr + rOperationMode);
	tmp &= ~ST;
	tmp &= ~SR;
	writel(tmp, db->io_addr + rOperationMode);
}

/*
 * Stop the interface.
 * The interface is stopped when it is brought.
 */
static int gmac_stop(struct net_device *ndev)
{
	board_info_t *db = netdev_priv(ndev);

	gmac_dbg(db, "entering %s\n", __func__);
	printk("gmac_stop\r\n");

	cancel_delayed_work_sync(&db->phy_poll);

	netif_stop_queue(ndev);
	netif_carrier_off(ndev);

	/* free interrupt */
	free_irq(ndev->irq, ndev);

	gmac_shutdown(ndev);

	return 0;
}

static const struct net_device_ops gmac_netdev_ops = {
	.ndo_open = gmac_open,
	.ndo_stop = gmac_stop,
	.ndo_start_xmit = gmac_start_xmit,
	.ndo_tx_timeout = gmac_timeout,
	.ndo_set_multicast_list = gmac_hash_table,
	.ndo_do_ioctl = gmac_ioctl,
	.ndo_change_mtu = eth_change_mtu,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_mac_address = eth_mac_addr,
};

/* open all GPIO control registers for ethernet */
void gmac_cfg_gpio(void)
{
	uint32_t tmp;
	writel(0xAAAAAAAA, rGPKCON);

	tmp = readl(rGPJCON);
	tmp &= ~(0x3<<16);
	tmp |= 0x2<<16;
	writel(tmp, rGPJCON);
}

/*****************************************************************************
 * ** -Function:
 * **   gmac_probe(struct platform_device *pdev)
 * **
 * ** -Description:
 * **   This function mainly allocate space and resources for the gmac board.
 * **	Also net driver system function(netdev_ops) will be initalized. At last
 * **	the net device will be registerred into the kernel.
 * **
 * ** -Input Param
 * **	pdev: point to the struct platform_device which contain the IO resources
 * **	allocated for the gmac board.
 * **
 * ** -Return
 * **   0 means success, other mean failure 
 * *****************************************************************************/
static int __devinit gmac_probe(struct platform_device *pdev)
{
	struct board_info *db; 
	struct net_device *ndev;
	int ret = 0;
	int i;
	unsigned long base;
	u8  enet_addr[6]= {0x00, 0x1B, 0xFC, 0xEB, 0x92, 0xFA};		//mac address is 0x001BFCEB92FA

	/* config gpio control register for gmac */
	gmac_cfg_gpio();

	/* Init network device */
	ndev = alloc_etherdev(sizeof(struct board_info));
	if (!ndev)
	{
		dev_err(&pdev->dev, "could not allocate device.\n");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(ndev, &pdev->dev);

	dev_dbg(&pdev->dev, "gmac_probe()\n");

	/* setup board info structure */
	db = netdev_priv(ndev);
	memset(db, 0, sizeof(*db));

	db->debug_level = DEBUG_LEVEL;	//you can open the debug gate by setting this pamameter to any number more than 0;

	db->dev = &pdev->dev;
	db->ndev = ndev;

	//spin_lock_init(&db->lock);
	mutex_init(&db->addr_lock);

	INIT_DELAYED_WORK(&db->phy_poll, gmac_poll_work);
#if GMAC_TASK_IRQ
	tasklet_init(&db->my_tasklet, my_tasklet_handler, (unsigned long)db->ndev);
#endif

	/* These are the virtual addresses by ioremapping those physical addresses*/
	db->eth_base_txdes_vir = (volatile unsigned int *)dma_alloc_coherent(db->dev, 0x300000, &ETH_BASE_TXDES_PHY, GFP_KERNEL);
	db->eth_base_txbuf_vir = (volatile unsigned int *)(db->eth_base_txdes_vir + SIZE_OF_TXDES / 4);
	ETH_BASE_TXBUF_PHY = ETH_BASE_TXDES_PHY + SIZE_OF_TXDES;
	db->eth_base_rxdes_vir = (volatile unsigned int *)(db->eth_base_txbuf_vir + SIZE_OF_TXBUF / 4) ;
	ETH_BASE_RXDES_PHY = ETH_BASE_TXBUF_PHY + SIZE_OF_TXBUF;
	db->eth_base_rxbuf_vir = (volatile unsigned int *)(db->eth_base_rxdes_vir + SIZE_OF_RXDES / 4);
	ETH_BASE_RXBUF_PHY = ETH_BASE_RXDES_PHY + SIZE_OF_RXDES;

	if (!db->eth_base_txdes_vir || !db->eth_base_txbuf_vir || !db->eth_base_rxdes_vir || !db->eth_base_rxbuf_vir)
	{
		dev_err(db->dev,"failed to remap address reg");
		gmac_release_board(pdev, db);
		return -EINVAL;
	}

	/* Allocate the IO space for the gmac registers */
	base = pdev->resource[0].start;

	if (!(db->addr_req = request_mem_region(base, SIZE_OF_REG_SPA, ndev->name)))
	{
		gmac_release_board(pdev, db);
		return -EBUSY;
	}

	db->io_addr = ioremap(base, SIZE_OF_REG_SPA);
	if (!(db->io_addr))
	{
		dev_err(db->dev,"failed to remap address reg");
		gmac_release_board(pdev, db);
		return -EINVAL;
	}

	ndev->base_addr = base;
	/* Got the irq for gmac */
	ndev->irq = pdev->resource[1].start;

	/* driver system function */
	ether_setup(ndev);

	ndev->netdev_ops = &gmac_netdev_ops;
	ndev->watchdog_timeo = msecs_to_jiffies(watchdog);
	ndev->ethtool_ops = &gmac_ethtool_ops;

	/* Use the universal MII layer. Need GMII device surport */
	db->msg_enable = NETIF_MSG_LINK; 
	db->mii.phy_id_mask	= 0x1f;
	db->mii.reg_num_mask = 0x1f;
	db->mii.force_media	= 0;
	db->mii.full_duplex	= 0;
	db->mii.dev	= ndev;
	db->mii.mdio_read = gmac_phy_read;
	db->mii.mdio_write = gmac_phy_write;

	for (i=0; i<6; i++)
	{
		ndev->dev_addr[i] = enet_addr[i];
	}

	platform_set_drvdata(pdev, ndev);

	ret = register_netdev(ndev);
	if (ret == 0)
	{
		printk(KERN_EMERG "%s: gmac-univ at %p IRQ %d MAC: %pM \n",
		   ndev->name, db->io_addr, ndev->irq, ndev->dev_addr);
	}

	return 0;
}

static int gmac_drv_suspend(struct platform_device *dev, pm_message_t state)
{
	struct net_device *ndev = platform_get_drvdata(dev);
	board_info_t *db = netdev_priv(ndev);

	gmac_dbg(db, "entering %s\n", __func__);

	if (ndev)
	{
		db->in_suspend = 1;

		if (netif_running(ndev))
		{
			netif_stop_queue(ndev);
			netif_device_detach(ndev);

			gmac_shutdown(ndev);
		}
	}
	return 0;
}

static int gmac_drv_resume(struct platform_device *dev)
{
	struct net_device *ndev = platform_get_drvdata(dev);
	board_info_t *db = netdev_priv(ndev);

	gmac_dbg(db, "entering %s\n", __func__);

	if (ndev)
	{
		if (netif_running(ndev))
		{
			gmac_reset(db);
			gmac_init_gmac(ndev);

			netif_device_attach(ndev);
			netif_start_queue(ndev);
		}

		db->in_suspend = 0;
	}
	return 0;
}

static int __devexit gmac_drv_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	unregister_netdev(ndev);
	gmac_release_board(pdev, netdev_priv(ndev));
	free_netdev(ndev);		//free device structure

	dev_dbg(&pdev->dev, "released and freed device\n");
	return 0;
}

static struct platform_driver gmac_driver = {
	.driver	= {
		.name    = "imapx200_mac",
		.owner	 = THIS_MODULE,
	},
	.probe   = gmac_probe,
	.remove  = __devexit_p(gmac_drv_remove),
	.suspend = gmac_drv_suspend,
	.resume  = gmac_drv_resume,
};

static int __init gmac_init(void)
{
	printk(KERN_INFO "%s Ethernet Driver, V%s\n", CARDNAME, DRV_VERSION);

	return platform_driver_register(&gmac_driver);
}

static void __exit gmac_cleanup(void)
{
	platform_driver_unregister(&gmac_driver);
}

module_init(gmac_init);
module_exit(gmac_cleanup);

MODULE_AUTHOR("Bob.yang");
MODULE_DESCRIPTION("gmac-univ_ethernet driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imapx200_device_mac");




