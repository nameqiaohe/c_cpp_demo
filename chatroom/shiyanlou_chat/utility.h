#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

//clients_list save all the clients's socket
list<int> clients_list;

/************ macro definition ****************/
#define SERVER_IP "127.0.0.1"	//server ip
#define SERVER_PORT 8888	//server port

#define EPOLL_SIZE 5000		//epoll size

#define BACK_LOG 5

#define BUF_SIZE 0xffff		//message buffer size

#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is : Client--%d"

#define SERVER_MSG "ClientID %d say >> %s"	

#define EXIT "EXIT"

#define CAUTION "There is only one in the chat room!"

/****************** functions ******************/
int setnonblocking(int sockfd){
	int ret = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK);
	if(ret != -1){
		return 0;
	}else{
		return -1;
	}
}

void addfd(int epollfd, int fd, bool enable_et){
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if(enable_et != false){
		ev.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblocking(fd);
	printf("fd added to epoll!\n");
}

int sendBroadcastMsg(int clientfd){
	//buf : receive new chat message
	//message : save formatmessage
	char buf[BUF_SIZE], message[BUF_SIZE];
	bzero(buf, BUF_SIZE);
	bzero(message, BUF_SIZE);

	//receive message
	printf("read from clientfd(clientID = %d)\n", clientfd);
	int len = recv(clientfd, buf, BUF_SIZE, 0);
	if(len == 0){//means the client closed connection
		close(clientfd);
		clients_list.remove(clientfd);
		printf("ClientID = %d closed.\nNow there are %d client in the chat room!\n", clientfd, (int)clients_list.size());
	}else{//broadcast message
		if(clients_list.size() == 1){//means there is only one in the chat room
			send(clientfd, CAUTION, strlen(CAUTION), 0);
			return len;
		}
		//format message to broadcast
		sprintf(message, SERVER_MSG, clientfd, buf);
		
		list<int>::iterator it;
		for(it = clients_list.begin(); it != clients_list.end(); ++it){
			if(*it != clientfd){//send the message to other client
				if(send(*it, message, BUF_SIZE, 0) < 0){
					perror("send message error");
					exit(-1);
				}
			}
		}
	}
	return len;
}
#endif //UTILITY
