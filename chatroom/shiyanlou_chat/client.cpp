#include "utility.h"

int main(int argc, char *argv[]){
	//client connect server ip + port
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		perror("socket error");
		exit(-1);
	}

	//connect the server
	if(connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
		perror("connect error");
		exit(-1);
	}

	//create pipe, fd[0] for parent read, fd[1] for child write
	int pipefd[2];
	if(pipe(pipefd) < 0){
		perror("pipe error");
		exit(-1);
	}

	//create epoll
	int epfd = epoll_create(EPOLL_SIZE);
	if(epfd < 0){
		perror("epoll_create error");
		exit(-1);
	}

	//add sockfd and pipefd[0] to the kernel events list
	static struct epoll_event events[2];
	addfd(epfd, sockfd, true);
	addfd(epfd, pipefd[0], true);

	bool isClientWork = true;

	//chat message buffer
	char message[BUF_SIZE];

	//fork
	int pid = fork();
	if(pid < 0){
		perror("fork error");
		exit(-1);
	}else if(pid == 0){//child
		//child write pipe, so close read side
		close(pipefd[0]);
		printf("Enter 'EXIT' to exit the chat room\n");

		while(isClientWork){
			bzero(&message, BUF_SIZE);
			fgets(message, BUF_SIZE, stdin);

			//client exit
			if(strncasecmp(message, EXIT, strlen(EXIT)) < 0){
				isClientWork = false;
			}else{//child write msg to pipe
				if(write(pipefd[1], message, strlen(message)-1) < 0){
					perror("write pipe error");
					exit(-1);
				}
			}

		}
	}else{//parent
		//parent read, so close pipe write side
		close(pipefd[1]);

		while(isClientWork){
			int epoll_events_count = epoll_wait(epfd, events, 2, -1);
			//handle ready event
			for(int i = 0; i < epoll_events_count; ++i){
				bzero(&message, BUF_SIZE);

				//message from server
				if(events[i].data.fd = sockfd){
					//recv server message
					int ret = recv(sockfd, message, BUF_SIZE, 0);
					if(ret == 0){//server closed
						printf("Server closed connection: %d\n", sockfd);
						close(sockfd);
						isClientWork = false;
					}else{
						printf("%s\n", message);
					}
				}else{//child write event happened, parent handle and send to server
					//parent read message from pipe
					int ret = read(events[i].data.fd, message, BUF_SIZE);
					if(ret == 0){//server closed
						isClientWork = false;
					}else{
						send(sockfd, message, BUF_SIZE, 0);
					}
				
				}
			}
		}
	}
	if(pid){//pid != 0
		close(pipefd[0]);
		close(sockfd);
	}else{//pid == 0
		close(pipefd[1]);
	}

	return 0;
}
