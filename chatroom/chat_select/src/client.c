/*####################################################
# File Name: client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 16:51:12
# Last Modified: 2016-12-10 18:20:06
####################################################*/
#include "../include/header.h"
#include "../include/client.h"

int main(int argc, char *argv[]){
	int connfd;
	socklen_t socklen = sizeof(struct sockaddr_in);

	struct sockaddr_in serverAddr;

	connfd = socket(AF_INET, SOCK_STREAM, 0);
	if(connfd < 0){
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	bzero(&serverAddr, socklen);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if(connect(connfd, (struct sockaddr *)&serverAddr, socklen) < 0){
		perror("connect error");
		exit(EXIT_FAILURE);
	}

	printf("Welcome to here\n");

	handle(connfd);
	close(connfd);
	printf("exit\n");

	exit(0);
}

void handle(int connfd){
	char sendBuf[BUF_SIZE] = {0};
	char recvBuf[BUF_SIZE] = {0};
	fd_set rset;

	FD_ZERO(&rset);

	int maxfd = max(fileno(stdin), connfd) + 1;
	int nread;

	while(1){
		bzero(sendBuf, BUF_SIZE);
		bzero(recvBuf, BUF_SIZE);

		FD_SET(fileno(stdin), &rset);
		FD_SET(connfd, &rset);

		if(select(maxfd, &rset, NULL, NULL, NULL) == -1){
			perror("select error");
			continue;
		}

		//收到服务器响应
		if(FD_ISSET(connfd, &rset)){
			nread = read(connfd, recvBuf, BUF_SIZE);
			if(nread < 0){
				perror("read error");
				break;
			}else if(nread == 0){
				printf("Server close the connection\n");
				break;
			}else{
				write(STDOUT_FILENO, recvBuf, nread);
			}
		}

		//终端输入
		if(FD_ISSET(fileno(stdin), &rset)){
			if(fgets(sendBuf, BUF_SIZE, stdin) == NULL){
				break;
			}else{
				write(connfd, sendBuf, strlen(sendBuf));
			}
		}
	}
}
