/*####################################################
# File Name: client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 16:51:12
# Last Modified: 2016-12-11 17:59:50
####################################################*/
#include "../include/header.h"
#include "../include/client.h"
#include "../include/game.h"

int main(int argc, char *argv[]){
	initClientValue();

	socklen_t socklen = sizeof(struct sockaddr_in);

	struct sockaddr_in serverAddr;

	signal(SIGINT, handler);	

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

	whileProcess(connfd);
	close(connfd);
	printf("exit\n");

	exit(0);
}

void initClientValue(){
	connfd = -1;
	FD_ZERO(&rset);

	bzero(sendBuf, BUF_SIZE);
	bzero(recvBuf, BUF_SIZE);
}

void whileProcess(int connfd){
	int maxfd = max(fileno(stdin), connfd) + 1;

	printf("press Enter, start 2048 game\n");

	pthread_t tid;
	int err;
	err = pthread_create(&tid, NULL, thread_func, NULL);
	assert(!err);

	while(1){
		bzero(sendBuf, BUF_SIZE);
		bzero(recvBuf, BUF_SIZE);

		FD_SET(fileno(stdin), &rset);
		FD_SET(connfd, &rset);

		if(select(maxfd, &rset, NULL, NULL, NULL) == -1){
			perror("select error");
			continue;
		}	

		//终端输入
		if(FD_ISSET(fileno(stdin), &rset)){
			init();
			play();
			endwin();
		}
		if(gameOverFlag != 0){
			break;
		}
	}
}

void *thread_func(){
	int nread;
	//收到服务器响应
	if(FD_ISSET(connfd, &rset)){
		nread = read(connfd, recvBuf, BUF_SIZE);
		if(nread < 0){
			perror("read error");
		}else if(nread == 0){
			printf("Server close the connection\n");
		}else{
			write(STDOUT_FILENO, recvBuf, nread);
			raise(SIGINT);
		}
	}
	return (void *)0;
}

void handler(int sig){
	if(sig == SIGINT){
		gameOver();
		//gameOverFlag = 1;
		printf("exiting....\n");
		exit(0);
	}
}
