/*
 * chardev - basic character device Linux kernel module.
 * Copyright (C) 2016 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * \file chardev.c
 * \brief Basic character device module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2016
 */

/*
 This code is based and adapted from
 http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>

#include <asm/uaccess.h>

/**
 * \brief Device name.
 */
static const char* const DEVICE_NAME = "chardev";

/**
 * \brief Class name.
 */
static const char* const CLASS_NAME = "test";

/**
 * \brief Name of the module.
 */
static char* name = "chardev";

/**
 * \brief Cookie value.
 */
static int cookie = 0;

/**
 * \brief File operations.
 */
static struct file_operations fops = {0};

/**
 * \brief Major number.
 */
static int major_number = 0;

/**
 * \brief The device class.
 */
static struct class* chardev_class = NULL;

/**
 * \brief The device.
 */
static struct device* chardev_device = NULL;

/**
 * \brief Message in kernel side for the device.
 */
static char message[1024] = {0};

/**
 * \brief Size of message stored in kernel side.
 */
static size_t message_size = 0;

/**
 * \brief Number of times device is opened.
 */
static size_t number_open = 0;

/**
 * \brief Mutex to have only one process to open and use device.
 */
static DEFINE_MUTEX(chardev_mutex);

/**
 * \brief Open callback for character device.
 * \param inodep inode.
 * \param filep file.
 * \return 0 if success, negative value otherwise.
 */
static int chardev_open(struct inode* inodep, struct file* filep)
{
    if(!mutex_trylock(&chardev_mutex))
    {
        printk(KERN_ALERT "%s.%d mutex already locked!\n", name, cookie);
        return -EBUSY;
    }
    number_open++;
    printk(KERN_INFO "%s.%d: open (%zu)\n", name, cookie, number_open);
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
    number_open--;
    printk(KERN_INFO "%s.%d: release (%zu)\n", name, cookie, number_open);
    mutex_unlock(&chardev_mutex);
    return 0;
}

/**
 * \brief Read callback for character device.
 * \param filep file.
 * \param buffer buffer to fill.
 * \param len length to read.
 * \param offset offset of the buffer.
 * \return number of characters read, or negative value if failure.
 */
static ssize_t chardev_read(struct file* filep, char* buffer, size_t len,
        loff_t* offset)
{
    int err = 0;
    printk(KERN_INFO "%s.%d: wants to read at most %zu bytes\n", name, cookie,
            len);

    err = copy_to_user(buffer, message, message_size);

    if(err == 0)
    {
        /* success */
        ssize_t snd = message_size;

        printk(KERN_INFO "%s.%d sent %zu characters to user\n", name, cookie,
                message_size);
        message_size = 0;
        return snd;
    }
    else
    {
        printk(KERN_INFO "%s.%d failed to send %zu characters to user\n",
                name, cookie, message_size);
        return -EFAULT;
    }
}

/**
 * \brief Write callback for character device.
 * \param filep file.
 * \param buffer buffer that contains data to write.
 * \param len length to write.
 * \param offset offset of the buffer.
 * \return number of characters written, or negative value if failure.
 */
static ssize_t chardev_write(struct file* filep, const char* buffer,
        size_t len, loff_t* offset)
{
    printk(KERN_INFO "%s.%d: wants to write %zu bytes\n", name, cookie, len);

    if(len > sizeof(message) - 1)
    {
        return -EFBIG;
    }

    if(copy_from_user(message, buffer, len) != 0)
    {
        message_size = 0;
        return -EFAULT;
    }
    message_size = len;

    printk(KERN_INFO "%s.%d received %zu characters from user\n", name, cookie,
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
    printk(KERN_INFO "%s.%d: initialization\n", name, cookie);

    /* initialize fops */
    fops.open = chardev_open;
    fops.release = chardev_release;
    fops.read = chardev_read;
    fops.write = chardev_write;

    /* register major number (dynamically) */
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if(major_number < 0)
    {
        printk(KERN_ALERT "%s.%d failed to register a major number\n", name,
            cookie);
        return major_number;
    }
    printk(KERN_INFO "%s.%d registered correctly a major number (%d)\n", name,
            cookie, major_number);

    /* register class */
    chardev_class = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(chardev_class))
    {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "%s.%d failed to register device class\n", name,
            cookie);
        return PTR_ERR(chardev_class);
    }
    printk(KERN_INFO "%s.%d device class registered correctly\n", name, cookie);

    /* register device */
    chardev_device = device_create(chardev_class, NULL,
            MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if(IS_ERR(chardev_device))
    {
        class_destroy(chardev_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "%s.%d failed to create the device\n", name, cookie);
        return PTR_ERR(chardev_device);
    }
    printk(KERN_INFO "%s.%d device created correctly\n", name, cookie);

    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit chardev_exit(void)
{
    device_destroy(chardev_class, MKDEV(major_number, 0));
    class_destroy(chardev_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    mutex_destroy(&chardev_mutex);
    printk(KERN_INFO "%s.%d: exit\n", name, cookie);
}

/* entry/exit points of the module */
module_init(chardev_init);
module_exit(chardev_exit);

/* parameters */
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "Name of the module");
module_param(cookie, int, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(cookie, "Cookie value");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("character device module");
MODULE_VERSION("0.1");

