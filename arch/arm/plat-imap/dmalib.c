
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/dmapool.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <plat/imapx.h>
#include <plat/dma.h>
#include <asm/cacheflush.h>
//#include <plat/dmalib.h>
//#define CONFIG_DMA__IMAP_DEBUG
#ifdef CONFIG_DMA__IMAP_DEBUG
#define imapx200_dbg(x...) printk(x)
#else
#define imapx200_dbg(x...)
#endif
u32 imapx200_dma_init_lli(struct imapx200_dma_chan *chan)
{
	u8 src_datawidth, dst_datawidth;
	u8 src_burst, dst_burst;
	u8 src_incre, dst_incre;
	u8 fc;
	u32 value;
	u32 i;
	u8 src_master, dst_master;
	u32 vDMA_CFG0, vDMA_CFG0H;
	u32 vDMA_SGR0, vDMA_DSR0, vDMA_SSTAT0, vDMA_DSTAT0;
	u32 vDMA_SSTATAR0, vDMA_DSTATAR0;
	u32 phy_addr;
	struct imapx200_dma_client *pclient = chan->client;
	struct imapx200_dma_ch *pch = pclient->pch;
	u32 intrx = 1<<chan->number;
	
	chan->plli = kzalloc(pclient->block_num * sizeof(struct dma_lli), GFP_KERNEL);
	if (!chan->plli) {
		printk(KERN_ERR "%s: failed to alloc mem\n", __func__);
		return -ENOMEM;
	}
	phy_addr = virt_to_phys(chan->plli);
	imapx200_dbg("phy_addr is %x\n",phy_addr);
	for (i = 0; i < pclient->block_num; i++) {
		//Widt-----h
		if (pch[i].src_datawidth == 8) src_datawidth = 0;
		else if (pch[i].src_datawidth == 16) src_datawidth = 1;
		else if (pch[i].src_datawidth == 32) src_datawidth = 2;
		else if (pch[i].src_datawidth == 64) src_datawidth = 3;
		else if (pch[i].src_datawidth == 128) src_datawidth = 4;
		else if (pch[i].src_datawidth == 256) src_datawidth = 5;
		else return -1;
		                 
		if (pch[i].dst_datawidth == 8) dst_datawidth = 0;
		else if (pch[i].dst_datawidth == 16) dst_datawidth = 1;
		else if (pch[i].dst_datawidth == 32) dst_datawidth = 2;
		else if (pch[i].dst_datawidth == 64) dst_datawidth = 3;
		else if (pch[i].dst_datawidth == 128) dst_datawidth = 4;
		else if (pch[i].dst_datawidth == 256) dst_datawidth = 5;
		else return -1;

		if (pch[i].src_burstlen == 1) src_burst = 0;
		else if (pch[i].src_burstlen == 4) src_burst = 1;
		else if (pch[i].src_burstlen == 8) src_burst = 2;
		else if (pch[i].src_burstlen == 16) src_burst = 3;
		else if (pch[i].src_burstlen == 32) src_burst = 4;
		else if (pch[i].src_burstlen == 64) src_burst = 5;
		else if (pch[i].src_burstlen == 128) src_burst = 6;
		else if (pch[i].src_burstlen == 256) src_burst = 7;
		else return -1;

		if (pch[i].dst_burstlen == 1) dst_burst = 0;
		else if (pch[i].dst_burstlen == 4) dst_burst = 1;
		else if (pch[i].dst_burstlen == 8) dst_burst = 2;
		else if (pch[i].dst_burstlen == 16) dst_burst = 3;
		else if (pch[i].dst_burstlen == 32) dst_burst = 4;
		else if (pch[i].dst_burstlen == 64) dst_burst = 5;
		else if (pch[i].dst_burstlen == 128) dst_burst = 6;
		else if (pch[i].dst_burstlen == 256) dst_burst = 7;
		else return -1;

		//Increment
		if (pch[i].src_incre == INCRE_ADD) src_incre = 0;
		else if (pch[i].src_incre == INCRE_SUB) src_incre = 1;
		else if (pch[i].src_incre == INCRE_CONST) src_incre = 2;
		else return -1;
		
		if (pch[i].dst_incre == INCRE_ADD) dst_incre = 0;
		else if (pch[i].dst_incre == INCRE_SUB) dst_incre = 1;
		else if (pch[i].dst_incre == INCRE_CONST) dst_incre = 2;
		else return -1;

		//Flow-Control
		if (pch[i].flow_ctrl == FC_DMA_M2M) fc = 0;
		else if (pch[i].flow_ctrl == FC_DMA_M2P) fc = 1;
		else if (pch[i].flow_ctrl == FC_DMA_P2M) fc = 2;
		else if (pch[i].flow_ctrl == FC_DMA_P2P) fc = 3;
		else if (pch[i].flow_ctrl == FC_PHI_P2M) fc = 4;
		else if (pch[i].flow_ctrl == FC_SPHI_P2P) fc = 5;
		else if (pch[i].flow_ctrl == FC_PHI_M2P) fc = 6;
		else if (pch[i].flow_ctrl == FC_DPHI_P2P) fc = 7;
		else return -1;

	    	if (pch[i].src_addr < MASTER_SELECT) src_master = 0;
	    	else src_master = 1;
	    	if (pch[i].dst_addr < MASTER_SELECT) dst_master = 0;
	    	else dst_master = 1;

		chan->plli[i].ctllo = chan_readl(chan,CTL_LO); 
		chan->plli[i].ctlhi = chan_readl(chan,CTL_HI); 
			value = pch[i].block_size/(pch[i].src_datawidth/8);
			if (value > DMA_MAX_BLOCKSIZE) {
				return -1;
			}
			chan->plli[i].ctlhi = value;
			chan->plli[i].ctllo &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
			chan->plli[i].ctllo |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
			chan->plli[i].ctllo |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);
			chan->plli[i].sar = pch[i].src_addr;
			imapx200_dbg("chan->plli[i].sar is %x\n", chan->plli[i].sar);
			chan->plli[i].dar = pch[i].dst_addr;
    		//src-master-select
    		chan->plli[i].ctllo &= ~(3 << DMA_CTL_SMS);
    		if (src_master) chan->plli[i].ctllo |= (1 << DMA_CTL_SMS);
		else chan->plli[i].ctllo &= ~(1 <<DMA_CTL_SMS);
    		
    		//dst-master-select
    		chan->plli[i].ctllo &= ~(3 << DMA_CTL_DMS);
    		if (dst_master) chan->plli[i].ctllo |= (1 << DMA_CTL_DMS);
		else chan->plli[i].ctllo &= ~(1 <<DMA_CTL_DMS);
			
			chan->plli[i].ctllo &= ~(0x3F << DMA_CTL_DEST_MSIZE);
			chan->plli[i].ctllo |= (dst_burst << DMA_CTL_DEST_MSIZE);
			chan->plli[i].ctllo |= (src_burst << DMA_CTL_SRC_MSIZE);
			chan->plli[i].ctllo &= ~(0xF << DMA_CTL_DINC);
			chan->plli[i].ctllo |= (dst_incre << DMA_CTL_DINC);
			chan->plli[i].ctllo |= (src_incre << DMA_CTL_SINC);
			chan->plli[i].ctllo &= ~(0x7 << DMA_CTL_TT_FC);
			chan->plli[i].ctllo |= (fc << DMA_CTL_TT_FC);
			if (pch[i].intr_en == 1) {
				chan->plli[i].ctllo |= (1 << DMA_CTL_INT_EN);

				//enable
				dma_set_bit(chan->dmac,MASK.XFER,intrx);
				dma_set_bit(chan->dmac,MASK.BLOCK,intrx);
				dma_set_bit(chan->dmac,MASK.ERROR,intrx);
			} else {
				dma_clear_bit(chan->dmac,MASK.XFER,intrx);
				dma_clear_bit(chan->dmac,MASK.BLOCK,intrx);
				dma_clear_bit(chan->dmac,MASK.ERROR,intrx);
				dma_clear_bit(chan->dmac,MASK.SRC_TRAN,intrx);
				dma_clear_bit(chan->dmac,MASK.DST_TRAN,intrx);
			}
			vDMA_CFG0 = chan_readl(chan,CFG_LO);
			vDMA_CFG0H = chan_readl(chan,CFG_HI);
			if (pch[i].hs_src_type == HANDSHAKE_HARD) {
				if ((pch[i].hs_src_index < 0) || (pch[i].hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
				
				vDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
				vDMA_CFG0H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
				vDMA_CFG0H |= (pch[i].hs_src_index << DMA_CFG_SRC_PER);
			} else if (pch[i].hs_src_type == HANDSHAKE_SOFT) {
				vDMA_CFG0 |= (1 << DMA_CFG_HS_SEL_SRC);
			} else if (pch[i].hs_src_type == HANDSHAKE_NO) {
				vDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_SRC);
				vDMA_CFG0H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			} else {
				return -1;
			}

			if (pch[i].hs_dst_type == HANDSHAKE_HARD) {
				if ((pch[i].hs_dst_index < 0) || (pch[i].hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
				
				vDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
				vDMA_CFG0H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
				vDMA_CFG0H |= (pch[i].hs_dst_index << DMA_CFG_DEST_PER);
			} else if (pch[i].hs_dst_type == HANDSHAKE_SOFT) {
				vDMA_CFG0 |= (1 << DMA_CFG_HS_SEL_DST);
			} else if (pch[i].hs_dst_type == HANDSHAKE_NO) {
				vDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_DST);
				vDMA_CFG0H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			} else {
				return -1;
			}

			if (pch[i].trans_type & TRANS_TYPE_MULTI) {
				if (pch[i].reload_type & RELOAD_LLI_SRC) {
					chan->plli[i].ctllo |= (1 << DMA_CTL_LLP_SRC_EN);
				} else {
					vDMA_CFG0 &= ~(1 << DMA_CTL_LLP_SRC_EN);
					vDMA_CFG0H &= ~(1 << DMA_CFG_SS_UPD_EN);
				}
				
				if (pch[i].reload_type & RELOAD_LLI_DST) {
					chan->plli[i].ctllo |= (1 << DMA_CTL_LLP_DST_EN);
				} else {
					vDMA_CFG0 &= ~(1 << DMA_CTL_LLP_SRC_EN);
				}

				if ((pch[i].reload_type & RELOAD_SRC) != 0) vDMA_CFG0 |= (1 << DMA_CFG_RELOAD_SRC);

				else vDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_SRC);
				                  
				if ((pch[i].reload_type & RELOAD_DST) != 0)  vDMA_CFG0 |= (1 << DMA_CFG_RELOAD_DST);
				else vDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_DST);
			}

			//if ((pch[i].trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				//if (pch->reload_type & RELOAD_SRC) src = 0;
				//else src = -1;
				//if (pch->reload_type & RELOAD_DST) dst = 0;
				//else dst = -1;
				//if (dmareg_cfg_auto_reload(reg, dma_index, src, dst) == -1) return -1;
				//if ((pch[i].reload_type & RELOAD_SRC) != 0) vDMA_CFG0 |= (1 << DMA_CFG_RELOAD_SRC);
				//else vDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_SRC);
				                  
				//if ((pch[i].reload_type & RELOAD_DST) != 0)  vDMA_CFG0 |= (1 << DMA_CFG_RELOAD_DST);
				//else vDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_DST);
			//}
			
			if ((pch[i].trans_type & TRANS_TYPE_SINGLE) == TRANS_TYPE_SINGLE) {
				//TRANSFER TYPE
				chan->plli[i].llp = 0;
				chan->plli[i].ctllo &= ~(1 << DMA_CTL_LLP_SRC_EN);
				chan->plli[i].ctllo &= ~(1 << DMA_CTL_LLP_DST_EN);
				vDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_SRC);
				vDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
			
		// 2D
		if ((pch[i].enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			//src = 0;
			pch[i].src2d_size = pch[i].src2d_size/(pch[i].src_datawidth/8);
			pch[i].src2d_interval = pch[i].src2d_interval/(pch[i].src_datawidth/8);
			chan->plli[i].ctllo |= (1 << DMA_CTL_SRC_GATHER_EN);
			vDMA_SGR0= (pch[i].src2d_size << DMA_SGR_SGC) | (pch[i].src2d_interval << DMA_SGR_SGI);
		} else {
			chan->plli[i].ctllo &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch[i].enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			//dst = 0;
			pch[i].dst2d_size = pch[i].dst2d_size/(pch[i].dst_datawidth/8);
			pch[i].dst2d_interval = pch[i].dst2d_interval/(pch[i].dst_datawidth/8);
			chan->plli[i].ctllo |= (1 << DMA_CTL_DST_SCATTER_EN);
			vDMA_DSR0= (pch[i].dst2d_size << DMA_DSR_DSC) | (pch[i].dst2d_interval << DMA_DSR_DSI);
		} else {
			chan->plli[i].ctllo &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}

		vDMA_CFG0 &= ~(1 << DMA_CFG_CH_SUSP);
		vDMA_CFG0 |= (16 << DMA_CFG_MAX_ABRST);
		
		
			chan->plli[i].sar = pch[i].src_addr;
			chan->plli[i].dar = pch[i].dst_addr;
			
			if ((i+1) < pclient->block_num) chan->plli[i].llp = (u32)(phy_addr+(i+1)*sizeof(struct dma_lli));
			else if (pclient->is_loop == 1) chan->plli[i].llp = (u32)phy_addr;
			else chan->plli[i].llp = 0;
			
			chan->plli[i].llp &= ~(3 << DMA_LLP_LMS);
			if (chan->plli[i].llp < MASTER_SELECT)  chan->plli[i].llp &= ~(1 << DMA_LLP_LMS);
			else chan->plli[i].llp |= (1 << DMA_LLP_LMS);

			chan->plli[i].ctllo = chan->plli[i].ctllo;
			chan->plli[i].ctlhi = chan->plli[i].ctlhi;
			chan->plli[i].sstat = vDMA_SSTAT0;
			chan->plli[i].dstat = vDMA_DSTAT0;
//		} 

		
		if (chan->plli) {
			if (pch[i].reload_type & RELOAD_LLI_SRC) {
				chan->plli[i].ctllo |= (1 << DMA_CTL_LLP_SRC_EN);
			} else {
				vDMA_CFG0 &= ~(1 << DMA_CTL_LLP_SRC_EN);
				//vreg.vDMA_CFG0H &= ~(1 << DMA_CFG_SS_UPD_EN);
			}
			
			if (pch[i].reload_type & RELOAD_LLI_DST) {
				chan->plli[i].ctllo |= (1 << DMA_CTL_LLP_DST_EN);
			} else {
				vDMA_CFG0 &= ~(1 << DMA_CTL_LLP_DST_EN);
			}
		}
		
		
		//WRITE-BACK
		//if ((pch[0].enable_wb & WRITEBACK_SRC) == WRITEBACK_SRC) wb_src = 0;
		//if ((pch[0].enable_wb & WRITEBACK_DST) == WRITEBACK_DST) wb_dst = 0;
		//if (dmareg_cfg_writeback(dma_index, wb_src, pch[0].srcwb_addr, wb_dst, pch[0].dstwb_addr) == -1) return -1;
		if ((pch[0].enable_wb & WRITEBACK_SRC) == WRITEBACK_SRC) {
			vDMA_CFG0H |= (1 << DMA_CFG_SS_UPD_EN);
			vDMA_SSTATAR0 = pch[0].srcwb_addr;
		} else {
			vDMA_CFG0H &= ~(1 << DMA_CFG_SS_UPD_EN);
		}

		if ((pch[0].enable_wb & WRITEBACK_DST) == WRITEBACK_DST) {
			vDMA_CFG0H |= (1 << DMA_CFG_DS_UPD_EN);
			vDMA_DSTATAR0 = pch[0].dstwb_addr;
		} else {
			vDMA_CFG0H &= ~(1 << DMA_CFG_DS_UPD_EN);
		}
		
		vDMA_CFG0 |= (16 << DMA_CFG_MAX_ABRST);
	}



		chan_writel(chan, SAR, chan->plli[0].sar);
		chan_writel(chan, DAR, chan->plli[0].dar);
		
		
		if (phy_addr < MASTER_SELECT) chan_writel(chan, LLP, phy_addr & ~(1 << DMA_LLP_LMS));//(u32)physAddr;
		else chan_writel(chan, LLP, phy_addr | (1 << DMA_LLP_LMS));

		chan_writel(chan, CTL_LO, chan->plli[0].ctllo);
		chan_writel(chan, CTL_HI, chan->plli[0].ctlhi);
		chan_writel(chan, CFG_LO, vDMA_CFG0);
		chan_writel(chan, CFG_HI, vDMA_CFG0H);

		
		dmac_clean_range(chan->plli, chan->plli + pclient->block_num * sizeof(struct dma_lli)); 	
	return 1;
}


