/*####################################################
# File Name: rio.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 22:44:17
# Last Modified: 2017-05-17 00:15:57
####################################################*/
#ifndef RIO_H
#define RIO_H

#include <sys/types.h>

#define RIO_BUF_SIZE 8192

typedef struct{
	int rio_fd;//与缓冲区绑定的文件描述符
	ssize_t rio_cnt;//缓冲区中未读取的字节数
	char *rio_buf_ptr;//缓冲区中下一个未读取字节的位置
	char rio_buf[RIO_BUF_SIZE];//内部缓冲区
}rio_t;

ssize_t rio_read_n(int fd, void *usr_buf, size_t n);

ssize_t rio_write_n(int fd, void *usr_buf, size_t n);

void rio_read_init_b(rio_t *rp, int fd);

ssize_t rio_read(rio_t *rp, char *usr_buf, size_t n);

ssize_t rio_read_n_b(rio_t *rp, void *usr_buf, size_t n);

ssize_t rio_read_line_b(rio_t *rp, void *usr_buf, size_t max_len);

#endif
