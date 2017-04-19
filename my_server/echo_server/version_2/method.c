/*####################################################
# File Name: method.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:34:41
# Last Modified: 2017-04-19 19:39:36
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
