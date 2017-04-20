/*####################################################
# File Name: echo_client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-19 11:53:54
# Last Modified: 2017-04-20 20:02:08
####################################################*/
#include "header.h"

int main(int argc, char *argv[]){
	int port;
	char *host;
	int ret_val;

	/*  Get port number from the command line, and set to default port if no arguments were supplied  */
#if 0
	if(argc != 3){
		fprintf(stderr, "echo_client : usage %s <host> <port>!\n", argv[0]);
		exit(0);
	}
#endif
	if(argc == 3){
		host = argv[1];
		port = atoi(argv[2]);	
	}else if(argc == 2){
		host = argv[1];		
		port = ECHO_PORT;
	}else{
		fprintf(stderr, "echo_client : usage %s <host> <port> or just <host>!\n", argv[0]);
		exit(0);
	}

	rio_t rp;
	int client_fd = open_client_fd(host, port);
	rio_read_initb(&rp, client_fd);

	char buffer[MAX_LINE];
	while(fgets(buffer, MAX_LINE, stdin) != NULL){
		ret_val = rio_writen(client_fd, buffer, strlen(buffer));
		if(ret_val != strlen(buffer)){
			fprintf(stderr, "echo_client : error calling rio_writen()!\n");
			exit(EXIT_FAILURE);
		}

		rio_read_lineb(&rp, buffer, MAX_LINE);

		if(fputs(buffer, stdout) == EOF){
			fprintf(stderr, "echo_client : error calling fputs()!\n");
		}
	}

	close(client_fd);//结束之后，关闭sock_fd

	return 0;
}
