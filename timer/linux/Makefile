ifneq (${KERNELRELEASE},)

	obj-m += ktimer.o
	obj-m += timerhr.o
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
	depmod -a

uninstall:
	rm /lib/modules/$(shell uname -r)/extra/ktimer.ko
	rm /lib/modules/$(shell uname -r)/extra/timerhr.ko
	depmod -a

clean:
	rm -f *.o *.ko *.mod.c .*.o .*.ko .*.mod.c .*.cmd *~
	rm -f Module.symvers Module.markers modules.order
	rm -rf .tmp_versions
endif


