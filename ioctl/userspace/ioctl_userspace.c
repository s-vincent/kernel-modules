/*
 * ioctl_userspace - userspace program to interact with ioctl module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file ioctl_userspace.c
 * \brief Userspace program to test the ioctl kernel module.
 * \author Sebastien Vincent
 * \date 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "kioctl.h"

/**
 * \brief Entry point of the program.
 * \param argc number of arguments.
 * \param argv array of arguments.
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
    int fd = -1;
    uint32_t val = 0;
    int ret = -1;
    int set = (argc > 1); 

    if(set)
    {
        char* tmp = NULL;
        val = strtoul(argv[1], &tmp, 0);

        if(*tmp != 0x00)
        {
            fprintf(stderr, "Invalid argument: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "Set value to %u\n", val);
    }
    else
    {
        fprintf(stdout, "Get value\n");
    }
    
    fd = open("/dev/kioctl", O_RDWR);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }


    ret = ioctl(fd, (set ? KIOCSVAL : KIOCGVAL), &val);

    if(ret == -1)
    {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if(!set)
    {
        fprintf(stdout, "Value is %u\n", val);
    }
                
    close(fd);
    return EXIT_SUCCESS;
}

