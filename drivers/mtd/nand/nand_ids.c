/*
 *  drivers/mtd/nandids.c
 *
 *  Copyright (C) 2002 Thomas Gleixner (tglx@linutronix.de)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/module.h>
#include <linux/mtd/nand.h>
/*
*	Chip ID list
*
*	Name. ID code, pagesize, chipsize in MegaByte, eraseblock size,
*	options
*
*	Pagesize; 0, 256, 512
*	0	get this information from the extended chip ID
+	256	256 Byte page size
*	512	512 Byte page size
*/
struct nand_flash_dev nand_flash_ids[] = {

#define LP_OPTIONS (NAND_SAMSUNG_LP_OPTIONS | NAND_NO_READRDY | NAND_NO_AUTOINCR)
#define LP_OPTIONS16 (LP_OPTIONS | NAND_BUSWIDTH_16)
    //name              id_len      id                                  //chipsize
    {"K9GAG08U0M",      5,          {0xEC,0xD5,0x14,0xB6,0x74,0xec},    2048,   LP_OPTIONS},

    //soft ecc support
    {"K9LBG08U0D",      6,          {0xEC,0xD7,0xD5,0x29,0x38,0x41},    4096,   LP_OPTIONS},
    {"H27UAG8T2MTR",    5,          {0xad,0xd5,0x14,0xb6,0x44,0xad},    2048,   LP_OPTIONS},
    //unsupport
    //{"HY27UU088G5M",    5,          {0xad,0xd5,0x94,0x25,0x44,0x41},    1024,   LP_OPTIONS},
    {"K9GAG08U0E",      6,	        {0xEC,0xD5,0x84,0x72,0x50,0x42},    2048,   LP_OPTIONS},
	{NULL,}
};

/*
*	Manufacturer ID list
*/
struct nand_manufacturers nand_manuf_ids[] = {
	{NAND_MFR_TOSHIBA, "Toshiba"},
	{NAND_MFR_SAMSUNG, "Samsung"},
	{NAND_MFR_FUJITSU, "Fujitsu"},
	{NAND_MFR_NATIONAL, "National"},
	{NAND_MFR_RENESAS, "Renesas"},
	{NAND_MFR_STMICRO, "ST Micro"},
	{NAND_MFR_HYNIX, "Hynix"},
	{NAND_MFR_MICRON, "Micron"},
	{NAND_MFR_AMD, "AMD"},
	{0x0, "Unknown"}
};

EXPORT_SYMBOL(nand_manuf_ids);
EXPORT_SYMBOL(nand_flash_ids);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Thomas Gleixner <tglx@linutronix.de>");
MODULE_DESCRIPTION("Nand device & manufacturer IDs");
