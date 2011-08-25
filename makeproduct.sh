#!/bin/sh


chmod 777 -R cpio
export MK_ROOT=out

if [ ! -d $MK_ROOT ];then
mkdir $MK_ROOT
else
rm -rf $MK_ROOT/*
fi

# B0 batt35 1024 576
export MK_VERSION=B0_batt35_1024_576
export MK_CPIO_NAME=imap.cpio.mtd
export MK_CPIO_SRC=src_mtd
export MK_CONFIG=ZT180_B0_batt35_1024_576_defconfig
./makeone.sh

# B0 batt35 1024 600
export MK_VERSION=B0_batt35_1024_600
export MK_CPIO_NAME=imap.cpio.mtd
export MK_CPIO_SRC=src_mtd
export MK_CONFIG=ZT180_B0_batt35_1024_600_defconfig
./makeone.sh

# B0 batt35 1024 576 GPS
export MK_VERSION=B0_batt35_1024_576_GPS
export MK_CPIO_NAME=imap.cpio.mtd.gps
export MK_CPIO_SRC=src_mtd_gps
export MK_CONFIG=ZT180_B0_batt35_1024_576_defconfig
./makeone.sh

# B0 batt35 1024 600 GPS
export MK_VERSION=B0_batt35_1024_600_GPS
export MK_CPIO_NAME=imap.cpio.mtd.gps
export MK_CPIO_SRC=src_mtd_gps
export MK_CONFIG=ZT180_B0_batt35_1024_600_defconfig
./makeone.sh

# B0 1024 576
export MK_VERSION=B0_1024_576
export MK_CPIO_NAME=imap.cpio.mtd
export MK_CPIO_SRC=src_mtd
export MK_CONFIG=ZT180_B0_1024_576_defconfig
./makeone.sh

# B0 1024 600
export MK_VERSION=B0_1024_600
export MK_CPIO_NAME=imap.cpio.mtd
export MK_CPIO_SRC=src_mtd
export MK_CONFIG=ZT180_B0_1024_600_defconfig
./makeone.sh

# B0 1024 576 GPS
export MK_VERSION=B0_1024_576_GPS
export MK_CPIO_NAME=imap.cpio.mtd.gps
export MK_CPIO_SRC=src_mtd_gps
export MK_CONFIG=ZT180_B0_1024_576_defconfig
./makeone.sh

# B0 1024 600 GPS
export MK_VERSION=B0_1024_600_GPS
export MK_CPIO_NAME=imap.cpio.mtd.gps
export MK_CPIO_SRC=src_mtd_gps
export MK_CONFIG=ZT180_B0_1024_600_defconfig
./makeone.sh

# E3
export MK_VERSION=E3
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E3_defconfig
./makeone.sh

# E3 batt35
export MK_VERSION=E3_batt35
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E3_batt35_defconfig
./makeone.sh

# E4
export MK_VERSION=E4
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E4_defconfig
./makeone.sh

# E4 ES8388
export MK_VERSION=E4_ES8388
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E4_ES8388_defconfig
./makeone.sh

# E5
export MK_VERSION=E5
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E5_defconfig
./makeone.sh

# E5 ES8388
export MK_VERSION=E5_ES8388
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E5_ES8388_defconfig
./makeone.sh
# G0
export MK_VERSION=G0
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G0_defconfig
./makeone.sh

# G0 batt35
export MK_VERSION=G0_batt35
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G0_batt35_defconfig
./makeone.sh
# G2
export MK_VERSION=G2
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_defconfig
./makeone.sh

# G2 ES8388
export MK_VERSION=G2_ES8388
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_ES8388_defconfig
./makeone.sh

# G2 batt35
export MK_VERSION=G2_batt35
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_batt35_defconfig
./makeone.sh

# G2 nohdmi no camara
export MK_VERSION=G2_nohdmi_nocamara
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_nohdmi_nocamera_defconfig
./makeone.sh

# G2 nohdmi no camara ES8388
export MK_VERSION=G2_nohdmi_nocamara_ES8388
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_nohdmi_nocamera_ES8388_defconfig
./makeone.sh

# G2 nohdmi no camara batt35
export MK_VERSION=G2_batt35_nohdmi_nocamara
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_nohdmi_nocamera_batt35_defconfig
./makeone.sh

# H0
export MK_VERSION=H0
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_H0_defconfig
./makeone.sh



chmod 777 -R ./$MK_ROOT
