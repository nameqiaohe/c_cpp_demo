/*####################################################
# File Name: ../include/client.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 16:49:46
# Last Modified: 2016-12-11 00:01:52
####################################################*/
#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>

#define max(a,b) ((a) > (b) ? (a) : (b))

int connfd;
fd_set rset;

char sendBuf[BUF_SIZE];
char recvBuf[BUF_SIZE];

void initClientValue();
void whileProcess(int connfd);
void handler(int sig);

void *thread_func();

#endif//CLIENT_H
