# kernel-modules
Kernel modules samples for GNU/Linux and FreeBSD OS.

## Contents
This repository contains the following samples:

* helloworld: simple hello world module (init/exit handlers);
* chardev: basic character device that handles open/close/read/write operations
  (i.e. bufferize data with write() and returns them with read());
* proc (GNU/Linux only): simple /proc entry;
* ioctl: character device with ioctl support.

## License

All modules are under BSD-3 license.

