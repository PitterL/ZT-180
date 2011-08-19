


#include <linux/platform_device.h>
#include <linux/clk.h>
#include <plat/regs-clock.h>
#include <plat/imapx.h>

#define IMAP_DEBUG  0

#if IMAP_DEBUG
#define IMAP_OHCI_DEBUG(debugs, ...)	printk(KERN_INFO debugs,  ##__VA_ARGS__)
#else
#define IMAP_OHCI_DEBUG(debugs, ...)	do{}while(0)
#endif

static struct clk *clk;
//static struct clk *usb_clk;

/*
static struct imapx200_hcd_info *to_imapx200_info(struct usb_hcd *hcd)
{
	return hcd->self.controller->platform_data;
}
*/

static void imapx200_start_hc(struct platform_device *pdev,struct usb_hcd *hcd)
{
	struct ohci_regs __iomem *regs = hcd->regs;

	IMAP_OHCI_DEBUG("++imapx200_start_hc \r\n");
	
	//clk_enable(usb_clk);
	mdelay(4);

	//clk_enable(clk);

	__raw_writel(0x0, &regs->control);
	
	IMAP_OHCI_DEBUG("--imapx200_start_hc \r\n");
}

static void imapx200_stop_hc(struct platform_device *pdev)
{
	IMAP_OHCI_DEBUG("++imapx200_stop_hc \r\n");
	
	//clk_disable(clk);
	//clk_disable(usb_clk);
	
	IMAP_OHCI_DEBUG("--imapx200_stop_hc \r\n");
}



static void usb_hcd_imapx200_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	imapx200_stop_hc(pdev);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start,hcd->rsrc_len);
	usb_put_hcd(hcd);
}


static int usb_hcd_imapx200_probe(const struct hc_driver *driver,struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	struct platform_data *info;
	int ret,irq;
	struct resource *r_mem;
//	int valTmp,i;

	IMAP_OHCI_DEBUG("++usb_hcd_imapx200_probe\r\n");
	
	info = pdev->dev.platform_data;
	if(info)
		return -ENODEV;

	//__raw_writel(0x0, &regs->control);
	
	irq = platform_get_irq(pdev,0);
	if(irq < 0)
	{
		dev_err(&pdev->dev,"no resource of IORESOURCE_IRQ");
		return -ENXIO;
	}
	IMAP_OHCI_DEBUG("USB Resource IRQ is 0x%x \r\n",irq);

	r_mem = platform_get_resource(pdev,IORESOURCE_MEM,0);
	if(!r_mem)
	{
		dev_err(&pdev->dev,"no resource of IORESOURCE_MEM");
		return -ENXIO;
	}
	hcd = usb_create_hcd(driver,&pdev->dev, "imapx200");
	if(hcd == NULL)
	{
		dev_err(&pdev->dev,"usb_create_hcd failed! \r\n");
		return -ENOMEM;
	}

	hcd->rsrc_start = r_mem->start;
	hcd->rsrc_len = resource_size(r_mem);
	IMAP_OHCI_DEBUG("USB Resource MEM is 0x%x \r\n",r_mem->start);
	IMAP_OHCI_DEBUG("USB Resource Len is 0x%x \r\n",hcd->rsrc_len);

	if(!request_mem_region(hcd->rsrc_start,hcd->rsrc_len,hcd_name))
	{
		dev_err(&pdev->dev,"request_mem_region failed");
		ret = -EBUSY;
		goto err_put;
	}

	hcd->regs = ioremap(hcd->rsrc_start,hcd->rsrc_len);
	if(!hcd->regs)
	{
		dev_err(&pdev->dev,"ioremap failed \r\n");
		ret = -ENOMEM;
		goto err_ioremap;
	}

	IMAP_OHCI_DEBUG("USB Register Map Address 0x%x \r\n",hcd->regs);

/*	valTmp = __raw_readl(rMEM_CFG);
	valTmp |= (1<<8);
	__raw_writel(valTmp,rMEM_CFG); 
	// epll setting
	valTmp = __raw_readl(rDIV_CFG2);
	valTmp &=~(3<<16);
	valTmp |= (2<<16);
	valTmp &=~(0x1f<<18);
	valTmp |=(9<<18);
	__raw_writel(valTmp,rDIV_CFG2); 
	
	// usb gate
	valTmp = __raw_readl(rPAD_CFG);
	valTmp &= ~0xe;
	__raw_writel(valTmp, rPAD_CFG);
	__raw_writel(0x0, rUSB_SRST);
	for(i=0;i<6000;i++);
	valTmp = __raw_readl(rPAD_CFG);
	valTmp |= 0xe;
	__raw_writel(valTmp,rPAD_CFG);
	mdelay(4);
	__raw_writel(0x5,rUSB_SRST);
	for(i=0;i<1000;i++);
	__raw_writel(0xf,rUSB_SRST);
*/
#if 0
	clk = clk_get(0,"usb-host");
	if(IS_ERR(clk))	
	{
		dev_err(&pdev->dev,"cannot get usb-host clock \r\n");
		ret = -ENOENT;
		goto err_mem;
	}
	//IMAP_OHCI_DEBUG("USB clock got enabled:: %ld.%03ld Mhz \r\n",print_mhz(clk_get_rate(clk)));
#endif
	imapx200_start_hc(pdev,hcd);

	ohci_hcd_init(hcd_to_ohci(hcd));

	IMAP_OHCI_DEBUG("ohci_hcd_init ok! \r\n");
	ret = usb_add_hcd(hcd,irq,IRQF_DISABLED);
	if(ret != 0)
	{
		dev_err(&pdev->dev,"usb_add_hcd failed \r\n");
		goto err_usb_clk;
	}
	IMAP_OHCI_DEBUG("--usb_hcd_imapx200_probe \r\n");

	return 0;

err_usb_clk:
	imapx200_stop_hc(pdev);

//err_clk:
	clk_put(clk);

//err_mem:
	iounmap(hcd->regs);

err_ioremap:
	release_mem_region(hcd->rsrc_start,hcd->rsrc_len);

err_put:
	usb_put_hcd(hcd);

	return ret;
}


