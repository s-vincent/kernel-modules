obj-m += chardev.o

all: 
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) clean

install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) modules_install
	cp 99-chardev.rules /etc/udev/rules.d/
	chmod 644 /etc/udev/rules.d/99-chardev.rules
	depmod -a

uninstall:
	rm /etc/udev/rules.d/99-chardev.rules
	rm /lib/modules/$(shell uname -r)/extra/chardev.ko
	depmod -a

