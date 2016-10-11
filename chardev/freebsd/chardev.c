/*
 * chardev - basic character device FreeBSD kernel module.
 * Copyright (c) 2016, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file chardev.c
 * \brief Character device kernel module for FreeBSD.
 * \author Sebastien Vincent
 * \date 2016
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kenv.h>
#include <sys/conf.h>
#include <sys/uio.h>

/* forward declarations */
static d_open_t chardev_open;
static d_close_t chardev_close;
static d_write_t chardev_write;
static d_read_t chardev_read;

/**
 * \brief Name of the module.
 */
static char name[1024] = "chardev";

/**
 * \brief Cookie value.
 */
static int cookie = 0;

/**
 * \brief Character device specifications.
 */
static struct cdevsw chardev_cdevsw =
{
    .d_version = D_VERSION,
    .d_open = chardev_open,
    .d_close = chardev_close,
    .d_write = chardev_write,
    .d_read = chardev_read,
    .d_name = "chardev",
};

/**
 * \brief The character device.
 */
static struct cdev* chardev_cdev = NULL;

/**
 * \brief Number of times device is opened.
 */
size_t number_open = 0;

/**
 * \brief Message buffer in kernel side for the device.
 */
static char message[1024];

/**
 * \brief Size of message stored in kernel side.
 */
static size_t message_size = 0;

/**
 * \brief Open callback for character device.
 * \param dev device pointer.
 * \param oflags flags.
 * \param devtype device type.
 * \param thread thread information.
 * \return 0 if success, negative value otherwise.
 */
static int chardev_open(struct cdev* dev, int oflags, int devtype,
    struct thread* td)
{
    int err = 0;

    number_open++;
    printf("%s.%d: open (%zu)\n", name, cookie, number_open);
    return err;
}

/**
 * \brief Close callback for character device.
 * \param dev device pointer.
 * \param oflags flags.
 * \param devtype device type.
 * \param thread thread information.
 * \return 0 if success, negative value otherwise.
 */
static int chardev_close(struct cdev* dev, int oflags, int devtype,
        struct thread* td)
{
    int err = 0;

    number_open--;
    printf("%s.%d: close (%zu)\n", name, cookie, number_open);
    return err;
}

/**
 * \brief Read callback for character device.
 * \param dev device pointer.
 * \param uio userspace I/O.
 * \param ioflags flags.
 * \return 0 if success, negative number otherwise.
 */
static int chardev_read(struct cdev* dev, struct uio* uio, int ioflags)
{
    int err = 0;

    printf("%s.%d: wants to read at most %zu bytes\n", name, cookie,
        uio->uio_resid);

    err = uiomove(message, message_size, uio);
    if(err == 0)
    {
        size_t snd = message_size;

        message_size = 0;
        printf("%s.%d: sent %zu characters to user\n", name, cookie, snd);
        return 0;
    }

    printf("%s.%d: failed to send %zu characters to user\n", name, cookie,
        message_size);
    return -EFAULT;
}

/**
 * \brief Write callback for character device.
 * \param dev device pointer.
 * \param uio userspace I/O.
 * \param ioflags flags.
 * \return 0 if success, negative number otherwise.
 */
static int chardev_write(struct cdev* dev, struct uio* uio, int ioflags)
{
    int err = 0;
    size_t resid = uio->uio_resid;

    if(resid > sizeof(message) - 1)
    {
        return -EFBIG;
    }

    message_size = uio->uio_resid;
    err = uiomove(message, uio->uio_resid, uio);

    if(err != 0)
    {
        message_size = 0;
        return -EFAULT;
    }

    printf("%s.%d: received %zu characters from user\n", name, cookie, resid);
    return 0;
}

/**
 * \brief Module loader.
 * \param m module pointer.
 * \param evt event (load, unload...).
 * \param arg additionnal argument.
 * \return 0 if success, error code otherwise.
 */
static int chardev_loader(struct module* m, int evt, void* arg)
{
  int err = 0;

  switch(evt)
  {
  case MOD_LOAD:
    TUNABLE_STR_FETCH("chardev.name", name, sizeof(name));
    TUNABLE_INT_FETCH("chardev.cookie", &cookie);
    printf("%s.%d: initialization\n", name, cookie);

    err = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
        &chardev_cdev, &chardev_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666,
        "chardev");

    if(err != 0)
    {
        break;
    }

    break;
  case MOD_UNLOAD:
    if(chardev_cdev)
    {
        destroy_dev(chardev_cdev);
    }
    
    printf("%s.%d: finalization\n", name, cookie);
    break;
  default:
    err = EOPNOTSUPP;
    break;
  }

  return err;
}

/**
 * \brief Module structure for kernel registration.
 */
static moduledata_t chardev_mod =
{
  "chardev", /* name of the module */
  chardev_loader, /* callback for event */
  NULL /* argument */
};

DECLARE_MODULE(chardev, chardev_mod, SI_SUB_KLD, SI_ORDER_ANY);

