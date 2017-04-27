/*####################################################
# File Name: common.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 17:42:47
# Last Modified: 2017-04-27 18:14:32
####################################################*/
#include "common.h"

/* 创建监听套接字
 * 返回值：错误返回 -1，正确返回套接字描述符*/
int socket_create(int port){
	int sock_fd;
	int opt = 1;
	struct sockaddr_in server_addr;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		perror("socket_create : socket() error");
		return -1;
	}

	//设置本地套接字地址
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//设置端口复用
	int ret_val = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if(ret_val == -1){
		close(sock_fd);
		perror("socket_create : setsockopt() error");
		return -1;
	}

	ret_val = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret_val < 0){
		close(sock_fd);
		perror("socket_create : bind() error");
		return -1;
	}

	ret_val = listen(sock_fd, 5);
	if(ret_val < 0){
		close(sock_fd);
		perror("socket_create : listen() error");
		return -1;
	}

	return sock_fd;
}

/* 套接字接受请求
 * 返回值：错误返回 -1，正确返回新的连接套接字*/
int socket_accept(int sock_listen){
	int sock_fd;
	struct sockaddr_in client_addr;
	socketlen_t client_len = sizeof(client_addr);

	sock_fd = accept(sock_listen, (struct sockaddr *)&client_addr, &client_len);
	if(sock_fd < 0){
		perror("socket_accept : accept() error");
		return -1;
	}

	return sock_fd;
}

/* 连接远端主机--服务器 */
int socket_connect(int port, char *host){
	int sock_fd;
	struct sockaddr_in server_addr;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		perror("socket_connect : socket() error");
		return -1;
	}

	//设置协议地址
	memset(&server_addr, 0. sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(host);

	int ret_val = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret_val < 0){
		perror("socket_connect : connect() error");
		close(sock_fd);
		return -1;
	}

	return sock_fd;
}

int recv_data(int sock_fd, char *buf, int buf_size){
	size_t num_bytes;
	bzero(buf, buf_size);

	num_bytes = recv(sock_fd, buf, buf_size, 0);
	if(num_bytes < 0){
		return -1;
	}

	return num_bytes;
}

/* 去除字符串中的空格和换行符 */
void trim_str(char *str, int n){
	int i;
	for(i = 0; i < n; i++){
		if(isspace(str[i])){
			str[i] = 0;
		}

		if(str[i] == '\n'){
			str[i] = 0;
		}
	}
}

/* 发送响应码到 sockfd 
 * 返回值：错误返回 -1，正确返回 0*/
int send_response(int sock_fd, int rc){
	int conv = htonl(rc);
	int ret_val = send(sock_fd, &conv, sizeof(conv), 0);
	if(ret_val < 0){
		perror("send_response : send() error");
		return -1;
	}

	return 0;
}

void read_input(char *buf, int size){
	char *index = NULL;
	bzero(buf, 0, size);

	if(fgets(buf, size, stdin) != NULL){
		index = strchr(buf, '\n');
		if(index != NULL){
			*index = '\0';// 出现换行符，则将该位置字符置'\0'（字符串结尾）
		}
	}
}
