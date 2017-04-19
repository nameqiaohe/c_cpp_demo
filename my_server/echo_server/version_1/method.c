/*####################################################
# File Name: method.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:34:41
# Last Modified: 2017-04-19 13:38:47
####################################################*/
#include "method.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

/*  Read a line from a socket  */
ssize_t read_line(int fd, void *ptr, size_t maxlen){
	ssize_t i, rc;
	char c, *buffer;

	buffer = ptr;

	for(i = 1; i < maxlen; i++){
		rc = read(fd, &c, 1);
		if(rc == 1){
			*buffer++ = c;
			if(c == '\n'){//客户端发来的数据中必须要有一个回车符\n，否则会阻塞在这里
				break;
			}
		}else if(rc == 0){
			if(i = 1){
				return 0;
			}else{
				break;
			}
		}else{
			if(errno == EINTR){
				continue;
			}
			return -1;
		}
	}

	*buffer = 0;
	return i;
}

/*  Write a line to a socket  */
ssize_t write_line(int fd, const void *ptr, size_t maxlen){
	size_t left;
	ssize_t written;
	const char *buffer;

	buffer = ptr;
	left = maxlen;

	while(left > 0){
		written = write(fd, buffer, left);
		if(written <= 0){
			if(errno == EINTR){
				written = 0;
			}else{
				return -1;
			}
		}

		left -= written;
		buffer += written;
	}

	return maxlen;
}
