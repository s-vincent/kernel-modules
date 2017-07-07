/*
 * ramdisk - ramdisk kernel module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file ramdisk.c
 * \brief Ram disk kernel module for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/spinlock.h>

/* forward declarations */
static int ramdisk_open(struct block_device* blkdev, fmode_t mode);
static void ramdisk_release(struct gendisk* disk, fmode_t mode);
static int ramdisk_getgeo(struct block_device* blkdev, struct hd_geometry* geo);

/**
 * \brief Sector size.
 */
static const size_t RAMDISK_SECTOR_SIZE = 512;

/**
 * \brief Number of minor numbers the device supports.
 */
static const size_t RAMDISK_MINORS = 5;

/**
 * \brief Driver major number value (configuration parameter).
 */
static int major = 0;

/**
 * \brief Number of sectors (configuration parameter).
*/
static size_t sectors = 131070;

/**
 * \brief Removable (configuration parameter).
 */
static bool removable = 0;

/**
 * \brief Spinlock for the block queue.
 */
static spinlock_t spl_queue;

/**
 * \brief Memory that will serve for ramdisk.
 */
static char* g_mem = NULL;

/**
 * \brief The disk.
 */
static struct gendisk* g_disk = NULL;

/**
 * \brief Operations on the block device. 
 */
static struct block_device_operations fops = {
    .owner = THIS_MODULE,
    .open = ramdisk_open,
    .release = ramdisk_release,
    .getgeo = ramdisk_getgeo,
};

/**
 * \brief Open callback.
 * \param blkdev the block device.
 * \param mode mode.
 * \return 0 if success, negative value otherwise.
 */
static int ramdisk_open(struct block_device* blkdev, fmode_t mode)
{
    printk(KERN_INFO "%s: open", THIS_MODULE->name);
    return 0;
}

/**
 * \brief Release callback.
 * \param disk the disk device to release.
 * \param mode mode.
 * \return 0 if success, negative value otherwise.
 */
static void ramdisk_release(struct gendisk* disk, fmode_t mode)
{
    printk(KERN_INFO "%s: release", THIS_MODULE->name);
}

/**
 * \brief Return geometry of the disk (simulated for the ramdisk).
 * \param blkdev the block device.
 * \param geo geometry structure.
 * \return 0 if success, negative value otherwise.
 */
static int ramdisk_getgeo(struct block_device* blkdev, struct hd_geometry* geo)
{
    if(!geo)
    {
        return -EINVAL;
    }

    /* simulated something real */
    geo->start = 0;
    geo->heads = 8;
    geo->sectors = 16;
    geo->cylinders = sectors / geo->sectors / geo->heads;
    geo->start = 0;

    return 0;
}

/**
 * \brief Callback function when queue received disk requests.
 * \param queue the block request queue.
 */
static void ramdisk_queue_request(struct request_queue* queue)
{
    struct request* req = NULL;

    req = blk_fetch_request(queue);

    while(req)
    {
        int ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
        if(blk_rq_is_passthrough(req))
#else
        if(req->cmd_type != REQ_TYPE_FS)
#endif
        {
            ret = -EIO;
        }
        else
        {
            /* blk_rq_pos and blk_rq_cur_sectors is number of sectors */
            unsigned long offset = blk_rq_pos(req) * RAMDISK_SECTOR_SIZE;
            unsigned long len = blk_rq_cur_sectors(req) * RAMDISK_SECTOR_SIZE;
            int direction = rq_data_dir(req);

            if(direction == 1) 
            {
                /* write to block device */
                memcpy(g_mem + offset, bio_data(req->bio), len);
                printk(KERN_DEBUG "%s: queue_request write", THIS_MODULE->name);
            }
            else
            {
                /* read from block device */
                memcpy(bio_data(req->bio), g_mem + offset, len);
                printk(KERN_DEBUG "%s: queue_request read", THIS_MODULE->name);
            }
        }

        /* check if the request is terminated and go to the next */
        if(__blk_end_request_cur(req, ret) == 0)
        {
            req = blk_fetch_request(queue);
        }
    }
}

/**
 * \brief Module initialization.
 *
 * Set up stuff when module is added.
 * \return 0 if success, negative value otherwise.
 */
static int __init ramdisk_init(void)
{
    int ret = 0;
    struct request_queue* queue = NULL;

    printk(KERN_INFO "%s: initialization\n", THIS_MODULE->name);

    if(sectors == 0)
    {
        return -EINVAL;
    }

    ret = register_blkdev(major, THIS_MODULE->name);

    if(ret < 0)
    {
        return ret;
    }

    /* if major = 0, kernel will alloate one for us */
    if(major == 0)
    {
        major = ret;
    }

    spin_lock_init(&spl_queue);

    queue = blk_init_queue(ramdisk_queue_request, &spl_queue);

    if(!queue)
    {
        unregister_blkdev(major, THIS_MODULE->name);
        return -ENOMEM;
    }

    g_disk = alloc_disk(RAMDISK_MINORS);
    if(!g_disk)
    {
        blk_cleanup_queue(queue);
        unregister_blkdev(major, THIS_MODULE->name);
        return -ENOMEM;
    }

    /* setup disk */
    g_disk->major = major;
    g_disk->first_minor = 0;
    g_disk->fops = &fops;
    g_disk->queue = queue;
    snprintf(g_disk->disk_name, sizeof(g_disk->disk_name) - 1, "ramdisk%d", 0);
    g_disk->disk_name[sizeof(g_disk->disk_name) - 1] = 0x00;
    set_capacity(g_disk, sectors);
    
    if(removable)
    {
        /* act as a removable device */
        g_disk->flags |= GENHD_FL_REMOVABLE;
    }

    g_mem = vmalloc(sectors * RAMDISK_SECTOR_SIZE);
    if(!g_mem)
    {
        del_gendisk(g_disk);
        blk_cleanup_queue(queue);
        unregister_blkdev(major, THIS_MODULE->name);
        return -ENOMEM;
    }

    add_disk(g_disk);

    return 0;
}

/**
 * \brief Module finalization.
 *
 * Clean up stuff when module is removed.
 */
static void __exit ramdisk_exit(void)
{
    del_gendisk(g_disk);
    blk_cleanup_queue(g_disk->queue);
    g_disk->queue = NULL;
    unregister_blkdev(major, THIS_MODULE->name);
    vfree(g_mem);

    printk(KERN_INFO "%s: exit\n", THIS_MODULE->name);
}

/* entry/exit points of the module */
module_init(ramdisk_init);
module_exit(ramdisk_exit);

/* parameters */
module_param(major, int, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(major, "Device major value");
module_param(sectors, ulong, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(sectors, "Number of sectors");
module_param(removable, bool, (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));
MODULE_PARM_DESC(removable, "Act as removable ramdisk");

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sebastien Vincent");
MODULE_DESCRIPTION("Ramdisk module");
MODULE_VERSION("0.1");

