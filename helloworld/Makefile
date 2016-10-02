obj-m += helloworld.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) clean

install:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) modules_install
	depmod -a

uninstall:
	rm /lib/modules/$(shell uname -r)/extra/helloworld.ko
	depmod -a

