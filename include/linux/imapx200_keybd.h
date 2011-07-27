#ifndef __imapx200_keybd_H
#define __imapx200_keybd_H

#define MAX_MATRIX_KEY_ROWS	(18)
#define MAX_MATRIX_KEY_COLS	(8)
#define MAX_MATRIX_KEY_NUM	(MAX_MATRIX_KEY_ROWS * MAX_MATRIX_KEY_COLS)


struct imapx200_keybd_platform_data{
	unsigned int matrix_key_rows;
	unsigned int matrix_key_cols;

	unsigned int *matrix_key_map;

	int matrix_key_map_size;

};

#define KEY(row, col, val)	(((row) << 24) | ((col) << 20) | (val))

extern void imapx200_set_keybd_info(struct imapx200_keybd_platform_data *info);
 
#endif 
