extern int nand_correct_data_bch(struct mtd_info *mtd, unsigned char *buf,
   unsigned char *read_ecc, unsigned char *calc_ecc);
extern int nand_calculate_ecc_bch(struct mtd_info *mtd, const unsigned char *buf,
   unsigned char *code);
extern void bch_init(void);
