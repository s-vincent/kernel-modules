/*
 * kthread - kernel thread worker module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file kthread.c
 * \brief Thread kernel module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/completion.h>

/**
 * \struct kthread_thread
 * \brief Argument structure for thread.
 */
struct kthread_thread
{
    /**
     * \brief Variable condition for thread started.
     */
    struct completion started;

    /**
     * \brief Variable condition for thread stopped.
     */
    struct completion stopped;

    /**
     * \brief Variable to tell the thread to stop.
     */
    bool stop;

    /**
     * \brief Value for the thread.
     */
    int value;
};

/**
 * \brief Thread argument.
 */
static struct kthread_thread g_thread;

/**
 * \brief Function for the thread.
 * \param arg argument.
 * \return 0 if success, -1 otherwise.
 */
static int kthread_runner(void* arg)
{
    struct kthread_thread* thread = (struct kthread_thread*)arg;

    complete(&thread->started);

    while(!thread->stop)
    {
        printk(KERN_INFO "%s: %s running (%d)\n", THIS_MODULE->name,
                __FUNCTION__, thread->value);
        mdelay(500);
    }

    complete(&thread->stopped);
    return 0;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init kthread_init(void)
{
    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    g_thread.stop = 0;
    g_thread.value = 42;
    init_completion(&g_thread.started);
    init_completion(&g_thread.stopped);

    if(IS_ERR(kthread_run(kthread_runner, &g_thread, THIS_MODULE->name)))
    {
        return -ENOMEM;
    }

    printk(KERN_INFO "%s: wait for thread starts\n", THIS_MODULE->name);
    /* wait the thread to be started */
    wait_for_completion(&g_thread.started);
    printk(KERN_INFO "%s: runner thread started\n", THIS_MODULE->name);

    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit kthread_exit(void)
{
    /* ask ending of thread */
    g_thread.stop = 1;
    g_thread.value = 0;
    wait_for_completion(&g_thread.stopped);

    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(kthread_init);
module_exit(kthread_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("kthread module");
MODULE_VERSION("0.1");

