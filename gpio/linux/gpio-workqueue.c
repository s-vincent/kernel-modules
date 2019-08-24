/*
 * gpio-workqueue - GPIO workqueue kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file irq-workqueue.c
 * \brief IRQ workqueue module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>

/* forward declarations */
static void gpio_workqueue_bh(struct work_struct* arg);
static DECLARE_WORK(gpio_workqueue_wq, gpio_workqueue_bh);

/**
 * \brief GPIO to monitor (configuration parameter).
 */
static int gpio_number = -1;

/**
 * \brief GPIO input direction to monitor (configuration parameter).
 */
static bool gpio_input = 1;

/**
 * \brief GPIO value in case of output direction (configuration parameter).
 */
static bool gpio_value = 0;

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
 * \param arg work structure.
 */
static void gpio_workqueue_bh(struct work_struct* arg)
{
    do
    {
        printk(KERN_INFO "%s: GPIO bottom-half callback!\n", THIS_MODULE->name);
    }while(atomic_dec_return(&g_irq_cnt));
}

/**
 * \brief IRQ workqueue callback.
 * \param irq iIRQ number.
 * \param ident parameter.
 * \return IRQ_HANDLED or IRQ_NONE.
 */
static irqreturn_t gpio_workqueue_handler(int irq, void* ident)
{
    printk(KERN_INFO "%s: GPIO callback!\n", (char*)ident);

    /* increment our counter so that we know how many times IRQ happened in 
     * the bottom-half
     */
    atomic_inc(&g_irq_cnt);

    /* schedule bottom-half */
    schedule_work(&gpio_workqueue_wq);
    return IRQ_HANDLED;
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init gpio_workqueue_init(void)
{
    int flags = 0;
    int ret = 0;

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    if(gpio_number == -1)
    {
      printk(KERN_ERR "%s: bad GPIO number %d\n", THIS_MODULE->name,
              gpio_number);
      return -EINVAL;
    }

    ret = gpio_request(gpio_number, THIS_MODULE->name);
    if(ret != 0)
    {
        printk(KERN_ERR "%s: failed to find GPIO %d\n", THIS_MODULE->name,
                gpio_number);
        return ret;
    }

    if(gpio_input)
    {
        ret = gpio_direction_input(gpio_number);
    }
    else
    {
        ret = gpio_direction_output(gpio_number, 0);
        gpio_set_value(gpio_number, gpio_value);
    }

    if(ret != 0)
    {
        gpio_free(gpio_number);
        return ret;
    }

    if(gpio_input)
    {
        flags |= irq_share ? IRQF_SHARED : 0;
        flags |= irq_rising ? IRQF_TRIGGER_RISING : 0;
        flags |= irq_falling ? IRQF_TRIGGER_FALLING : 0;
        flags |= irq_high ? IRQF_TRIGGER_HIGH : 0;
        flags |= irq_low ? IRQF_TRIGGER_LOW : 0;

        printk(KERN_INFO "%s: try to register GPIO %d flags 0x%x\n",
                THIS_MODULE->name, gpio_number, flags);

        ret = request_irq(gpio_to_irq(gpio_number), gpio_workqueue_handler,
                flags, "GPIO workqueue", THIS_MODULE->name);

        if(ret != 0)
        {
            printk(KERN_ERR "%s: failed to register workqueue GPIO\n",
                    THIS_MODULE->name);
            gpio_free(gpio_number);
            return ret;
        }
    }
    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit gpio_workqueue_exit(void)
{
    flush_scheduled_work();
    free_irq(gpio_to_irq(gpio_number), THIS_MODULE->name);
    gpio_free(gpio_number);
    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(gpio_workqueue_init);
module_exit(gpio_workqueue_exit);

/* configuration parameters */
module_param(gpio_number, int, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(gpio_number, "GPIO number");
module_param(gpio_input, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(gpio_input, "Use GPIO input otherwise output");
module_param(gpio_value, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(gpio_value, "GPIO value in case of output");
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
MODULE_DESCRIPTION("IRQ workqueue");
MODULE_VERSION("0.1");

