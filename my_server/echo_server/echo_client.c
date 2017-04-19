/*####################################################
# File Name: echo_client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-19 11:53:54
# Last Modified: 2017-04-19 13:52:32
####################################################*/
#include "server_header.h"

int main(int argc, char *argv[]){
	int port;
	char *end_ptr;

	/*  Get port number from the command line, and set to default port if no arguments were supplied  */
	if(argc == 2){
		port = strtol(argv[1], &end_ptr, 0);
		if(*end_ptr){
			fprintf(stderr, "echo_client: invalid port number!\n");
			exit(EXIT_FAILURE);
		}
	}else if(argc < 2){
		port = ECHO_PORT;
	}else{
		fprintf(stderr, "echo_client: invalid arguments!\n");
		exit(EXIT_FAILURE);
	}

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0){
		fprintf(stderr, "echo_client: error calling socket!\n");
		exit(EXIT_FAILURE);
	}

	//怎么样自动获取本机的IP地址？？？
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr("192.168.1.120");

	int ret_val = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret_val < 0){
		fprintf(stderr, "echo_client: error calling connect!\n");
		exit(EXIT_FAILURE);
	}

	char buffer[MAX_LINE] = {0};//读终端、读server用的是同一个buffer
	char read_msg[MAX_LINE] = {0};
	int write_len, read_len;

	/* 只能跟server交互一次
	 * 若想与server交互多次，加一个while循环，对应的还需修改server端的处理流程 */

	bzero(buffer, MAX_LINE);//清空buffer，用于读取终端的输入
	fgets(buffer, MAX_LINE, stdin);//从终端读取
	//write_len = write(sock_fd, buffer, strlen(buffer)-1);//发送给server. //fgets 会读入 换行符\n
	write_len = write_line(sock_fd, buffer, strlen(buffer));

	bzero(read_msg, MAX_LINE);//清空buffer，用于接收server发来的数据
	//read_len = read_line(sock_fd, read_msg, MAX_LINE);
	read_len = read(sock_fd, read_msg, MAX_LINE);
	if(read_len < 0){
		fprintf(stderr, "echo_client: error calling read!\n");
		close(sock_fd);//结束之后，关闭sock_fd
		exit(EXIT_FAILURE);
	}else if(read_len == 0){
		fprintf(stderr, "echo_client: server close the connection!\n");
	}else{
		fprintf(stdout, "echo_client--read server msg : %s\n", read_msg);
	}

	close(sock_fd);//结束之后，关闭sock_fd

	return 0;
}
