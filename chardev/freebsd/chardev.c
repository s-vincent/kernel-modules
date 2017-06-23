/*
 * chardev - basic character device kernel module.
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
 * \brief Name of the module (configuration parameter).
 */
static char name[1024] = "chardev";

/**
 * \brief Cookie value (configuration parameter).
 */
static int cookie = 0;

/**
 * \brief Mutex to have only one process to open and use device.
 */
static struct mtx mutex_chardev;

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
size_t g_number_open = 0;

/**
 * \brief Message buffer in kernel side for the device.
 */
static char g_message[1024];

/**
 * \brief Size of g_message stored in kernel side.
 */
static size_t g_message_size = 0;

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

    g_number_open++;
    printf("%s.%d: open (%zu)\n", name, cookie, g_number_open);
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

    g_number_open--;
    printf("%s.%d: close (%zu)\n", name, cookie, g_number_open);
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
    ssize_t len_msg = 0;
    ssize_t len = uio->uio_resid;
    off_t offset = uio->uio_offset;

    printf("%s.%d: wants to read %zu bytes from %ld offset\n", name, cookie,
        len, offset);

    /* calculate buffer size left to copy */
    len_msg = g_message_size - offset;

    if(len_msg == 0)
    {
        /* EOF */
        return 0;
    }
    else if(len_msg > len)
    {
        len_msg = len;
    }
    else if(len_msg < 0)
    {
        return -EINVAL;
    }

    err = uiomove(g_message + offset, len_msg, uio);
    if(err == 0)
    {
        printf("%s.%d: sent %zu characters to user\n", name, cookie, len_msg);
        return 0;
    }

    printf("%s.%d: failed to send %zu characters to user\n", name, cookie,
        len_msg);
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
    ssize_t len_msg = uio->uio_resid + uio->uio_offset;
    ssize_t len = uio->uio_resid;
    off_t offset = uio->uio_offset;

    printf("%s.%d: wants to write %zu bytes from %ld offset\n", name, cookie,
        len, offset);

    if(len_msg > sizeof(g_message))
    {
        return -EFBIG;
    }

    err = uiomove(g_message + offset, len, uio);

    if(err != 0)
    {
        g_message_size = 0;
        return -EFAULT;
    }

    if(offset == 0)
    {
        g_message_size = 0;
    }

    g_message_size += len;

    printf("%s.%d: received %zu characters from user\n", name, cookie, len);
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

    mtx_init(&mutex_chardev, "Chardev lock", NULL, MTX_DEF);
    break;
  case MOD_UNLOAD:

    mtx_destroy(&mutex_chardev);
    if(chardev_cdev)
    {
        destroy_dev(chardev_cdev);
    }
    
    printf("%s.%d: finalization\n", name, cookie);
    break;
  case MOD_QUIESCE:
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

