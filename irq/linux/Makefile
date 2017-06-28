ifneq (${KERNELRELEASE},)

	obj-m += irq-handler.o
	obj-m += irq-tasklet.o
	obj-m += irq-workqueue.o
	obj-m += irq-threaded.o
else

	KERNELDIR        ?= /lib/modules/$(shell uname -r)/build

	ifneq (${KERNEL_SRC},)
		KERNELDIR := ${KERNEL_SRC}
	endif

	MODULE_DIR       ?= $(shell pwd)
	ARCH             ?= $(shell uname -i)
	
	ifeq ($(ARCH),unknown)
		ARCH := $(shell uname -m | sed -e s/i.86/x86/ \
				  -e s/sun4u/sparc64/ \
				  -e s/arm.*/arm/ -e s/sa110/arm/ \
				  -e s/s390x/s390/ -e s/parisc64/parisc/ \
				  -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
				  -e s/sh[234].*/sh/ -e s/aarch64.*/arm64/ )
	endif

	CROSS_COMPILE    ?=
	INSTALL_MOD_PATH ?= /

all: modules

modules:
	${MAKE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C ${KERNELDIR} SUBDIRS=${MODULE_DIR}  modules

modules_install:
	${MAKE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} INSTALL_MOD_PATH=${INSTALL_MOD_PATH} -C ${KERNELDIR} SUBDIRS=${MODULE_DIR}  modules_install
	cp 99-irq-handler.rules /etc/udev/rules.d/
	chmod 644 /etc/udev/rules.d/99-irq-handler.rules
	depmod -a

modules_uninstall:
	rm /lib/modules/$(shell uname -r)/extra/irq-handler.ko
	rm /lib/modules/$(shell uname -r)/extra/irq-tasklet.ko
	rm /lib/modules/$(shell uname -r)/extra/irq-workqueue.ko
	rm /lib/modules/$(shell uname -r)/extra/irq-threaded.ko
	depmod -a

clean:
	rm -f *.o *.ko *.mod.c .*.o .*.ko .*.mod.c .*.cmd *~
	rm -f Module.symvers Module.markers modules.order
	rm -rf .tmp_versions
endif

