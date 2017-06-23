SRCS=kioctl.c
KMOD=kioctl

.include <bsd.kmod.mk>

uninstall:
	rm /boot/kernel/kioctl.ko
	kldxref /boot/kernel

