obj-m += chardev.o
CFLAGS_BIN =-std=c11 -Wall -Wextra -Werror -Wstrict-prototypes \
			-Wredundant-decls -Wshadow -pedantic -pedantic-errors \
			-fno-strict-aliasing -D_XOPEN_SOURCE=700 -O2 
BIN = chardev_userspace

all: $(BIN) module

.c.o:
	$(CC) -c $(CFLAGS_BIN) $< -o $@

$(BIN): $(BIN).o
	$(CC) -o $(BIN) -O $(BIN).o

module:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(CURDIR) modules

clean:
	rm -f $(BIN) *.o
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

