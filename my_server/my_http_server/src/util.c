/*####################################################
# File Name: util.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 19:52:16
# Last Modified: 2017-05-18 19:58:00
####################################################*/
#include "util.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>

#include "dbg.h"

int open_listen_fd(int port){
	if(port <= 0){
		port = 3000;
	}

	int listen_fd, opt_val = 1;
	struct sockaddr_in server_addr;
	
	// 创建监听 socket描述符
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		log_error("socket() failed!");
		return -1;
	}

	// 避免 bind() 时出错： "Address already in use"
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt_val, sizeof(int)) < 0){
		log_error("setsockopt() failed!");
		close(listen_fd);
		return -1;
	}

	//本地IP、协议、端口号赋值
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	// 绑定本地IP、端口
	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		log_error("bind() failed!");
		close(listen_fd);
		return -1;
	}

	if(listen(listen_fd, LISTENQ) < 0){
		log_error("listen() failed!");
		close(listen_fd);
		return -1;	
	}

	return listen_fd;
}

int make_socket_nonblocking(int fd){
	int flags, rc;
	flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1){
		log_error("fcntl()--F_GETFL");
		return -1;
	}

	flags |= O_NONBLOCK;
	rc = fcntl(fd, F_SETFL, 0);
	if(rc == -1){
		log_error("fcntl()--F_SETFL");
		return -1;
	}

	return 0;
}

int read_conf(char *filename, st_conf_t *cf, char *buf, int len){
	FILE *fp = fopen(filename, "r");
	if(!fp){
		log_error("can not open config file : %s", filename);
		return ST_CONF_ERROR;
	}

	int pos;
	char *delim_pos;//分隔符所在位置
	int line_len;
	char *current_pos = buf + pos;// current_pos指向 buf，buf用来存储读取到的内容

	// 若读到的内容不为空
	// 每次 current_pos 指向的都是新读到的内容
	while(fgets(current_pos, len - pos, fp)){
		delim_pos = strstr(current_pos, DELIM);
		line_len = strlen(current_pos);

		debug("read one line from conf : %s, len = %d", current_pos, line_len);

		if(!delim_pos){
			return ST_CONF_ERROR;
		}

		if(current_pos[strlen(current_pos) - 1] == '\n'){
			current_pos[strlen(current_pos) - 1] = '\0';
		}

		if(strncmp("root", current_pos, 4) == 0){
			cf->root = delim_pos + 1;
		}

		if(strncmp("port", current_pos, 4) == 0){
			cf->port = atoi(delim_pos + 1);
		}

		if(strncmp("thread_num", current_pos, 10) == 0){
			cf->thread_num = atoi(delim_pos + 1);
		}

		current_pos += line_len;// 移动 current_pos 到字符串末尾  // 最终，buf中包含所有读取到的内容
	}

	fclose(fp);

	return ST_CONF_OK;
}
