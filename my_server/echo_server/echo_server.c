/*####################################################
# File Name: echo_server.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:03:33
# Last Modified: 2017-04-19 13:48:22
####################################################*/
#include "server_header.h"

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
		port = strtol(argv[1], &endptr, 0);
		if(*endptr){
			fprintf(stderr, "echo_server: invalid port number!\n");
			exit(EXIT_FAILURE);
		}
	}else if(argc < 2){
		port = ECHO_PORT;
	}else{
		fprintf(stderr, "echo_server: invalid arguments!\n");
		exit(EXIT_FAILURE);
	}

	/*  Create the listening socket  */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0){
		fprintf(stderr, "echo_server: error creating listening socket!\n");
		exit(EXIT_FAILURE);
	}

	//设置端口复用
	int opt = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
	    //perror("setsockopt error");
		fprintf(stderr, "echo_server: setsockopt error!\n");
	    exit(-1);
	}

	/*  Set all bytes in socket address structure to zero, and fill in the relevant data members   */
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*  Bind our socket addresss to the listening socket, and call listen()  */
	ret_val = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret_val < 0){
		fprintf(stderr, "echo_server: error calling bind()!\n");
		exit(EXIT_FAILURE);
	}

	ret_val = listen(listen_fd, LISTENQ);
	if(ret_val < 0){
		fprintf(stderr, "echo_server: error calling listen()!\n");
		exit(EXIT_FAILURE);
	}

	//若这里不定义 client_addr，则accept函数中的后两个参数传 NULL
	/*
	struct sockaddr_in  client_addr;
	socklen_t len = sizeof(client_addr);
	*/

	/* 只能跟一个client交互一次 */
	/*  Enter an infinite loop to respond to client requests and echo input  */
	while(1){
		/*  Wait for a connection, then accept() it  */
		conn_fd = accept(listen_fd, NULL, NULL);
		//conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
		if(conn_fd < 0){
			fprintf(stderr, "echo_server: error calling accept()!\n");
			exit(EXIT_FAILURE);
		}

		memset(&buffer, 0, sizeof(buffer));
		/*  Retrieve an input line from the connected socket then simply write it back to the same socket.     */
		read_line(conn_fd, buffer, MAX_LINE-1);
		fprintf(stdout, "-----read-----%s\n", buffer);
		write_line(conn_fd, buffer, strlen(buffer));
		fprintf(stdout, "-----write-----%s\n", buffer);

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
