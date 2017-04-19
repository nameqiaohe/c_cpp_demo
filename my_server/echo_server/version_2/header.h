/*####################################################
# File Name: header.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:04:14
# Last Modified: 2017-04-19 18:53:35
####################################################*/
#ifndef HEADER_H
#define HEADER_H

#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions. for ssize_t data type  */

#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*  Global constants  */
#define ECHO_PORT          (8888)
#define MAX_LINE           (1000)
#define LISTENQ (1024)   /*  Backlog for listen()   */

/*  Function declarations  */
ssize_t read_line(int fd, void *ptr, size_t maxlen);
ssize_t write_line(int fd, const void *ptr, size_t maxlen);

int open_listen_fd(int port);
int open_client_fd(char *hostname, int port);

#endif
