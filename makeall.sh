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

# E5
export MK_VERSION=E5
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_E5_defconfig
./makeone.sh

# F0
export MK_VERSION=F0
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_F0_defconfig
./makeone.sh

# G0 3G
export MK_VERSION=G0_3G
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G0_3G_defconfig
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

# G0 nohdmi no camara
export MK_VERSION=G0_nohdmi_nocamara
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G0_nohdmi_nocamera_defconfig
./makeone.sh

# G0 nohdmi no camara batt35
export MK_VERSION=G0_nohdmi_nocamara_batt35
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G0_nohdmi_nocamera_batt35_defconfig
./makeone.sh

# G1
export MK_VERSION=G1
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G1_defconfig
./makeone.sh

# G1 nohdmi no camara
export MK_VERSION=G1_nohdmi_nocamara
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G1_nohdmi_defconfig
./makeone.sh

# G2
export MK_VERSION=G2
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_G2_defconfig
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

# I0 1024
export MK_VERSION=I0_1024
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_I0_1024_defconfig
./makeone.sh

# I0
export MK_VERSION=I0
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_I0_defconfig
./makeone.sh

# J0 3G 1024 600
export MK_VERSION=J0_3G_1024_600
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_J0_3G_1024_600_defconfig
./makeone.sh

# J0 3G 1024 768
export MK_VERSION=J0_3G_1024_768
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_J0_3G_1024_768_defconfig
./makeone.sh

# J0 1024 768
export MK_VERSION=J0_1024_768
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_J0_1024_768_defconfig
./makeone.sh

# J0 1024 600
export MK_VERSION=J0_1024_600
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_J0_1024_600_defconfig
./makeone.sh

# J0
export MK_VERSION=J0
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_J0_defconfig
./makeone.sh

# K0
export MK_VERSION=K0
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_K0_defconfig
./makeone.sh

# K0 NAS
export MK_VERSION=K0_NAS
export MK_CPIO_NAME=imap.cpio
export MK_CPIO_SRC=src
export MK_CONFIG=ZT180_K0_NAS_defconfig
./makeone.sh

# K0 NAS GPS
export MK_VERSION=K0_NAS_GPS
export MK_CPIO_NAME=imap.cpio.gps
export MK_CPIO_SRC=src_gps
export MK_CONFIG=ZT180_K0_NAS_defconfig
./makeone.sh



chmod 777 -R ./$MK_ROOT
