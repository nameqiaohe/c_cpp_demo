/*####################################################
# File Name: header.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:04:14
# Last Modified: 2017-04-20 18:46:41
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

/* 服务端、客户端的socket封装函数 */
int open_listen_fd(int port);
int open_client_fd(char *hostname, int port);

/* Persistent state for the robust I/O (Rio) package */
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

/* rio_read functions */
void rio_read_initb(rio_t *rp, int fd);
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);//不带缓冲区
size_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_read_lineb(rio_t *rp, void *usrbuf, size_t maxlen);

/* rio_write functions */
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

#endif
