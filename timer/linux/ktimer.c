/*
 * ktimer - Timer kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file ktimer.c
 * \brief Timer kernel module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/version.h>

/**
 * \brief The timer.
 */
struct timer_list g_timer;

/**
 * \brief Timer callback.
 * \param arg argument.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void ktimer_function(unsigned long arg)
#else
static void ktimer_function(struct timer_list* arg)
#endif
{
    printk(KERN_INFO "%s: %s %lu\n", THIS_MODULE->name, __FUNCTION__, jiffies);
   
    /* rearm timer in 5 seconds */
    mod_timer(&g_timer, jiffies + (HZ * 5));
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init ktimer_init(void)
{
    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
    init_timer(&g_timer);
    g_timer.function = &ktimer_function;
#else
    timer_setup(&g_timer, ktimer_function, 0);
#endif
    /* HZ = number of jiffies per second, so timer triggers in 5 seconds */
    g_timer.expires = jiffies + (HZ * 5);

    /* arm timer */
    add_timer(&g_timer);
    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit ktimer_exit(void)
{
    del_timer(&g_timer);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(ktimer_init);
module_exit(ktimer_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("Timer module");
MODULE_VERSION("0.1");

