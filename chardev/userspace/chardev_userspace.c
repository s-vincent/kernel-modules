/*
 * chardev_userspace - userspace program to interact with chardev module.
 * Copyright (c) 2016, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file chardev_userspace.c
 * \brief Userspace program to test the chardev kernel module.
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
    const char* input = argc > 1 ? argv[1] : "Test echo!";
    size_t input_len = argc > 1 ? strlen(argv[1]) : sizeof("Test echo!");
    ssize_t nb = 0;

    fd = open("/dev/chardev", O_RDWR);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("write(%s) size=%zu\n", input, input_len);
    if(write(fd, input, input_len) <= 0)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);

    fd = open("/dev/chardev", O_RDWR);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    nb = read(fd, buf, sizeof(buf) - 1);
    printf("read(): %zd\n", nb); 
    if(nb == -1)
    {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        buf[nb] = 0x00;
    }

    printf("Buffer: %s\n", buf);
    /* sleep(120); */
    close(fd);
    return EXIT_SUCCESS;
}