u32 imapx200_dma_init_cmn(struct imapx200_dma_chan *chan)
{

#if 0
	u8 src_datawidth, dst_datawidth;
	u8 src_burst, dst_burst;
	u8 src_incre, dst_incre;
	u8 fc;
	u32 value;
	u8 src_master, dst_master;
	
	if ((dma_index < 0) || (dma_index >= DMA_MAX_NUM)) {
		return -1;
	}

	//Width
	if (pch->src_datawidth == 8) src_datawidth = 0;
	else if (pch->src_datawidth == 16) src_datawidth = 1;
	else if (pch->src_datawidth == 32) src_datawidth = 2;
	else if (pch->src_datawidth == 64) src_datawidth = 3;
	else if (pch->src_datawidth == 128) src_datawidth = 4;
	else if (pch->src_datawidth == 256) src_datawidth = 5;
	else return -1;
	
	if (pch->dst_datawidth == 8) dst_datawidth = 0;
	else if (pch->dst_datawidth == 16) dst_datawidth = 1;
	else if (pch->dst_datawidth == 32) dst_datawidth = 2;
	else if (pch->dst_datawidth == 64) dst_datawidth = 3;
	else if (pch->dst_datawidth == 128) dst_datawidth = 4;
	else if (pch->dst_datawidth == 256) dst_datawidth = 5;
	else return -1;

	//Burst
	if (((pch->src_bstwidth*8)%pch->src_datawidth) != 0) {
		return -1;
	}
	if (((pch->dst_bstwidth*8)%pch->dst_datawidth) != 0) {
		return -1;
	}

	if (((pch->src_bstwidth*8)/pch->src_datawidth) == 1) src_burst = 0;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 4) src_burst = 1;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 8) src_burst = 2;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 16) src_burst = 3;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 32) src_burst = 4;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 64) src_burst = 5;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 128) src_burst = 6;
	else if (((pch->src_bstwidth*8)/pch->src_datawidth) == 256) src_burst = 7;
	else return -1;

	if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 1) dst_burst = 0;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 4) dst_burst = 1;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 8) dst_burst = 2;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 16) dst_burst = 3;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 32) dst_burst = 4;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 64) dst_burst = 5;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 128) dst_burst = 6;
	else if (((pch->dst_bstwidth*8)/pch->dst_datawidth) == 256) dst_burst = 7;
	else return -1;

	//Increment
	if (pch->src_incre == INCRE_ADD) src_incre = 0;
	else if (pch->src_incre == INCRE_SUB) src_incre = 1;
	else if (pch->src_incre == INCRE_CONST) src_incre = 2;
	else return -1;
	
	if (pch->dst_incre == INCRE_ADD) dst_incre = 0;
	else if (pch->dst_incre == INCRE_SUB) dst_incre = 1;
	else if (pch->dst_incre == INCRE_CONST) dst_incre = 2;
	else return -1;

	//Flow-Control
	if (pch->flow_ctrl == FC_DMA_M2M) fc = 0;
	else if (pch->flow_ctrl == FC_DMA_M2P) fc = 1;
	else if (pch->flow_ctrl == FC_DMA_P2M) fc = 2;
	else if (pch->flow_ctrl == FC_DMA_P2P) fc = 3;
	else if (pch->flow_ctrl == FC_PHI_P2M) fc = 4;
	else if (pch->flow_ctrl == FC_SPHI_P2P) fc = 5;
	else if (pch->flow_ctrl == FC_PHI_M2P) fc = 6;
	else if (pch->flow_ctrl == FC_DPHI_P2P) fc = 7;
	else return -1;

	if (pch->src_addr < MASTER_SELECT) src_master = 0;
	else src_master = 1;
	if (pch->dst_addr < MASTER_SELECT) dst_master = 0;
	else dst_master = 1;

	if (dma_index == 0) {
		//if (dmareg_init(reg, dma_index) == -1) return -1; 
		//if (dmareg_cfg_block(reg, dma_index, pch->block_size, pch->src_datawidth) == -1) return -1;
			value = pch->block_size/(pch->src_datawidth/8);
			if (value > DMA_MAX_BLOCKSIZE) {
				return -1;
			}
			reg->rDMA_CTL0H = value;
		//if (dmareg_cfg_datawidth(reg, dma_index, pch->src_datawidth, pch->dst_datawidth) == -1) return -1;
			reg->rDMA_CTL0 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
			reg->rDMA_CTL0 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
			reg->rDMA_CTL0 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);
		//if (dmareg_cfg_address(reg, dma_index, pch->src_addr, pch->dst_addr) == -1) return -1;
			reg->rDMA_SAR0 = pch->src_addr;
			reg->rDMA_DAR0 = pch->dst_addr;
		    //src-master-select
    		reg->rDMA_CTL0 &= ~(3 << DMA_CTL_SMS);
    		if (src_master) reg->rDMA_CTL0 |= (1 << DMA_CTL_SMS);
    		//dst-master-select
    		reg->rDMA_CTL0 &= ~(3 << DMA_CTL_DMS);
    		if (dst_master) reg->rDMA_CTL0 |= (1 << DMA_CTL_DMS);

		//if (dmareg_cfg_burst(reg, dma_index, pch->src_bstwidth, pch->src_datawidth, pch->dst_bstwidth, pch->dst_datawidth) == -1) return -1;
			reg->rDMA_CTL0 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
			reg->rDMA_CTL0 |= (dst_burst << DMA_CTL_DEST_MSIZE);
			reg->rDMA_CTL0 |= (src_burst << DMA_CTL_SRC_MSIZE);
			//DEBUGMSG(1, (TEXT("dmareg_cfg_burst: reg->rDMA_CTL0H = %x\n"), reg->rDMA_CTL0H));	
		//if (dmareg_cfg_incre(reg, dma_index, pch->src_incre, pch->dst_incre) == -1) return -1;
			reg->rDMA_CTL0 &= ~(0xF << DMA_CTL_DINC);
			reg->rDMA_CTL0 |= (dst_incre << DMA_CTL_DINC);
			reg->rDMA_CTL0 |= (src_incre << DMA_CTL_SINC);
		//if (dmareg_cfg_flowctrl(reg, dma_index, pch->flow_ctrl) == -1) return -1;
			reg->rDMA_CTL0 &= ~(0x7 << DMA_CTL_TT_FC);
			reg->rDMA_CTL0 |= (fc << DMA_CTL_TT_FC);
		//if (dmareg_cfg_interrupt(reg, dma_index, pch->intr_en) == -1) return -1;
			if (pch->intr_en == 0) {
				//enable
				reg->rDMA_MASKTFR |= (1 << DMA_INTR_H0) | (1 << DMA_INTR_0);		
				reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H0) | (1 << DMA_INTR_0);
				//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H0) | (1 << DMA_INTR_0);
				//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H0) | (1 << DMA_INTR_0);
				reg->rDMA_MASKERR |= (1 << DMA_INTR_H0) | (1 << DMA_INTR_0);
				//DEBUGMSG(1, (TEXT("dmareg_cfg_interrupt: reg->rDMA_CTL0H = %x\n"), reg->rDMA_CTL0H));	
			} else {
				reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_0);
				reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_0);
				//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_0);
				//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_0);
				reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_0);

				//DEBUGMSG(1, (TEXT("dmareg_cfg_interrupt: reg->rDMA_CTL0H = %x\n"), reg->rDMA_CTL0H));	
			}
		//if (dmareg_cfg_handshake(reg, dma_index, pch->hs_src_type, pch->hs_src_index, pch->hs_dst_type, pch->hs_dst_index) == -1) return -1;
			if (pch->hs_src_type == HANDSHAKE_HARD) {
				if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
				
				reg->rDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
				reg->rDMA_CFG0H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
				reg->rDMA_CFG0H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
			} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
				reg->rDMA_CFG0 |= (1 << DMA_CFG_HS_SEL_SRC);
			} else if (pch->hs_src_type == HANDSHAKE_NO) {
				reg->rDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_SRC);
			} else {
				return -1;
			}

			if (pch->hs_dst_type == HANDSHAKE_HARD) {
				if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
				
				reg->rDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
				reg->rDMA_CFG0H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
				reg->rDMA_CFG0H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
			} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
				reg->rDMA_CFG0 |= (1 << DMA_CFG_HS_SEL_DST);
			} else if (pch->hs_dst_type == HANDSHAKE_NO) {
				reg->rDMA_CFG0 &= ~(1 << DMA_CFG_HS_SEL_DST);
			} else {
				return -1;
			}

		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				//if (pch->reload_type & RELOAD_SRC) src = 0;
				//else src = -1;
				//if (pch->reload_type & RELOAD_DST) dst = 0;
				//else dst = -1;
				//if (dmareg_cfg_auto_reload(reg, dma_index, src, dst) == -1) return -1;
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG0 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG0 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			//TRANSFER TYPE
			//dmareg_cfg_trans_type_default(reg, dma_index);
			reg->rDMA_LLP0 = 0;
			reg->rDMA_CTL0 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL0 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG0 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			//src = 0;
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL0 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR0= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			//src = -1;
			reg->rDMA_CTL0 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}
		//if (dmareg_cfg_src2d(reg, dma_index, src, pch->src2d_size, pch->src2d_interval) == -1) return -1;


		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			//dst = 0;
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL0 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR0= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			//dst = -1;
			reg->rDMA_CTL0 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		//if (dmareg_cfg_dst2d(reg, dma_index, dst, pch->dst2d_size, pch->dst2d_interval) == -1) return -1;

		//
		//if (dmareg_cfg_default(reg, dma_index, pch->block_num) == -1) return -1;
		reg->rDMA_CFG0 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG0 |= (16 << DMA_CFG_MAX_ABRST);
		
		//dma_print_channel(dma_index, reg);
		return 0;
	}	

	if (dma_index == 1) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL1H = value;

		//data-width
		reg->rDMA_CTL1 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL1 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL1 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR1 = pch->src_addr;
		reg->rDMA_DAR1 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL1 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL1 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL1 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL1 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL1 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL1 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL1 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL1 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL1 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL1 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL1 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL1 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H1) | (1 << DMA_INTR_1);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H1) | (1 << DMA_INTR_1);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H1) | (1 << DMA_INTR_1);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H1) | (1 << DMA_INTR_1);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H1) | (1 << DMA_INTR_1);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H1) & ~(1 << DMA_INTR_1);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H1) & ~(1 << DMA_INTR_1);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_1);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_1);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H1) & ~(1 << DMA_INTR_1);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG1 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG1H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG1H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG1 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG1 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG1 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG1H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG1H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG1 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG1 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG1 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG1 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG1 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG1 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP1 = 0;
			reg->rDMA_CTL1 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL1 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG1 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG1 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL1 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR1= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL1 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL1 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR1= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL1 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG1 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG1 |= (16 << DMA_CFG_MAX_ABRST);
		
		//dma_print_channel(dma_index, reg);
		return 0;
	}	

	if (dma_index == 2) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL2H = value;

		//data-width
		reg->rDMA_CTL2 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL2 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL2 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR2 = pch->src_addr;
		reg->rDMA_DAR2 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL2 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL2 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL2 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL2 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL2 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL2 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL2 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL2 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL2 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL2 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL2 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL2 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H2) | (1 << DMA_INTR_2);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H2) | (1 << DMA_INTR_2);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H2) | (1 << DMA_INTR_2);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H2) | (1 << DMA_INTR_2);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H2) | (1 << DMA_INTR_2);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H2) & ~(1 << DMA_INTR_2);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H2) & ~(1 << DMA_INTR_2);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_2);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_2);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H2) & ~(1 << DMA_INTR_2);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG2 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG2H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG2H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG2 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG2 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG2 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG2H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG2H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG2 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG2 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG2 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG2 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG2 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG2 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP2 = 0;
			reg->rDMA_CTL2 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL2 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG2 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG2 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL2 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR2= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL2 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL2 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR2= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL2 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG2 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG2 |= (16 << DMA_CFG_MAX_ABRST);
		
		return 0;
	}	

	if (dma_index == 3) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL3H = value;

		//data-width
		reg->rDMA_CTL3 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL3 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL3 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR3 = pch->src_addr;
		reg->rDMA_DAR3 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL3 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL3 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL3 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL3 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL3 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL3 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL3 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL3 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL3 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL3 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL3 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL3 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H3) | (1 << DMA_INTR_3);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H3) | (1 << DMA_INTR_3);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H3) | (1 << DMA_INTR_3);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H3) | (1 << DMA_INTR_3);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H3) | (1 << DMA_INTR_3);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H3) & ~(1 << DMA_INTR_3);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H3) & ~(1 << DMA_INTR_3);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_3);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H0) & ~(1 << DMA_INTR_3);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H3) & ~(1 << DMA_INTR_3);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG3 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG3H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG3H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG3 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG3 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG3 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG3H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG3H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG3 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG3 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG3 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG3 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG3 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG3 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP3 = 0;
			reg->rDMA_CTL3 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL3 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG3 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG3 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL3 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR3= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL3 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL3 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR3= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL3 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG3 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG3 |= (16 << DMA_CFG_MAX_ABRST);
		
		return 0;
	}	


	if (dma_index == 4) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL4H = value;

		//data-width
		reg->rDMA_CTL4 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL4 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL4 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR4 = pch->src_addr;
		reg->rDMA_DAR4 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL4 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL4 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL4 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL4 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL4 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL4 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL4 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL4 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL4 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL4 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL4 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL4 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H4) | (1 << DMA_INTR_4);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H4) | (1 << DMA_INTR_4);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H4) | (1 << DMA_INTR_4);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H4) | (1 << DMA_INTR_4);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H4) | (1 << DMA_INTR_4);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H4) & ~(1 << DMA_INTR_4);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H4) & ~(1 << DMA_INTR_4);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H4) & ~(1 << DMA_INTR_4);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H4) & ~(1 << DMA_INTR_4);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H4) & ~(1 << DMA_INTR_4);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG4 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG4H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG4H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG4 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG4 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG4 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG4H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG4H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG4 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG4 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG4 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG4 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG4 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG4 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP4 = 0;
			reg->rDMA_CTL4 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL4 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG4 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG4 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL4 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR4= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL4 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL4 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR4= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL4 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG4 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG4 |= (16 << DMA_CFG_MAX_ABRST);
		
		return 0;
	}	
	
	if (dma_index == 5) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL5H = value;

		//data-width
		reg->rDMA_CTL5 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL5 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL5 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR5 = pch->src_addr;
		reg->rDMA_DAR5 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL5 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL5 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL5 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL5 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL5 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL5 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL5 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL5 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL5 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL5 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL5 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL5 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H5) | (1 << DMA_INTR_5);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H5) | (1 << DMA_INTR_5);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H5) | (1 << DMA_INTR_5);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H5) | (1 << DMA_INTR_5);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H5) | (1 << DMA_INTR_5);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H5) & ~(1 << DMA_INTR_5);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H5) & ~(1 << DMA_INTR_5);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H5) & ~(1 << DMA_INTR_5);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H5) & ~(1 << DMA_INTR_5);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H5) & ~(1 << DMA_INTR_5);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG5 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG5H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG5H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG5 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG5 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG5 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG5H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG5H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG5 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG5 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG5 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG5 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG5 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG5 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP5 = 0;
			reg->rDMA_CTL5 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL5 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG5 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG5 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL5 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR5= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL5 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL5 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR5= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL5 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG5 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG5 |= (16 << DMA_CFG_MAX_ABRST);
		
		return 0;
	}	

	
	if (dma_index == 6) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL6H = value;

		//data-width
		reg->rDMA_CTL6 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL6 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL6 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR6 = pch->src_addr;
		reg->rDMA_DAR6 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL6 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL6 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL6 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL6 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL6 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL6 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL6 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL6 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL6 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL6 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL6 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL6 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H6) | (1 << DMA_INTR_6);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H6) | (1 << DMA_INTR_6);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H6) | (1 << DMA_INTR_6);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H6) | (1 << DMA_INTR_6);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H6) | (1 << DMA_INTR_6);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H6) & ~(1 << DMA_INTR_6);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H6) & ~(1 << DMA_INTR_6);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H6) & ~(1 << DMA_INTR_6);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H6) & ~(1 << DMA_INTR_6);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H6) & ~(1 << DMA_INTR_6);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG6 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG6H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG6H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG6 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG6 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG6 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG6H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG6H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG6 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG6 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG6 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG6 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG6 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG6 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP6 = 0;
			reg->rDMA_CTL6 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL6 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG6 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG6 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL6 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR6= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL6 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL6 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR6= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL6 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG6 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG6 |= (16 << DMA_CFG_MAX_ABRST);
		
		return 0;
	}	

	
	if (dma_index == 7) {
		value = pch->block_size/(pch->src_datawidth/8);
		if (value > DMA_MAX_BLOCKSIZE) {
			return -1;
		}
		reg->rDMA_CTL7H = value;

		//data-width
		reg->rDMA_CTL7 &= ~(0x3F << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL7 |= (dst_datawidth << DMA_CTL_DST_TR_WIDTH);
		reg->rDMA_CTL7 |= (src_datawidth << DMA_CTL_SRC_TR_WIDTH);

		//addr
		reg->rDMA_SAR7 = pch->src_addr;
		reg->rDMA_DAR7 = pch->dst_addr;
	    //src-master-select
		reg->rDMA_CTL7 &= ~(3 << DMA_CTL_SMS);
		if (src_master) reg->rDMA_CTL7 |= (1 << DMA_CTL_SMS);
		//dst-master-select
		reg->rDMA_CTL7 &= ~(3 << DMA_CTL_DMS);
		if (dst_master) reg->rDMA_CTL7 |= (1 << DMA_CTL_DMS);
		    
		//burst_length
		reg->rDMA_CTL7 &= ~(0x3F << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL7 |= (dst_burst << DMA_CTL_DEST_MSIZE);
		reg->rDMA_CTL7 |= (src_burst << DMA_CTL_SRC_MSIZE);
		
		//increment
		reg->rDMA_CTL7 &= ~(0xF << DMA_CTL_DINC);
		reg->rDMA_CTL7 |= (dst_incre << DMA_CTL_DINC);
		reg->rDMA_CTL7 |= (src_incre << DMA_CTL_SINC);

		//flow-control
		reg->rDMA_CTL7 &= ~(0x7 << DMA_CTL_TT_FC);
		reg->rDMA_CTL7 |= (fc << DMA_CTL_TT_FC);

		//interrupt
		if (pch->intr_en == 0) {
			//enable
			reg->rDMA_MASKTFR |= (1 << DMA_INTR_H7) | (1 << DMA_INTR_7);		
			reg->rDMA_MASKBLOCK |= (1 << DMA_INTR_H7) | (1 << DMA_INTR_7);
			//reg->rDMA_MASKSRCTRAN |= (1 << DMA_INTR_H7) | (1 << DMA_INTR_7);
			//reg->rDMA_MASKDSTTRAN |= (1 << DMA_INTR_H7) | (1 << DMA_INTR_7);
			reg->rDMA_MASKERR |= (1 << DMA_INTR_H7) | (1 << DMA_INTR_7);
		} else {
			reg->rDMA_MASKTFR &= ~(1 << DMA_INTR_H7) & ~(1 << DMA_INTR_7);
			reg->rDMA_MASKBLOCK &= ~(1 << DMA_INTR_H7) & ~(1 << DMA_INTR_7);
			//reg->rDMA_MASKSRCTRAN &= ~(1 << DMA_INTR_H7) & ~(1 << DMA_INTR_7);
			//reg->rDMA_MASKDSTTRAN &= ~(1 << DMA_INTR_H7) & ~(1 << DMA_INTR_7);
			reg->rDMA_MASKERR &= ~(1 << DMA_INTR_H7) & ~(1 << DMA_INTR_7);
		}

		
		//hand-shake
		if (pch->hs_src_type == HANDSHAKE_HARD) {
			if ((pch->hs_src_index < 0) || (pch->hs_src_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG7 &= ~(1 << DMA_CFG_HS_SEL_SRC);	//hardshake
			reg->rDMA_CFG7H &= ~(0xF << DMA_CFG_SRC_PER);	//assign interface-index
			reg->rDMA_CFG7H |= (pch->hs_src_index << DMA_CFG_SRC_PER);
		} else if (pch->hs_src_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG7 |= (1 << DMA_CFG_HS_SEL_SRC);
		} else if (pch->hs_src_type == HANDSHAKE_NO) {
			reg->rDMA_CFG7 &= ~(1 << DMA_CFG_HS_SEL_SRC);
		} else {
			return -1;
		}

		if (pch->hs_dst_type == HANDSHAKE_HARD) {
			if ((pch->hs_dst_index < 0) || (pch->hs_dst_index >= HANDSHAKE_INDEX_MAX)) return -1;
			
			reg->rDMA_CFG7 &= ~(1 << DMA_CFG_HS_SEL_DST);	//hardshake
			reg->rDMA_CFG7H &= ~(0xF << DMA_CFG_DEST_PER);	//assign interface-index
			reg->rDMA_CFG7H |= (pch->hs_dst_index << DMA_CFG_DEST_PER);
		} else if (pch->hs_dst_type == HANDSHAKE_SOFT) {
			reg->rDMA_CFG7 |= (1 << DMA_CFG_HS_SEL_DST);
		} else if (pch->hs_dst_type == HANDSHAKE_NO) {
			reg->rDMA_CFG7 &= ~(1 << DMA_CFG_HS_SEL_DST);
		} else {
			return -1;
		}

		//multi-block
		if (pch->block_num > 1) {		
			if ((pch->trans_type & TRANS_TYPE_AUTO) == TRANS_TYPE_AUTO) {
				if ((pch->reload_type & RELOAD_SRC) != 0) reg->rDMA_CFG7 |= (1 << DMA_CFG_RELOAD_SRC);
				else reg->rDMA_CFG7 &= ~(1 << DMA_CFG_RELOAD_SRC);
				
				if ((pch->reload_type & RELOAD_DST) != 0)  reg->rDMA_CFG7 |= (1 << DMA_CFG_RELOAD_DST);
				else reg->rDMA_CFG7 &= ~(1 << DMA_CFG_RELOAD_DST);
			}
		} else {
			reg->rDMA_LLP7 = 0;
			reg->rDMA_CTL7 &= ~(1 << DMA_CTL_LLP_SRC_EN);
			reg->rDMA_CTL7 &= ~(1 << DMA_CTL_LLP_DST_EN);
			reg->rDMA_CFG7 &= ~(1 << DMA_CFG_RELOAD_SRC);
			reg->rDMA_CFG7 &= ~(1 << DMA_CFG_RELOAD_DST);
		}

		// 2D
		if ((pch->enable_2d & ENABLE_SRC_2D) == (ENABLE_SRC_2D)) {
			pch->src2d_size = pch->src2d_size/(pch->src_datawidth/8);
			pch->src2d_interval = pch->src2d_interval/(pch->src_datawidth/8);
			reg->rDMA_CTL7 |= (1 << DMA_CTL_SRC_GATHER_EN);
			reg->rDMA_SGR7= (pch->src2d_size << DMA_SGR_SGC) | (pch->src2d_interval << DMA_SGR_SGI);
		} else {
			reg->rDMA_CTL7 &= ~(1 << DMA_CTL_SRC_GATHER_EN);
		}

		if ((pch->enable_2d & ENABLE_DST_2D) == (ENABLE_DST_2D)) {
			pch->dst2d_size = pch->dst2d_size/(pch->dst_datawidth/8);
			pch->dst2d_interval = pch->dst2d_interval/(pch->dst_datawidth/8);
			reg->rDMA_CTL7 |= (1 << DMA_CTL_DST_SCATTER_EN);
			reg->rDMA_DSR7= (pch->dst2d_size << DMA_DSR_DSC) | (pch->dst2d_interval << DMA_DSR_DSI);
		} else {
			reg->rDMA_CTL7 &= ~(1 << DMA_CTL_DST_SCATTER_EN);
		}
		reg->rDMA_CFG7 &= ~(1 << DMA_CFG_CH_SUSP);
		reg->rDMA_CFG7 |= (16 << DMA_CFG_MAX_ABRST);
		
		return 0;
	}	
#endif

	return -1;
}
