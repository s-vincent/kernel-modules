SRCS=helloworld.c
KMOD=helloworld

.include <bsd.kmod.mk>

uninstall:
	rm /boot/kernel/helloworld.ko
	kldxref /boot/kernel

