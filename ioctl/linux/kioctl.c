/*
 * kioctl - character device kernel module with ioctl.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file kioctl.c
 * \brief Character device module with ioctl for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "kioctl.h"

/* forward declarations */
static int kioctl_open(struct inode* inodep, struct file* filep);
static int kioctl_release(struct inode* inodep, struct file* filep);
static ssize_t kioctl_read(struct file* filep, char* buffer, size_t len,
        loff_t* offset);
static long kioctl_ioctl(struct file* filep, unsigned int cmd,
        unsigned long arg);

/**
 * \brief Value to be set/get by ioctl.
 */
static uint32_t g_value = 0;

/**
 * \brief Number of times device is opened.
 */
static size_t g_number_open = 0;

/**
 * \brief Mutex to have only one process to open and use device.
 */
static DEFINE_MUTEX(mutex_kioctl);

/**
 * \brief File operations.
 */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = kioctl_open,
    .release = kioctl_release,
    .read = kioctl_read,
    .unlocked_ioctl = kioctl_ioctl,
};

/**
 * \brief Structure to register the misc driver.
 */
static struct miscdevice kioctl_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = THIS_MODULE->name,
    .fops  = &fops,
};

/**
 * \brief Open callback for character device.
 * \param inodep inode.
 * \param filep file.
 * \return 0 if success, negative value otherwise.
 */
static int kioctl_open(struct inode* inodep, struct file* filep)
{
    if(!mutex_trylock(&mutex_kioctl))
    {
        printk(KERN_ALERT "%s mutex already locked!\n", THIS_MODULE->name);
        return -EBUSY;
    }
    g_number_open++;
    printk(KERN_INFO "%s: open (%zu)\n", THIS_MODULE->name, g_number_open);
    return 0;
}

/**
 * \brief Release callback for character device.
 * \param inodep inode.
 * \param filep file.
 * \return 0 if success, negative value otherwise.
 */
static int kioctl_release(struct inode* inodep, struct file* filep)
{
    g_number_open--;
    printk(KERN_INFO "%s: release (%zu)\n", THIS_MODULE->name, g_number_open);
    mutex_unlock(&mutex_kioctl);
    return 0;
}

/**
 * \brief Read callback for character device.
 * \param filep file.
 * \param u_buffer buffer to fill.
 * \param len length to read.
 * \param offset offset of the buffer.
 * \return number of characters read, or negative value if failure.
 */
static ssize_t kioctl_read(struct file* filep, char* u_buffer, size_t len,
        loff_t* offset)
{
    int err = 0;
    ssize_t len_msg = 0;
    char message[128]; /* should be sufficient to display a uint32_t */

    printk(KERN_INFO "%s: wants to read %zu bytes from offset %lld\n",
            THIS_MODULE->name, len, *offset);

    snprintf(message, sizeof(message), "%u\n", g_value);
    message[sizeof(message) - 1] = 0x00;

    /* calculate buffer size left to copy */
    len_msg = strlen(message) - *offset;

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

    err = copy_to_user(u_buffer, message + *offset, len_msg);

    if(err == 0)
    {
        /* success */
        printk(KERN_DEBUG "%s sent %zu characters to user\n", THIS_MODULE->name,
                len_msg);

        *offset += len_msg;
        return len_msg;
    }
    else
    {
        printk(KERN_DEBUG "%s failed to send %zu characters to user\n",
                THIS_MODULE->name, len_msg);
        return -EFAULT;
    }
}

/**
 * \brief Ioctl callback for character device.
 * \param filep file.
 * \param cmd command to pass.
 * \param arg argument.
 * \return 0 if success, negative number if error.
 */
static long kioctl_ioctl(struct file* filep, unsigned int cmd, unsigned long arg)
{
    if(_IOC_TYPE(cmd) != KIOCTL_IOCTL_MAGIC)
    {
        return -ENOTTY;
    }

    switch(_IOC_NR(cmd))
    {
    case KIOCTL_GET_VALUE:
        if(copy_to_user((void*)arg, &g_value, sizeof(g_value)) != 0)
        {
            return -EFAULT;
        }
        break;
    case KIOCTL_SET_VALUE:
        if(copy_from_user(&g_value, (void*)arg, sizeof(g_value)) != 0)
        {
            return -EFAULT;
        }
        break;
    default:
        return -ENOTTY;
        break;
    }
    return 0;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init kioctl_init(void)
{
    int ret = 0;

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    /* register device */
    ret = misc_register(&kioctl_misc);

    if(ret == 0)
    { 
        printk(KERN_INFO "%s device created correctly\n", THIS_MODULE->name);
    }

    return ret;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit kioctl_exit(void)
{
    mutex_destroy(&mutex_kioctl);
    misc_deregister(&kioctl_misc);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(kioctl_init);
module_exit(kioctl_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("character device module with ioctl");
MODULE_VERSION("0.1");

