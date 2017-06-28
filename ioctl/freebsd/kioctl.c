/*
 * kioctl - basic character device kernel module with ioctl.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file kioctl.c
 * \brief Character device kernel module with ioctl for FreeBSD.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kenv.h>
#include <sys/conf.h>
#include <sys/uio.h>

#include "kioctl.h"

/* forward declarations */
static d_open_t kioctl_open;
static d_close_t kioctl_close;
static d_read_t kioctl_read;
static d_ioctl_t kioctl_ioctl;

/**
 * \brief Name of the module.
 */
static const char* const name = "kioctl";

/**
 * \brief Character device specifications.
 */
static struct cdevsw kioctl_cdevsw =
{
    .d_version = D_VERSION,
    .d_open = kioctl_open,
    .d_close = kioctl_close,
    .d_read = kioctl_read,
    .d_ioctl = kioctl_ioctl,
    .d_name = "kioctl",
};

/**
 * \brief The character device.
 */
static struct cdev* kioctl_cdev = NULL;

/**
 * \brief Mutex to have only one process to open and use device.
 */
static struct mtx mutex_kioctl;

/**
 * \brief Number of times device is opened.
 */
size_t g_number_open = 0;

/**
 * \brief The value to get/set with ioctl.
 */
uint32_t g_value = 0;

/**
 * \brief Open callback for character device.
 * \param dev device pointer.
 * \param oflags flags.
 * \param devtype device type.
 * \param thread thread information.
 * \return 0 if success, negative value otherwise.
 */
static int kioctl_open(struct cdev* dev, int oflags, int devtype,
    struct thread* td)
{
    int err = 0;

    if(!mtx_trylock(&mutex_kioctl))
    {
        printf("%s mutex already locked!\n", name);
        return -EBUSY;
    }

    g_number_open++;
    printf("%s: open (%zu)\n", name, g_number_open);
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
static int kioctl_close(struct cdev* dev, int oflags, int devtype,
        struct thread* td)
{
    int err = 0;

    g_number_open--;
    mtx_unlock(&mutex_kioctl);

    printf("%s: close (%zu)\n", name, g_number_open);
    return err;
}

/**
 * \brief Read callback for character device.
 * \param dev device pointer.
 * \param uio userspace I/O.
 * \param ioflags flags.
 * \return 0 if success, negative number otherwise.
 */
static int kioctl_read(struct cdev* dev, struct uio* uio, int ioflags)
{
    int err = 0;
    ssize_t len_msg = 0;
    ssize_t len = uio->uio_resid;
    off_t offset = uio->uio_offset;
    char message[128];

    printf("%s: wants to read %zu bytes from %ld offset\n", name,
        len, offset);
    
    snprintf(message, sizeof(message), "%u\n", g_value);
    message[sizeof(message) - 1] = 0x00;

    /* calculate buffer size left to copy */
    len_msg = strlen(message) - offset;

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

    err = uiomove(message + offset, len_msg, uio);
    if(err == 0)
    {
        printf("%s: sent %zu characters to user\n", name, len_msg);
        return 0;
    }

    printf("%s: failed to send %zu characters to user\n", name, len_msg);
    return -EFAULT;
}


/**
 * \brief Ioctl callback for character device.
 * \param dev device pointer.
 * \param cmd command to pass.
 * \param arg argument.
 * \param fflags flags.
 * \param thread thread information.
 * \return 0 if success, negative number if error.
 */
static int kioctl_ioctl(struct cdev* dev, u_long cmd, caddr_t arg, int fflags,
        struct thread* td)
{
    switch(cmd)
    {
    case KIOCGVAL:
        *((uint32_t*)arg) = g_value;
        break;
    case KIOCSVAL:
        g_value = *((uint32_t*)arg);
        break;
    default:
        return -ENOTTY;
        break;
    }
    return 0;
}

/**
 * \brief Module loader.
 * \param m module pointer.
 * \param evt event (load, unload...).
 * \param arg additionnal argument.
 * \return 0 if success, error code otherwise.
 */
static int kioctl_loader(struct module* m, int evt, void* arg)
{
  int err = 0;

  switch(evt)
  {
  case MOD_LOAD:
    printf("%s: initialization\n", name);

    err = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
        &kioctl_cdev, &kioctl_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666,
        "kioctl");

    if(err != 0)
    {
        break;
    }

    mtx_init(&mutex_kioctl, "Chardev lock", NULL, MTX_DEF);
    break;
  case MOD_UNLOAD:

    mtx_destroy(&mutex_kioctl);
    if(kioctl_cdev)
    {
        destroy_dev(kioctl_cdev);
    }
    
    printf("%s: finalization\n", name);
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
static moduledata_t kioctl_mod =
{
  "kioctl", /* name of the module */
  kioctl_loader, /* callback for event */
  NULL /* argument */
};

DECLARE_MODULE(kioctl, kioctl_mod, SI_SUB_KLD, SI_ORDER_ANY);

