/*
 * helloworld - simple hello world FreeBSD kernel module.
 * Copyright (c) 2016, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file helloworld.c
 * \brief Hello world kernel module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2016
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/**
 * \brief Name of the module.
 */
static char* name = "helloworld";

/**
 * \brief Cookie value.
 */
static int cookie = 0;

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init helloworld_init(void)
{
    printk(KERN_INFO "%s.%d: initialization\n", name, cookie);
    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit helloworld_exit(void)
{
    printk(KERN_INFO "%s.%d: exit\n", name, cookie);
}

/* entry/exit points of the module */
module_init(helloworld_init);
module_exit(helloworld_exit);

/* parameters */
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "Name of the module");
module_param(cookie, int, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(cookie, "Cookie value");

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("HelloWorld module");
MODULE_VERSION("0.1");

