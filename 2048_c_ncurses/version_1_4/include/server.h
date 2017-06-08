/*####################################################
# File Name: server.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 15:24:32
# Last Modified: 2016-12-11 20:08:53
####################################################*/
#ifndef SERVER_H
#define SERVER_H

#include "header.h"

#define STR_MAX 30

typedef struct User{
	int id;
	char winTime[STR_MAX];
	int connectFd;
	int port;
}User;

int clientIndex;//给每个连接进来的客户端一个编号，用于标记
int currentClients;//当前连接进来的客户端数目

int ch;	//server端 接收终端输入

int listenfd, connfd;
int nready;
char buf[BUF_SIZE];
int bytes;
User clientSockets[FD_SETSIZE];//保存已连接的用户信息
//int clientSockFds[FD_SETSIZE];
fd_set allset, rset;
int maxfd;
struct sockaddr_in clientAddr;
socklen_t socklen;

void initVaule();
int createListenFd();
void printServerInfo();
void printClientInfo();
void handle(User *clientSockets, int maxfd, fd_set *rset, fd_set *allset);

void closeAllFds();
int isAllClientsClosed();

#endif//SERVER_H