static int ohci_imapx200_start(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;

	IMAP_OHCI_DEBUG("++ohci_imapx200_start \r\n");
	if((ret = ohci_init(ohci))<0) 
	{
		return ret;
	}
	IMAP_OHCI_DEBUG("++ohci_init success \r\n");
	
	if((ret = ohci_run(ohci)) < 0) 
	{
		err("ohci can't start %s",hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}
	IMAP_OHCI_DEBUG("--ohci_imapx200_start \r\n");

	return 0;
}

static const struct hc_driver ohci_imapx200_hc_driver = {
	.description = hcd_name,
	.product_desc = "Infotm Ohci Controller",
	.hcd_priv_size = sizeof(struct ohci_hcd),
	.irq = ohci_irq,
	.flags = HCD_USB11 | HCD_MEMORY,
	
	.start = ohci_imapx200_start,
	.stop = ohci_stop,
	.shutdown = ohci_shutdown,

	.urb_enqueue = ohci_urb_enqueue,
	.urb_dequeue = ohci_urb_dequeue,
	.endpoint_disable = ohci_endpoint_disable,
	.get_frame_number = ohci_get_frame,
	
	/*
	 * root hub support
	 */
	.hub_status_data = ohci_hub_status_data,
	.hub_control = ohci_hub_control,
#ifdef CONFIG_PM
	.bus_suspend = ohci_bus_suspend,
	.bus_resume = ohci_bus_resume,
#endif	
	.start_port_reset = ohci_start_port_reset,
};



static int ohci_hcd_imapx200_drv_probe(struct platform_device *pdev)
{
	return usb_hcd_imapx200_probe(&ohci_imapx200_hc_driver,pdev);
}

static int ohci_hcd_imapx200_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_imapx200_remove(hcd,pdev);
	return 0;
}

static struct platform_driver ohci_hcd_imapx200_driver = {
	.probe	= ohci_hcd_imapx200_drv_probe,
	.remove	= ohci_hcd_imapx200_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "imapx200_usbhost11",
	},
};
