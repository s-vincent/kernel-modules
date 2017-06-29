/*
 * waitqueue_userspace - userspace program to interact with waitqueue module.
 * Copyright (c) 2016, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file waitqueue_userspace.c
 * \brief Userspace program to test the waitqueue kernel module.
 * \author Sebastien Vincent
 * \date 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

/**
 * \brief Entry point of the program.
 * \param argc number of arguments.
 * \param argv array of arguments.
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
    int fd = -1;
    char buf[1024];
    int nonblock = argc > 1 && !strcmp(argv[1], "1");
    ssize_t nb = 0;

    fd = open("/dev/waitqueue", O_RDWR | (nonblock ? O_NONBLOCK : 0));
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    nb = read(fd, buf, sizeof(buf));
    
    if(nb <= 0)
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    buf[nb] = 0x00;

    printf("Buffer: %s\n", buf);
    return EXIT_SUCCESS;
}

