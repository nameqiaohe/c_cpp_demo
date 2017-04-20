/*####################################################
# File Name: method.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:34:41
# Last Modified: 2017-04-20 21:27:18
####################################################*/
#include "header.h"
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

/* Associate a descriptor with a read buffer and reset buffer */
void rio_read_initb(rio_t *rp, int fd){
	rp->rio_fd = fd;//rio_fd 与缓冲区绑定的文件描述符
	rp->rio_cnt = 0;//rio_cnt 缓冲区中还未读取的字节数
	rp->rio_bufptr = rp->rio_buf;//rio_buf_ptr 当前下一个未读取字符的地址
	//rio_buf 缓冲区，其大小为 8192 byte
}

/* RIO--Robust I/O包 底层读取函数。当缓冲区数据充足时，此函数直接拷贝缓
 * 冲区的数据给上层读取函数；当缓冲区不足时，该函数通过系统调用
 * 从文件中读取最大数量的字节到缓冲区，再拷贝缓冲区数据给上层函数
 * 参数：rp 里面包含了文件描述符和其对应的缓冲区数据
 *		usrbuf 读取的目的地址
 *		n 读取的字节数量
 * 返回值：返回真正读取到的字节数（<=n）*/
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
	int cnt;

	while(rp->rio_cnt <= 0){//缓冲区中没有数据了
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt < 0){//读取出错
			if(errno != EINTR){//若出错不是由 中断 引起的，返回错误
				return -1;
			}
		}else if(rp->rio_cnt == 0){//读取遇到EOF
			return 0;
		}else{//读取正确
			rp->rio_bufptr = rp->rio_buf;//重置 rio_bufptr，指向缓冲区内第一个字节
		}
	}

#if 0
	cnt = n;
	if((size_t)rp->rio_cnt < n){
		cnt = rp->rio_cnt;
	}
#endif
	cnt = ((size_t)rp->rio_cnt < n) ? rp->rio_cnt : n;
	/* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
	memcpy(usrbuf, rp->rio_bufptr, cnt);//从缓冲区中未读取的字节开始拷贝
	rp->rio_bufptr += cnt;//读取后需要更新指针
	rp->rio_cnt -= cnt;//未读取字节减少

	return cnt;
}

/* 不带缓冲区的rio_read--robustly read n bytes (unbuffered) */
size_t rio_readn(int fd, void *usrbuf, size_t n){
	size_t left_cnt = n;
	ssize_t nread;
	char *buf = usrbuf;

	while(left_cnt > 0){
		nread = read(fd, buf, left_cnt);
		if(nread < 0){//读取错误
			if(errno == EINTR){//若读取错误是由 中断 引起的，则继续读取
				nread = 0;
			}else{//若读取错误不是由 中断 引起的，返回错误 
				return -1;
			}
		}else if(nread == 0){
			break;
		}

		left_cnt -= nread;
		buf += nread;
	}

	return (n - left_cnt);
}

/* 供用户使用的读取函数。从缓冲区中读取最大maxlen字节数据
 * 参数：rp 文件描述符与其对应的缓冲区
 *		usrbuf 目的地址
 *		n 用户想要读取的字节数量
 * 返回值：真正读取到的字节数。读到EOF返回0,读取失败返回-1。*/
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n){
	size_t left_cnt = n;
	ssize_t nread;
	char *buf = usrbuf;

	while(left_cnt > 0){
		nread = rio_read(rp, buf, left_cnt);
		if(nread < 0){//读取错误
			if(errno == EINTR){//若读取错误是由 中断 引起的，则继续读取
				nread = 0;
			}else{//若读取错误不是由 中断 引起的，返回错误
				return -1;
			}
		}else if(nread == 0){//遇到EOF
			break;
		}	

		left_cnt -= nread;
		buf += nread;
	}
	return (n - left_cnt);
}

ssize_t rio_read_lineb(rio_t *rp, void *usrbuf, size_t maxlen){
	int n, nread;
	char ch, *buf = usrbuf;

	for(n = 1; n < maxlen; n++){
		nread = rio_read(rp, &ch, 1);
		if(nread == 1){
			*buf++ = ch;
			if(ch == '\n'){
				break;
			}
		}else if(nread == 0){
			if(n == 1){/* EOF, no data read */
				return 0;
			}else{/* EOF, some data was read */
				break;
			}
		}else{/* error */
			return -1;
		}
	}
	*buf = 0;//设置字符串结束标志

	return n;
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n){
	size_t left_cnt = n;
	ssize_t nwritten;
	char *buf = usrbuf;

	while(left_cnt > 0){
		nwritten = write(fd, buf, left_cnt);
		if(nwritten <= 0){//写错误
			if(errno == EINTR){//若写错误是由 中断 引起的，则继续写
				nwritten = 0;
			}else{//若写错误是由 中断 引起的，则返回错误
				return -1;
			}
		}

		left_cnt -= nwritten;
		buf += nwritten;
	}

	return n;
}

/* open and return a listening socket on port
 * Returns -1 and sets errno on Unix error.
*/
int open_listen_fd(int port){
	int listen_fd, optval = 1;
	struct sockaddr_in server_addr;
	int ret_val;

	/* Create a socket descriptor */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0){
		fprintf(stderr, "open_listen_fd : error calling socket()!\n");
		//exit(EXIT_FAILURE);
		return -1;
	}

	/* Eliminates "Address already in use" error from bind */
	ret_val = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
	if(ret_val < 0){
		fprintf(stderr, "open_listen_fd : error calling setsockopt()!\n");
		return -1;
	}

	/* Listenfd willl be an end point for all request to port on any IP address for this host */
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);

	ret_val = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret_val < 0){
		fprintf(stderr, "open_listen_fd : error calling bind()!\n");
		return -1;
	}

	/* Make it a listening socket ready to accept connection requests */
	ret_val = listen(listen_fd, LISTENQ);
	if(ret_val < 0){
		fprintf(stderr, "open_listen_fd : error calling listen()!\n");
		return -1;
	}

	return listen_fd;
}


/* open connection to server at <hostname, port>, and return a socket descriptor ready for reading and writing.
 * Returns -1 and sets errno on Unix error. error calling socket() or connect()
 * Returns -2 and sets h_errno on DNS (gethostbyname) error. error calling gethostbyname()
*/
int open_client_fd(char *hostname, int port){
	int client_fd;
	struct hostent *hp;
	struct sockaddr_in server_addr;

	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(client_fd < 0){
		fprintf(stderr, "open_client_fd: error calling socket()!\n");
		//exit(EXIT_FAILURE);
		return -1;
	}

	/* Fill in the server's IP address and port */
	hp = gethostbyname(hostname);
	if(hp == NULL){
		fprintf(stderr, "open_client_fd: error calling gethostbyname()!\n");
		//exit(EXIT_FAILURE);
		return -1;
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, hp->h_length);
	server_addr.sin_port = htons(port);

	/* Establish a connection with the server */
	int ret_val = connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret_val < 0){
		fprintf(stderr, "open_client_fd: error calling connect()!\n");
		//exit(EXIT_FAILURE);
		return -1;
	}

	return client_fd;
}
