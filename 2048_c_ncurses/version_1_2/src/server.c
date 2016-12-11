/*####################################################
# File Name: server.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-10 15:22:29
# Last Modified: 2016-12-11 21:16:19
####################################################*/
#include "../include/server.h"

int main(int argc, char *argv[]){
	initVaule();

	int listenfd = createListenFd();
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	FD_SET(fileno(stdin), &allset);
	maxfd = listenfd;

	printServerInfo();

	int i = 0;
	while(1){
		rset = allset;
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if(nready < 0){
			perror("select error");
			exit(EXIT_FAILURE);
		}
		if(FD_ISSET(listenfd, &rset)){
			connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &socklen);
			if(connfd < 0){//如果 accept失败，则跳过，继续等待下一次连接
				perror("accept error");
				continue;
			}
			//accept 成功，打印出客户端信息
			printClientInfo();

			//accept 成功，则将 connfd加入到连接队列中
			for(i = 0; i < FD_SETSIZE; ++i){
				if(clientSockets[i].connectFd < 0){
					clientSockets[i].id = ++clientIndex;
					clientSockets[i].connectFd = connfd;
					clientSockets[i].port = clientAddr.sin_port;

					++currentClients;
					break;
				}
			}
			
			if(i == FD_SETSIZE){
				fprintf(stderr, "too many connections, more than %d\n", FD_SETSIZE);
				close(connfd);
				continue;
			}

			if(connfd > maxfd){
				maxfd = connfd;
			}

			FD_SET(connfd, &allset);
			if(--nready <= 0){
				continue;
			}
		}
		if(FD_ISSET(fileno(stdin), &rset)){
			int ch = fgetc(stdin);
			if(ch == 113 || ch == 81){
				printf("exiting......\n");
				closeAllFds();
				exit(0);
			}
		}
		handle(clientSockets, maxfd, &rset, &allset);

		if(!isAllClientsClosed()){
			printf("\nAll clients were closed, Goodbye!\n");
			break;
		}
	}
	return 0;
}

int createListenFd(){
	int listenfd;
	struct sockaddr_in serverAddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0){
		perror("socket error");
		exit(EXIT_FAILURE);
	}

	//设置端口复用
	int opt = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		perror("setsockopt error");
	}

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listenfd, (struct sockaddr *)&serverAddr, socklen) < 0){
		perror("bind error");
		exit(EXIT_FAILURE);
	}

	if(listen(listenfd, BACK_LOG) < 0){
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	return listenfd;
}

void initVaule(){
	clientIndex = -1;
	currentClients = 0;

	bzero(buf, BUF_SIZE);

	//socklen = sizeof(clientAddr);
	socklen = sizeof(struct sockaddr_in);

	int i = 0;
	for(i = 0; i < FD_SETSIZE; ++i){
		clientSockets[i].id = -1;
		bzero(clientSockets[i].winTime, STR_MAX);
		clientSockets[i].connectFd = -1;
		clientSockets[i].port = -1;
	}
}

void printServerInfo(){
	printf("server use select startup, listen on port : %d\n", SERVER_PORT);
	printf("max connection : %d\n", FD_SETSIZE);
}

void printClientInfo(){
	sprintf(buf, "accept from %s : %d\n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
	printf("%s\n", buf);
#if 0
	printf("connfd = %d\n", connfd);
	printf("IP : %s, Port : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
#endif
}

void handle(User *clientSockets, int maxfd, fd_set *rset, fd_set *allset){
	int nread, i, j;
	char buf[BUF_SIZE];

	for(i = 0; i < maxfd; ++i){
		bzero(buf, BUF_SIZE);
		if(clientSockets[i].connectFd != -1){//从已连接的客户端中取出一个
			if(FD_ISSET(clientSockets[i].connectFd, rset)){//若connectFd在被监测的集合中，表示客户端有数据发来
				nread = read(clientSockets[i].connectFd, buf, BUF_SIZE);
				if(nread < 0){
					perror("read error");
					close(clientSockets[i].connectFd);
					FD_CLR(clientSockets[i].connectFd, allset);
					clientSockets[i].connectFd = -1;
					clientSockets[i].id = -1;
					clientSockets[i].port = -1;
					
					//应该换一种方式来保存当前连接的client
					//--currentClients;//若编号在中间的client关闭，自减之后会导致编号在后边的client无法收到消息，且其对应的connfd无法在server退出之前被关闭
					continue;
				}else if(nread == 0){
					printf("client %d : %d close the connection\n", clientSockets[i].id, clientSockets[i].port);
					close(clientSockets[i].connectFd);
					FD_CLR(clientSockets[i].connectFd, allset);
					clientSockets[i].connectFd = -1;
					clientSockets[i].id = -1;
					clientSockets[i].port = -1;

					//--currentClients;
					continue;
				}else{
					for(j = 0; j < currentClients; ++j){
						if(clientSockets[j].id != clientSockets[i].id){
							write(clientSockets[j].connectFd, buf, nread);//向客户端写数据，这里没有考虑写数据失败的情况
						}
					}
				}
			}
		}
	}
}

void closeAllFds(){
	close(listenfd);
	close(connfd);

	int i = 0;
	for(i = 0; i < currentClients; ++i){
		if(clientSockets[i].connectFd != -1){
			close(clientSockets[i].connectFd);
		}
	}
}

int isAllClientsClosed(){
	int i = 0;
	for(i = 0; i < currentClients; ++i){
		if(clientSockets[i].connectFd != -1){
			return 1;
		}
	}

	return 0;
}
