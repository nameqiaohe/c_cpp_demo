#include "utility.h"

int main(int argc, char *argv[]){
	//server ip + port
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//create listenfd
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0){
		perror("socket error");
		exit(-1);
	}
	printf("listen socket create\n");

	//bind address
	if(bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
		perror("bind error");
		exit(-1);
	}

	//listen
	int ret = listen(listenfd, BACK_LOG);
	if(ret < 0){
		perror("listen error");
		exit(-1);
	}
	printf("Start to listen: %s\n", SERVER_IP);

	//create kernel event list
	int epfd = epoll_create(EPOLL_SIZE);
	if(epfd < 0){
		perror("epoll_create error");
		exit(-1);
	}
	printf("epoll created, epollfd = %d\n", epfd);

	//add event to the list of kernel events
	static struct epoll_event events[EPOLL_SIZE];
	addfd(epfd, listenfd, true);

	while(1){
		//ready events
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		if(epoll_events_count < 0){
			perror("epoll_wait error");
			break;
		}
		printf("epoll_events_count = %d\n", epoll_events_count);

		//handle the ready events
		for(int i = 0; i < epoll_events_count; ++i){
			int sockfd = events[i].data.fd;
			//new client connect
			if(sockfd == listenfd){
				struct sockaddr_in clientAddr;
				socklen_t clientAddrLen = sizeof(struct sockaddr_in);
				int clientfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
				printf("client connection from %s : %d(IP : port), clientfd = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientfd);

				addfd(epfd, clientfd, true);

				//server send welcome message
				printf("Welcome message\n");
				char message[BUF_SIZE];
				bzero(message, BUF_SIZE);
				sprintf(message, SERVER_WELCOME, clientfd);

				int ret = send(clientfd, message, BUF_SIZE, 0);
				if(ret < 0){
					perror("send error");
					exit(-1);
				}
			}else{//handle the message from client, and broadcast, let the other client recvd
				int ret = sendBroadcastMsg(sockfd);
				if(ret < 0){
					perror("sendBroadcastMsg error");
					exit(-1);
				}
			}
		}
	}
	close(listenfd);
	close(epfd);

	return 0;
}
