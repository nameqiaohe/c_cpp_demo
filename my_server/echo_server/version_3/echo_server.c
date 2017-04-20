/*####################################################
# File Name: echo_server.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:03:33
# Last Modified: 2017-04-20 19:40:09
####################################################*/
#include "header.h"

int main(int argc, char *argv[]){
	int listen_fd;
	int conn_fd;
	short int port;
	struct sockaddr_in server_addr;	/*  socket address structure  */
	char buffer[MAX_LINE];
	char *endptr;	/*  for strtol()    */

	int ret_val = 0;

	/*  Get port number from the command line, and set to default port if no arguments were supplied  */
	if(argc == 2){
		//给 port赋值的两种方式
#if 0
		port = strtol(argv[1], &endptr, 0);
		if(*endptr){
			fprintf(stderr, "echo_server: invalid port number!\n");
			exit(EXIT_FAILURE);
		}
#endif
		port = atoi(argv[1]);
	}else if(argc < 2){
		port = ECHO_PORT;
	}else{
		fprintf(stderr, "echo_server: invalid arguments!\n");
		exit(EXIT_FAILURE);
	}

	listen_fd = open_listen_fd(port);

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	struct hostent *hp;
	char *haddr;

	/* 只能跟一个client交互一次 */
	/*  Enter an infinite loop to respond to client requests and echo input  */
	while(1){
		/*  Wait for a connection, then accept() it  */
		conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
		if(conn_fd < 0){
			fprintf(stderr, "echo_server: error calling accept()!\n");
			exit(EXIT_FAILURE);
		}

#if 0
		/* Determine the domain name and IP address of the client */
		hp = gethostbyaddr(&client_addr.sin_addr.s_addr, sizeof(client_addr.sin_addr.s_addr), AF_INET);
		if(hp == NULL){
			fprintf(stderr, "echo_server: error calling gethostbyaddr() %d!\n", h_errno);
			exit(EXIT_FAILURE);
		}
		haddr = inet_ntoa(client_addr.sin_addr);
		printf("echo_server: server connected to %s %s\n", hp->h_name, haddr);
#endif
		//printf("echo_server--ip:0x%x, port:%d\n", ntohl(client_addr.sin_addr.s_addr), ntohs(client_addr.sin_port));
		printf("echo_server--ip:%s, port:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));// inet_ntoa() 将网络地址转为 点分字符串格式

		size_t n;
		rio_t rio;
		rio_read_initb(&rio, conn_fd);
		n = rio_read_lineb(&rio, buffer, MAX_LINE);
		if(n < 0){
			fprintf(stderr, "echo_server: error calling rio_read_lineb()!\n");
		}
		while(n != 0){
			fprintf(stderr, "echo_server--server received %d bytes\n", (int)n);
			ret_val = rio_writen(conn_fd, buffer, n);
			if(ret_val != n){
				fprintf(stderr, "echo_server: error calling rio_writen()!\n");
			}
			n -= ret_val;
		}

		/*  Close the connected socket  */
		ret_val = close(conn_fd);
		if(ret_val < 0){
			fprintf(stderr, "echo_server: error close conn_fd!\n");
			exit(EXIT_FAILURE);
		}
	}
	/*  Close the listening socket  */
	ret_val = close(listen_fd);
	if(ret_val < 0){
		fprintf(stderr, "echo_server: error close listen_fd!\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
