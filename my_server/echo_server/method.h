/*####################################################
# File Name: method.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:32:04
# Last Modified: 2017-04-09 15:36:03
####################################################*/
#ifndef METHOD_H
#define METHOD_H

#include <unistd.h>             /*  for ssize_t data type  */

#define LISTENQ (1024)   /*  Backlog for listen()   */

/*  Function declarations  */
ssize_t read_line(int fd, void *ptr, size_t maxlen);
ssize_t write_line(int fd, const void *ptr, size_t maxlen);

#endif
