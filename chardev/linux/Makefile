ifneq (${KERNELRELEASE},)

	obj-m += chardev.o
	obj-m += chardev2.o
else

	KERNELDIR        ?= /lib/modules/$(shell uname -r)/build
	MODULE_DIR       ?= $(shell pwd)
	ARCH             ?= $(shell uname -i)
	CROSS_COMPILE    ?=
	INSTALL_MOD_PATH ?= /

all: modules

modules:
	${MAKE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C ${KERNELDIR} SUBDIRS=${MODULE_DIR}  modules

modules_install:
	${MAKE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} INSTALL_MOD_PATH=${INSTALL_MOD_PATH} -C ${KERNELDIR} SUBDIRS=${MODULE_DIR}  modules_install
	cp 99-chardev.rules /etc/udev/rules.d/
	chmod 644 /etc/udev/rules.d/99-chardev.rules
	depmod -a

modules_uninstall:
	rm /etc/udev/rules.d/99-chardev.rules
	rm /lib/modules/$(shell uname -r)/extra/chardev.ko
	rm /lib/modules/$(shell uname -r)/extra/chardev2.ko
	depmod -a

clean:
	rm -f *.o *.ko *.mod.c .*.o .*.ko .*.mod.c .*.cmd *~
	rm -f Module.symvers Module.markers modules.order
	rm -rf .tmp_versions
endif

