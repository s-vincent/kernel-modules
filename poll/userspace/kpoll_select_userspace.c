/*
 * kpoll_select_userspace - userspace program to interact with kpoll module.
 * Copyright (c) 2017, Sebastien Vincent
 *
 * Distributed under the terms of the BSD 3-clause License.
 * See the LICENSE file for details.
 */

/**
 * \file kpoll_userspace.c
 * \brief Userspace program to test the kpoll kernel module with select().
 * \author Sebastien Vincent
 * \date 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

/**
 * \brief Entry point of the program.
 * \param argc number of arguments.
 * \param argv array of arguments.
 * \return EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv)
{
  int fd = -1;
  int ret = 0;
  int nonblock = argc > 1 && !strcmp(argv[1], "1");
  fd_set fdsr;
  struct timeval tv = { 10, 0 };

  FD_ZERO(&fdsr);

  fd = open("/dev/kpoll", O_RDWR | (nonblock ? O_NONBLOCK : 0));
  if(fd == -1)
  {
    perror("open");
    exit(EXIT_FAILURE);
  }

  FD_SET(fd, &fdsr);

  ret = select(fd + 1, &fdsr, NULL, NULL, &tv);

  fprintf(stdout, "select returned %d\n", ret);
  if(ret == -1)
  {
    perror("select");
    close(fd);
    exit(EXIT_FAILURE);
  }
  else if(ret > 0)
  {
    if(FD_ISSET(fd, &fdsr))
    {
      char buf[1024];
      ssize_t nb = 0;

      fprintf(stdout, "Read on descriptor %d\n", fd);

      nb = read(fd, buf, sizeof(buf));

      if(nb <= 0)
      {
        perror("read");
        close(fd);
        exit(EXIT_FAILURE);
      }

      buf[nb] = 0x00;
      printf("Buffer: %s\n", buf);
    }
  }
  else /* ret == 0 */
  {
    fprintf(stdout, "select timeout\n");
  }

  close(fd);
  return EXIT_SUCCESS;
}

