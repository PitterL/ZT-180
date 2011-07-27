#!/bin/sh

sed -i 's/CONFIG_USB_ANDROID_ADB=y/CONFIG_USB_ANDROID_MASS_STORAGE=y/g' .config
sed -i 's/# CONFIG_USB_ANDROID_MASS_STORAGE is not set/# CONFIG_USB_ANDROID_ADB is not set/g' .config
make ARCH=arm CROSS_COMPILE="ccache arm-infotm-linux-gnueabi-" uImage -j 4
./mkimage -A arm -O linux -T kernel -C none -a 0x40007fc0 -e 0x40008000 -n Linux-2.6.32.9 -d arch/arm/boot/zImage zImage
#cp zImage /tftpboot/zImage



sed -i 's/CONFIG_USB_ANDROID_MASS_STORAGE=y/CONFIG_USB_ANDROID_ADB=y/g' .config
sed -i 's/# CONFIG_USB_ANDROID_ADB is not set/# CONFIG_USB_ANDROID_MASS_STORAGE is not set/g' .config

make ARCH=arm CROSS_COMPILE="ccache arm-infotm-linux-gnueabi-" uImage -j 4
./mkimage -A arm -O linux -T kernel -C none -a 0x40007fc0 -e 0x40008000 -n Linux-2.6.32.9 -d arch/arm/boot/zImage zImage_adb
#cp zImage_adb /tftpboot/zImage_adb
