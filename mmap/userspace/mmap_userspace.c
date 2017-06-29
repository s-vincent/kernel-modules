/*
 * mmap_userspace - userspace program to interact with chardev module with mmap.
 * Copyright (c) 2016, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file mmap_userspace.c
 * \brief Userspace program to test kmmap kernel module.
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
#include <sys/mman.h>

/**
 * \brief Entry point of the program.
 * \param argc number of arguments.
 * \param argv array of arguments.
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
    int fd = -1;
    const char* input = argc > 1 ? argv[1] : "Test echo!";
    size_t input_len = argc > 1 ? strlen(argv[1]) : sizeof("Test echo!");
    char* mem = NULL;

    fd = open("/dev/kmmap", O_RDWR);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    mem = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 

    if(mem == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    /* writing */
    if(argc > 1)
    {
        strncpy(mem, input, input_len);
    }
    else
    {
        printf("Buffer: %s\n", mem);
    }

    close(fd);
    munmap(mem, 1024);

    return EXIT_SUCCESS;
}

