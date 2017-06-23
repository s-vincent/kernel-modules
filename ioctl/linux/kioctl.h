/*
 * kioctl - basic character device kernel module with ioctl.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file kioctl.h
 * \brief Basic character device module with ioctl for GNU/Linux.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <asm/ioctl.h>

#ifndef KIOCTL_H
#define KIOCTL_H

#define KIOCTL_IOCTL_MAGIC 't'

#define KIOCTL_GET_VALUE 1
#define KIOCTL_SET_VALUE 2

#define KIOCGVAL _IOR(KIOCTL_IOCTL_MAGIC, KIOCTL_GET_VALUE, int)
#define KIOCSVAL _IOW(KIOCTL_IOCTL_MAGIC, KIOCTL_SET_VALUE, int)

#endif /* KIOCTL_H */

