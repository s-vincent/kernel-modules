/*
 * chardev2 - basic character device kernel module.
 * Copyright (c) 2016-2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file chardev2.c
 * \brief Basic character device module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2016-2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <linux/uaccess.h>

/* forward declarations */
static int chardev_open(struct inode* inodep, struct file* filep);
static int chardev_release(struct inode* inodep, struct file* filep);
static ssize_t chardev_write(struct file* filep, const char* buffer,
        size_t len, loff_t* offset);
static ssize_t chardev_read(struct file* filep, char* buffer, size_t len,
        loff_t* offset);

/**
 * \brief Message in kernel side for the device.
 */
static char g_message[1024] = {0};

/**
 * \brief Size of g_message stored in kernel side.
 */
static size_t g_message_size = 0;

/**
 * \brief Number of times device is opened.
 */
static size_t g_number_open = 0;

/**
 * \brief Mutex to have only one process to open and use device.
 */
static DEFINE_MUTEX(mutex_chardev);

/**
 * \brief File operations.
 */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = chardev_open,
    .release = chardev_release,
    .read = chardev_read,
    .write = chardev_write,
};

/**
 * \brief Structure to register the misc driver.
 */
static struct miscdevice chardev_misc = {
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
static int chardev_open(struct inode* inodep, struct file* filep)
{
    if(!mutex_trylock(&mutex_chardev))
    {
        printk(KERN_ALERT "%s: mutex already locked!\n", THIS_MODULE->name);
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
static int chardev_release(struct inode* inodep, struct file* filep)
{
    g_number_open--;
    printk(KERN_INFO "%s: release (%zu)\n", THIS_MODULE->name, g_number_open);
    mutex_unlock(&mutex_chardev);
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
static ssize_t chardev_read(struct file* filep, char* u_buffer, size_t len,
        loff_t* offset)
{
    int err = 0;
    ssize_t len_msg = 0;

    printk(KERN_INFO "%s: wants to read %zu bytes from offset %lld\n",
            THIS_MODULE->name, len, *offset);

    /* calculate buffer size left to copy */
    len_msg = g_message_size - *offset;

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

    err = copy_to_user(u_buffer, g_message + *offset, len_msg);

    if(err == 0)
    {
        /* success */
        printk(KERN_DEBUG "%s: sent %zu characters to user\n", THIS_MODULE->name,
                len_msg);

        *offset += len_msg;
        return len_msg;
    }
    else
    {
        printk(KERN_DEBUG "%s: failed to send %zu characters to user\n",
                THIS_MODULE->name, len_msg);
        return -EFAULT;
    }
}

/**
 * \brief Write callback for character device.
 * \param filep file.
 * \param u_buffer buffer that contains data to write.
 * \param len length to write.
 * \param offset offset of the buffer.
 * \return number of characters written, or negative value if failure.
 */
static ssize_t chardev_write(struct file* filep, const char* u_buffer,
        size_t len, loff_t* offset)
{
    ssize_t len_msg = len + *offset;

    printk(KERN_INFO "%s: wants to write %zu bytes from %lld offset\n",
            THIS_MODULE->name, len, *offset);

    if(len_msg > sizeof(g_message))
    {
        return -EFBIG;
    }

    if(copy_from_user(g_message + *offset, u_buffer, len) != 0)
    {
        g_message_size = 0;
        return -EFAULT;
    }

    if(*offset == 0)
    {
        g_message_size = 0;
    }
    
    g_message_size += len;
    *offset += len;

    printk(KERN_INFO "%s: received %zu characters from user\n", THIS_MODULE->name,
        len);
    return len;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init chardev_init(void)
{
    int ret = 0;

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    /* register device */
    ret = misc_register(&chardev_misc);

    if(ret == 0)
    { 
        printk(KERN_INFO "%s: device created correctly\n", THIS_MODULE->name);
    }

    return ret;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit chardev_exit(void)
{
    mutex_destroy(&mutex_chardev);
    misc_deregister(&chardev_misc);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("character device module");
MODULE_VERSION("0.1");

