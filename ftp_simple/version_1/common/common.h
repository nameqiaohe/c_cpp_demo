/*####################################################
# File Name: common.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 17:36:28
# Last Modified: 2017-04-28 10:10:59
####################################################*/
#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>

#define DEBUG 1
#define MAXSIZE 512
#define CLIENT_PORT 30001

struct command{
	char arg[255];
	char code[5];
};

int socket_create(int port);
int socket_accept(int sock_listen);

int socket_connect(int port, char *host);

int recv_data(int sock_fd, char *buf, int buf_size);

int send_response(int sock_fd, int rc);

void trim_str(char *str, int n);

void read_input(char *buf, int size);

#endif
