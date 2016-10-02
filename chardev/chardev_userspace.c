/*
 * chardev_userspace - userspace program to interact with chardev module.
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
    int fd = open("/dev/chardev", O_RDWR);
    char buf[1024];
    const char* input = argc > 1 ? argv[1] : "Test echo!";
    size_t input_len = argc > 1 ? strlen(argv[1]) : sizeof("Test echo!");
    ssize_t nb = 0;

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

