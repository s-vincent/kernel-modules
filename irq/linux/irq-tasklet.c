/*
 * irq-tasklet - IRQ tasklet kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file irq-tasklet.c
 * \brief IRQ tasklet module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/atomic.h>

/* forward declarations */
static void irq_tasklet_bh(unsigned long arg);
static DECLARE_TASKLET(irq_tasklet_tasklet, irq_tasklet_bh,
        (unsigned long)THIS_MODULE->name);

/**
 * \brief IRQ to monitor (configuration parameter).
 */
static int irq_number = -1;

/**
 * \brief Share the IRQ to monitor (configuration parameter).
 */
static bool irq_share = 0;

/**
 * \brief Monitor the IRQ for rising trigger (configuration parameter).
 */
static bool irq_rising = 0;

/**
 * \brief Monitor the IRQ for falling trigger (configuration parameter).
 */
static bool irq_falling = 0;

/**
 * \brief Monitor the IRQ for high level trigger (configuration parameter).
 */
static bool irq_high = 0;

/**
 * \brief Monitor the IRQ for low level trigger (configuration parameter).
 */
static bool irq_low = 0;

/**
 * \brief Counter for IRQ.
 */
static atomic_t g_irq_cnt = ATOMIC_INIT(0);

/**
 * \brief IRQ bottom-half callback.
 * \param arg argument.
 */
static void irq_tasklet_bh(unsigned long arg)
{
    do
    {
        printk(KERN_INFO "%s: IRQ bottom-half callback!\n", (char*)arg);
    }while(atomic_dec_return(&g_irq_cnt));
}

/**
 * \brief IRQ handler callback.
 * \param irq iIRQ number.
 * \param ident parameter.
 * \return IRQ_HANDLED or IRQ_NONE.
 */
static irqreturn_t irq_handler(int irq, void* ident)
{
    printk(KERN_INFO "%s: IRQ callback!\n", (char*)ident);

    /* increment our counter so that we know how many times IRQ happened in 
     * the bottom-half
     */
    atomic_inc(&g_irq_cnt);

    /* schedule bottom-half */
    tasklet_schedule(&irq_tasklet_tasklet);
    return IRQ_HANDLED;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init irq_tasklet_init(void)
{
    int flags = 0;

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    if(irq_number == -1)
    {
      printk(KERN_ERR "%s: Bad IRQ number %d\n", THIS_MODULE->name, irq_number);
      return -EINVAL;
    }
    
    flags |= irq_share ? IRQF_SHARED : 0;
    flags |= irq_rising ? IRQF_TRIGGER_RISING : 0;
    flags |= irq_falling ? IRQF_TRIGGER_FALLING : 0;
    flags |= irq_high ? IRQF_TRIGGER_HIGH : 0;
    flags |= irq_low ? IRQF_TRIGGER_LOW : 0;

    printk(KERN_INFO "%s: try to register IRQ %d flags 0x%x\n",
        THIS_MODULE->name, irq_number, flags);

    if(request_irq(irq_number, irq_handler, flags,
                "IRQ handler", THIS_MODULE->name) != 0)
    {
        printk(KERN_ERR "%s: Failed to register IRQ handler\n",
                THIS_MODULE->name);
        return -1;
    }

    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit irq_tasklet_exit(void)
{
    /* be sure no tasklet is running */
    tasklet_kill(&irq_tasklet_tasklet);

    free_irq(irq_number, THIS_MODULE->name);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(irq_tasklet_init);
module_exit(irq_tasklet_exit);

/* configuration parameters */
module_param(irq_number, int, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(irq_number, "IRQ number");
module_param(irq_share, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(irq_share, "share IRQ");
module_param(irq_rising, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(irq_rising, "Monitore IRQ for rising trigger");
module_param(irq_falling, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(irq_falling, "Monitore IRQ for falling trigger");
module_param(irq_high, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(irq_high, "Monitore IRQ for high level trigger");
module_param(irq_low, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(irq_low, "Monitore IRQ for low level trigger");

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("IRQ tasklet");
MODULE_VERSION("0.1");

