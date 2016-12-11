/*####################################################
# File Name: client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 16:51:12
# Last Modified: 2016-12-11 17:38:01
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
	char tempStr[BUF_SIZE] = {0};
	fd_set rset;

	FD_ZERO(&rset);

	int maxfd = max(fileno(stdin), connfd) + 1;
	int nread;

	while(1){
		bzero(sendBuf, BUF_SIZE);
		bzero(recvBuf, BUF_SIZE);
		bzero(tempStr, BUF_SIZE);

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
				//strcpy(tempStr, "\e[33m>>\e[0m");
				//strcpy(tempStr, ">>>> ");
				//strncpy(tempStr+strlen(tempStr), recvBuf, sizeof(recvBuf));
				//printf("\e[33m%s\e[0m", tempStr);
				//write(STDOUT_FILENO, tempStr, strlen(tempStr));
				//write(STDOUT_FILENO, recvBuf, nread);
				sprintf(tempStr, "\e[33m>>>> %s\e[0m", recvBuf);
				write(STDOUT_FILENO, tempStr, strlen(tempStr));
			}
		}

		//终端输入
		if(FD_ISSET(fileno(stdin), &rset)){
			printf("\r\e[0m\e[0m");
			if(fgets(sendBuf, BUF_SIZE, stdin) == NULL){//如果输入回车，则另一个客户端显示出来的是空，怎么过滤这种情况？？？
				break;
			}else{
				write(connfd, sendBuf, strlen(sendBuf));
			}
		}
	}
}
