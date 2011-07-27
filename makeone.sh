#!/bin/sh

cd $MK_ROOT
if [ ! -d $MK_VERSION ];then
mkdir $MK_VERSION
fi
cd ..

cd cpio
rm ${MK_CPIO_NAME}
cd ${MK_CPIO_SRC}
find . |cpio -o -H newc >../${MK_CPIO_NAME}
cd ../../

rm imap.cpio
cp cpio/${MK_CPIO_NAME} imap.cpio
make ${MK_CONFIG}
cp .config arch/arm/configs/${MK_CONFIG}
./install.sh
mv zImage ${MK_ROOT}/${MK_VERSION}/
mv zImage_adb ${MK_ROOT}/${MK_VERSION}/


