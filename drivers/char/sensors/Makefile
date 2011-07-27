# sololz
# super mountain callage

obj-$(CONFIG_SENSOR_2AXIS) += sensor_2axis.o
obj-$(CONFIG_SENSOR_ACC) += sensor_aac.o

EXTRA_CFLAGS += -DLINUX
EXTRA_CFLAGS += -DDIVX_ENABLE

ifeq ($(CONFIG_IMAP_DECODE_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
endif

