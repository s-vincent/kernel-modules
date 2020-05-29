/*
 * proc - /proc sample kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file proc.c
 * \brief /proc kernel module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/version.h>

#include <asm/uaccess.h>
#include <linux/uaccess.h>

/* forward declarations */
static ssize_t proc_read(struct file* filep, char* __user u_buffer,
        size_t len, loff_t* offset);
static ssize_t proc_write(struct file* filep, const char* __user u_buffer,
        size_t len, loff_t* offset);

/**
 * \brief Entry name in /proc.
 */
static const char* const PROC_ENTRY_NAME = "entry";

/**
 * \brief Directory tree in /proc.
 */
static const char* const PROC_DIR_NAME = "vs";

/**
 * \brief The entry created in /proc.
 */
static struct proc_dir_entry* g_dir = NULL;

/**
 * \brief Value to be set in write.
 */
static uint32_t g_value = 0;

/**
 * \brief File operations.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,5,0)

static struct proc_ops fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

#else

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = proc_read,
    .write = proc_write,
};

#endif

/**
 * \brief Read callback.
 * \param filep file.
 * \param u_buffer buffer to fill.
 * \param len length to read.
 * \param offset offset of the buffer.
 * \return number of characters read, or negative value if failure.
 */
static ssize_t proc_read(struct file* filep, char* __user u_buffer,
        size_t len, loff_t* offset)
{
    int err = 0;
    ssize_t len_msg = 0;
    char info[256];

    printk(KERN_INFO "%s: wants to read %zu bytes from offset %lld\n",
            THIS_MODULE->name, len, *offset);

    snprintf(info, sizeof(info), "Process %s PID %u value %u\n", current->comm,
            current->pid, g_value);
    info[sizeof(info) - 1] = 0x00;

    /* calculate buffer size left to copy */
    len_msg = strlen(info) - *offset;

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

    err = copy_to_user(u_buffer, info + *offset, len_msg);

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
 * \brief Write callback. 
 * \param filep file.
 * \param u_buffer buffer that contains data to write.
 * \param len length to write.
 * \param offset offset of the buffer.
 * \return number of characters written, or negative value if failure.
 */
static ssize_t proc_write(struct file* filep, const char* __user u_buffer,
        size_t len, loff_t* offset)
{
    /* should be sufficient to store uint32_t */
    uint32_t value = 0;

    if(kstrtou32_from_user(u_buffer, len, 10, &value) < 0) 
    {
        return -EINVAL;
    }

    g_value = value;

    return len;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init proc_init(void)
{
    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    /* create directory */
    g_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    
    if(!g_dir)
    {
        printk(KERN_ERR "%s: failed to create /proc/%s directory\n",
                THIS_MODULE->name, PROC_DIR_NAME);
        return -EEXIST;
    }
    
    /* then the entry */
    if(!proc_create(PROC_ENTRY_NAME, 0644, g_dir, &fops))
    {
        proc_remove(g_dir);
        printk(KERN_ERR "%s: failed to create /proc/%s/%s entry\n",
                THIS_MODULE->name, PROC_DIR_NAME, PROC_ENTRY_NAME);
        return -EEXIST;
    }

    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit proc_exit(void)
{
    /* cleanup */
    if(g_dir)
    {
        remove_proc_entry(PROC_ENTRY_NAME, g_dir);
        proc_remove(g_dir);
    }

    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("/proc module");
MODULE_VERSION("0.1");

