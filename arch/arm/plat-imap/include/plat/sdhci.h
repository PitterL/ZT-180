/* 
 * arch/arm/plat_imap/include/plat/sdhci.h
 *
 *
 */
#include <linux/mmc/host.h>
/*
 char *imapx_sdi_clocks[3] =
{
	[0] = "hsmmc",
	[1] = "hsmmc",
	[2] = "mmc_bus",
};
*/

struct imapx_sdi_platdata {
	int		hw_port;
	unsigned	width;
	unsigned	caps;
	char 		**clocks;
};



