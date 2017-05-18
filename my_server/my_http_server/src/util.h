/*####################################################
# File Name: util.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 19:45:17
# Last Modified: 2017-05-18 10:52:26
####################################################*/
#ifndef UTIL_H
#define UTIL_H

//最大监听数量
#define LISTENQ 1024

#define BUF_LEN 8192

//分割符
#define DELIM "="

//ST : simple httpserver
#define ST_CONF_OK 0
#define ST_CONF_ERROR 100

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct conf_t{
	void *root;
	int port;
	int thread_num;
}conf_t;

//创建监听socket描述符
int open_listen_fd(int port);

//设置描述符 为 非阻塞方式
int make_socket_nonblocking(int fd);

//从配置文件中获取相关信息
int read_conf(char *filename, conf_t *cf, char *buf, int len);

#endif
