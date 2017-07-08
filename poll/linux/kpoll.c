/*
 * kpoll - poll device kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file kpoll.c
 * \brief Poll module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include <asm/uaccess.h>

/**
 * \def MSG_ARRAY_SIZE.
 * \brief Size of the messages array.
 */
#define MSG_ARRAY_SIZE 10

/* forward declarations */
static int kpoll_open(struct inode* inodep, struct file* filep);
static int kpoll_release(struct inode* inodep, struct file* filep);
static ssize_t kpoll_write(struct file* filep, const char* buffer,
        size_t len, loff_t* offset);
static ssize_t kpoll_read(struct file* filep, char* buffer, size_t len,
        loff_t* offset);
static unsigned int kpoll_poll(struct file* filep, poll_table* wait);

/**
 * \brief The wait queue.
 */
DECLARE_WAIT_QUEUE_HEAD(wq);

/**
 * \brief Use non-blocking read() if file requests it (configuration parameter).
 */
static bool nonblock = 0;

/**
 * \brief Message in kernel side for the device.
 */
static char g_messages[MSG_ARRAY_SIZE][1024]; 

/**
 * \brief Number of message in g_message.
 */
static size_t g_messages_count = 0;

/**
 * \brief Spinlock to control read/write in the array.
 */
static spinlock_t spinlock_wq;

/**
 * \brief File operations.
 */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = kpoll_open,
    .release = kpoll_release,
    .read = kpoll_read,
    .write = kpoll_write,
    .poll = kpoll_poll,
};

/**
 * \brief Structure to register the misc driver.
 */
static struct miscdevice kpoll_misc = {
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
static int kpoll_open(struct inode* inodep, struct file* filep)
{
    printk(KERN_INFO "%s: open\n", THIS_MODULE->name);
    return 0;
}

/**
 * \brief Release callback for character device.
 * \param inodep inode.
 * \param filep file.
 * \return 0 if success, negative value otherwise.
 */
static int kpoll_release(struct inode* inodep, struct file* filep)
{
    printk(KERN_INFO "%s: release\n", THIS_MODULE->name);
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
static ssize_t kpoll_read(struct file* filep, char* u_buffer, size_t len,
        loff_t* offset)
{
    int err = 0;
    ssize_t len_msg = 0;
    unsigned long mask = 0;

    printk(KERN_INFO "%s: wants to read %zu bytes from offset %lld\n",
            THIS_MODULE->name, len, *offset);

    spin_lock_irqsave(&spinlock_wq, mask);

    while(g_messages_count == 0)
    {
        /* array empty, wait for item */
        spin_unlock_irqrestore(&spinlock_wq, mask);

        /* returns now if nonblock is requested */
        if(nonblock && filep->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }

        if(wait_event_interruptible(wq, (g_messages_count > 0)) != 0)
        {
            return -ERESTARTSYS;
        }
        
        spin_lock_irqsave(&spinlock_wq, mask);
    }

    /* calculate buffer size left to copy */
    len_msg = strlen(g_messages[0]);

    if(len_msg == 0)
    {
        /* EOF */
        spin_unlock_irqrestore(&spinlock_wq, mask);
        return 0;
    }
    else if(len_msg > len)
    {
        len_msg = len;
    }
    else if(len_msg < 0)
    {
        spin_unlock_irqrestore(&spinlock_wq, mask);
        return -EINVAL;
    }

    err = copy_to_user(u_buffer, g_messages[0], len_msg);

    /* shift other messages */
    if(g_messages_count > 1)
    {
        memmove(g_messages, g_messages + 1, (g_messages_count - 1) * 1024);
    }
    g_messages_count--;

    spin_unlock_irqrestore(&spinlock_wq, mask);

    /* array has at least one space left */
    wake_up_interruptible(&wq);

    if(err == 0)
    {
        /* success */
        printk(KERN_DEBUG "%s: sent %zu characters to user\n", THIS_MODULE->name,
                len_msg);

        /* *offset += len_msg; */
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
static ssize_t kpoll_write(struct file* filep, const char* u_buffer,
        size_t len, loff_t* offset)
{
    ssize_t len_msg = len + *offset;
    unsigned long mask = 0;

    printk(KERN_INFO "%s: wants to write %zu bytes from %lld offset\n",
            THIS_MODULE->name, len, *offset);

    spin_lock_irqsave(&spinlock_wq, mask);

    while(g_messages_count >= MSG_ARRAY_SIZE)
    {
        /* array full, wait for empty space */
        spin_unlock_irqrestore(&spinlock_wq, mask);

        if(nonblock && filep->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }

        if(wait_event_interruptible(wq,
              (g_messages_count < MSG_ARRAY_SIZE)) != 0)
        {
            return -ERESTARTSYS;
        }

        spin_lock_irqsave(&spinlock_wq, mask);
    }

    if(len_msg > (1024 - 1))
    {
        spin_unlock_irqrestore(&spinlock_wq, mask);
        return -E2BIG;
    }

    if(copy_from_user(g_messages[g_messages_count], u_buffer, len) != 0)
    {
        spin_unlock_irqrestore(&spinlock_wq, mask);
        return -EFAULT;
    }

    g_messages[g_messages_count][len] = 0x00;
    g_messages_count++;
    
    spin_unlock_irqrestore(&spinlock_wq, mask);
 
    /* array has at least one item left */
    wake_up_interruptible(&wq);

    *offset += len;
    printk(KERN_INFO "%s: received %zu characters from user\n",
            THIS_MODULE->name, len);
    return len;
}

/**
 * \brief See if data is ready.
 * \param filep file descriptor.
 * \param wait table to hold waitqueues.
 * \return mask of POLL* value or 0 if nothing available yet.
 */
static unsigned int kpoll_poll(struct file* filep, poll_table* wait)
{
  unsigned int mask = 0;

  /* adds waitqueue to the table */
  poll_wait(filep, &wq, wait);

  if(g_messages_count < MSG_ARRAY_SIZE)
  {
    mask |= POLLOUT | POLLWRNORM;
  }
  
  if(g_messages_count > 0)
  {
    mask |= POLLIN | POLLRDNORM;
  }

  return mask;
}


/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init kpoll_init(void)
{
    int ret = 0;

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    /* register device */
    ret = misc_register(&kpoll_misc);

    if(ret == 0)
    { 
        printk(KERN_INFO "%s: device created correctly\n", THIS_MODULE->name);
    }

    spin_lock_init(&spinlock_wq); 

    return ret;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit kpoll_exit(void)
{
    misc_deregister(&kpoll_misc);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(kpoll_init);
module_exit(kpoll_exit);

module_param(nonblock, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(nonblock, "Authorize non-blocking read() if file requests it");

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("Poll device module");
MODULE_VERSION("0.1");

