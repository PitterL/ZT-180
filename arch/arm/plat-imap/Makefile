# arch/arm/plat-imap/Makefile
#
# Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
#
# Licensed under GPLv2


obj-y				:=
obj-m				:=
obj-n				:=
obj-				:=


# Core files

obj-y				+= cpu.o gpio.o
obj-y				+= dmalib.o
obj-y				+= dmaif_linux.o
#obj-$(CONFIG_PM)				+= pm-gpio.o
obj-$(CONFIG_PM)				+= pm_imap.o
obj-$(CONFIG_PM)			+= pm_imapx200.o
obj-$(CONFIG_PM)				+= sleep.o

#obj-y				+= dma.o

obj-y				+= clock.o
obj-y				+= mem_reserve.o
obj-y				+= debug.o
# Architecture dependant builds

