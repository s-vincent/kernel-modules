SRCS=chardev.c
KMOD=chardev

.include <bsd.kmod.mk>

uninstall:
	rm /boot/kernel/chardev.ko
	kldxref /boot/kernel

