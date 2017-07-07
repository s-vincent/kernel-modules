/*
 * timerhr - high resolution timer kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file timerhr.c
 * \brief High resolution timer kernel module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>

/**
 * \brief The timer.
 */
struct hrtimer g_timer;

/**
 * \brief Timer callback.
 * \param arg argument.
 */
static enum hrtimer_restart timerhr_function(struct hrtimer* timer)
{
    ktime_t period;
    
    printk(KERN_INFO "%s: %s %lu\n", THIS_MODULE->name, __FUNCTION__, jiffies);

    /* rearm timer */
    /* period of 1 second */
    period = ktime_set(0, 1000000000);
    hrtimer_forward_now(timer, period); 
    return HRTIMER_RESTART;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init timerhr_init(void)
{
    ktime_t period;
    
    /* period of 1 second */
    period = ktime_set(0, 1000000000);

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);
    hrtimer_init(&g_timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
    g_timer.function = timerhr_function;

    /* arm the timer */
    hrtimer_start(&g_timer, period, HRTIMER_MODE_REL);
    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit timerhr_exit(void)
{
    hrtimer_cancel(&g_timer);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(timerhr_init);
module_exit(timerhr_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("high resolution timer module");
MODULE_VERSION("0.1");

