/*
 * helloworld - simple hello world Linux kernel module.
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
 * \file helloworld.c
 * \brief Hello world module.
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
 * \return 0 if success, other value for failure.
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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("HelloWorld module");
MODULE_VERSION("0.1");

