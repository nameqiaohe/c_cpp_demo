/*####################################################
# File Name: client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 16:51:12
# Last Modified: 2016-12-13 20:10:52
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
	pthread_t tid;
	int err;
	err = pthread_create(&tid, NULL, thread_func, NULL);//接收 server 端发来的消息
	assert(!err);

	bzero(sendBuf, BUF_SIZE);
	bzero(recvBuf, BUF_SIZE);

	//终端输入
	init();
	play();

	//若达到获胜标志则给其他 client 发送一个消息
	if(map.m_maxNumber == map.m_winFlag){
		sprintf(sendBuf, "%d", map.m_maxNumber);
		write(connfd, sendBuf, strlen(sendBuf));

		printf("\n\t\e[1;5;33mCongratulations, You Win!\e[0m\n\n");//取胜，输出提示
	}
}

void *thread_func(){
	int nread;
	//收到服务器响应
	nread = read(connfd, recvBuf, BUF_SIZE);
	if(nread < 0){
		perror("read error");
	}else if(nread == 0){
		printf("Server close the connection\n");
	}else{
		write(STDOUT_FILENO, recvBuf, nread);
		//printf("\n\tSorry, You Lose!\n\n");//这里的失败提示没有在终端看到，可能是在游戏界面输出了，返回到终端之后看不到而已
		raise(SIGINT);
	}
	return (void *)0;
}

void handler(int sig){
	if(sig == SIGINT){
		gameOver();
		printf("\n\t\e[1;32mSorry, You Lose!\e[0m\n\n");
		printf("exiting....\n");
		exit(0);
	}
}
