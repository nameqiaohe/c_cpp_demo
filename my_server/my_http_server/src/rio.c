/*####################################################
# File Name: rio.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 23:23:51
# Last Modified: 2017-05-19 13:03:33
####################################################*/
#include "rio.h"
#include "dbg.h"
#include <unistd.h>

/*
 * robustly read n bytes, unbuffered
 * 保证在没有遇到 EOF的情况下，一定可以读取 n个字节
 * 返回值：
 *		> 0 表示 成功读取的字节数，若小于 n，说明中间遇到了 EOF
 *		== 0 表示 一开始读取就遇到 EOF
 *		-1	表示出错 (这里的 errno绝对不是 EINTR)
*/
ssize_t rio_read_n(int fd, void *usr_buf, size_t n){
	size_t n_left = n;
	ssize_t n_read;

	char *buf_p = (char *)usr_buf;

	while(n_left > 0){
		n_read = read(fd, buf_p, n_left);
		if(n_read < 0){// n_read < 0 表示读取出错
			if(errno == EINTR){// 若读取出错是由 中断引起的，则在下次 while循环继续读取
				n_read = 0;
			}else{//若读取出错不是由 中断引起的，返回错误
				log_error("rio_read_n() : ad() error, errno = %d\n", errno);
				return -1;
			}
		}else if(n_read == 0){// 若读取到的字节数为 0，表示对端关闭，本端退出循环
			break;
		}

		n_left -= n_read;
		buf_p += n_read;
	}

	return (n - n_left);// 返回读取到的字节数
}

/*
 * robustly write n bytes, unbuffered
 * 保证在没有遇到 EOF的情况下，一定可以写 n个字节
 * 返回值：
 *		n  表示 成功写入 n个字节
 *		-1 表示 写入失败 (这里也绝对不是 EINTR错误)
*/
ssize_t rio_write_n(int fd, void *usr_buf, size_t n){
	size_t n_left = n;
	ssize_t n_written;

	char *buf_p = (char *)usr_buf;

	while(n_left > 0){
		n_written = write(fd, buf_p, n_left);
		if(n_written <= 0){
			if(errno == EINTR){
				n_written = 0;
			}else{
				log_error("rio_write_n() : write() error, errno = %d\n", errno);
				return -1;
			}
		}
		n_left -= n_written;
		buf_p += n_written;
	}
	
	return n;//返回写入的字节数 //将 n个字节数据全部写入，才会跳出 while循环，或者写入出错，直接返回 -1
}

void rio_read_init_b(rio_t *rp, int fd){
	rp->rio_fd = fd;// 绑定文件
	rp->rio_cnt = 0;// 将 缓冲区中未读取的字节数设为 0
	rp->rio_buf_ptr = rp->rio_buf;// 将 下一个未读取的字节位置 指向 缓冲区首地址
}

// 带缓冲区
ssize_t rio_read(rio_t *rp, char *usr_buf, size_t n){
	size_t cnt;

	while(rp->rio_cnt <= 0){
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt < 0){
			if(errno == EAGAIN){// 已经读到 n 个字节的数据
				return -EAGAIN;// EAGAIN 表示：在非阻塞操作中，现在没有数据可读
			}

			if(errno != EINTR){// 若读取出错不是由 中断引起的，则返回错误
				log_error("rio_read() : read() error, errno = %d\n", errno);
				return -1;
			}
		}else if(rp->rio_cnt == 0){// EOF
			return 0;
		}else{
			rp->rio_buf_ptr = rp->rio_buf;// 读取到的内容在 rio_buf缓冲区中
		}
	}

	// 读取到多少个字节数据，就将多少个字节的数据拷贝到 usr_buf中
	cnt = n;
	if(rp->rio_cnt < (ssize_t)n){
		cnt = rp->rio_cnt;
	}
	memcpy(usr_buf, rp->rio_buf_ptr, cnt);
	rp->rio_buf_ptr += cnt;
	rp->rio_cnt -= cnt;

	return cnt;// 返回读取到的字节数
}

ssize_t rio_read_n_b(rio_t *rp, void *usr_buf, size_t n){
	size_t n_left = n;
	ssize_t n_read;

	char *buf_p = (char *)usr_buf;

	while(n_left > 0){
		n_read = rio_read(rp, buf_p, n_left);
		if(n_read < 0){
			if(errno == EINTR){
				n_read = 0;	
			}else{
				log_error("rio_read_n_b() : rio_read() error, errno = %d\n", errno);
				return -1;
			}
		}else if(n_read == 0){//EOF
			break;
		}

		n_left -= n_read;
		buf_p += n_read;
	}

	return (n - n_left);
}

ssize_t rio_read_line_b(rio_t *rp, void *usr_buf, size_t max_len){
	size_t n;
	ssize_t rc;

	char ch, *buf_p = (char *)usr_buf;

	for(n = 1; n < max_len; n++){
		rc = rio_read(rp, &ch, 1);
		if(rc == 1){
			*buf_p++ = ch;

			if(ch == '\n'){
				break;
			}
		}else if(rc == 0){
			if(n == 1){// 碰到EOF，没有读取到数据
				return 0;
			}else{// 碰到EOF，但是有读取到数据
				break;
			}
		}else if(rc == -EAGAIN){
			return rc;
		}else{
			log_error("rio_read_line_b() : rio_read() error, errno = %d\n", errno);
			return -1;
		}		
	}
	*buf_p = 0;

	return n;//返回读取到的数据：读取到的这一行有多少个字节
}
