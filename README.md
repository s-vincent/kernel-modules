# kernel-modules

Kernel modules samples for GNU/Linux, FreeBSD and NetBSD.

## Contents

This repository contains the following samples for both GNU/Linux and FreeBSD:

* helloworld: simple hello world module (init/exit handlers);
* chardev: basic character device that handles open/close/read/write operations
  (i.e. bufferize data with write() and returns them with read());
* ioctl: character device with ioctl support.

For GNU/Linux only:
* irq: different ways to handle IRQ (handler, tasklet, workqueue, threaded);
* proc: simple /proc entry;
* thread: thread worker;
* timer: simple timer and high-resolution timer;
* waitqueue: simple waitqueue notification for character device;
* mmap: character device with mmap to share kernel buffer.

## License

All modules are under BSD-3 license.

