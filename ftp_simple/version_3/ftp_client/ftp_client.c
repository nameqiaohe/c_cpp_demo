/*####################################################
# File Name: ftp_client.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 18:18:38
# Last Modified: 2017-05-03 01:42:58
####################################################*/
#include "ftp_client.h"

int sock_control;

/* 接收服务器响应
 * 返回值：错误返回 -1，正确返回状态码*/
int read_reply(){
	int ret_code = 0;
	int ret_val = recv(sock_control, &ret_code, sizeof(ret_code), 0);
	if(ret_val < 0){
		perror("read_reply : recv() error");
		return -1;
	}

	return ntohl(ret_code);
}

/* 打印响应信息 */
void print_reply(int rc){
	switch(rc){
		case 220:
			printf("print_reply : 220--Welcome, Service ready for new user.\n");
			break;
		case 221:
			printf("print_reply : 221--Service closing control connection.\n");
			break;
		case 226:
			printf("print_reply : 226--Closing data connection. Requested file action successful (for example, file transfer or file abort).\n");
			break;
		case 550:
			printf("print_reply : 550--Requested action not taken. File unavailable (e.g., file not found, no access).\n");
			break;
	}
}

/* 解析命令行到结构体 */
int ftp_client_read_cmd(char *buf, int size, struct command *cmd_str){
	memset(cmd_str->code, 0, sizeof(cmd_str->code));
	memset(cmd_str->arg, 0, sizeof(cmd_str->arg));

	printf("ftp_client> ");//提示符
	fflush(stdout);

	read_input(buf, size);//获取用户输入命令

	char *arg = NULL;
	arg = strtok(buf, " ");
	arg = strtok(NULL, " ");

	if(arg != NULL){
		strncpy(cmd_str->arg, arg, strlen(arg));
	}

	if(strcmp(buf, "list") == 0){
		strcpy(cmd_str->code, "LIST");
	}else if(strcmp(buf, "get") == 0){
		strcpy(cmd_str->code, "RETR");
	}else if(strcmp(buf, "quit") == 0){
		strcpy(cmd_str->code, "QUIT");
	}else{
		return -1;//非法命令
	}

	memset(buf, 0, strlen(buf));
	strcpy(buf, cmd_str->code);//存储命令到 buf 开始处

	if(arg != NULL){
		strcat(buf, " ");
		strncat(buf, cmd_str->arg, strlen(cmd_str->arg));
	}

	return 0;
}

/* 实现 get <filename> 命令行 */
int ftp_client_get(int sock_fd, int sock_control, char *arg){
	char data[MAXSIZE];
	int size;
	FILE *fp = fopen(arg, "w");//创建并打开名字为 arg 的文件

	while((size = recv(sock_fd, data, MAXSIZE, 0)) > 0){
		fwrite(data, 1, size, fp);
	}

	if(size < 0){
		perror("ftp_client_get : recv() error");
	}

	fclose(fp);
	return 0;
}

/* 打开数据连接 */
int ftp_client_open_conn(int sock_conn){
	int sock_listen = socket_create(CLIENT_PORT);

	int ack = 1;
	if((send(sock_conn, (char *)&ack, sizeof(ack), 0)) < 0){
		printf("ftp_client_open_conn : ack write error : %d\n", errno);
		exit(EXIT_FAILURE);
	}

	int sock_connection = socket_accept(sock_listen);
	close(sock_listen);
	return sock_connection;
}

/* list 命令 */
int ftp_client_list(int sock_fd, int sock_conn){
	size_t num_recvd;
	char buf[MAXSIZE];
	int tmp = 0;

	/* 等待服务器启动的信息 */
	int ret_val = recv(sock_conn, &tmp, sizeof(tmp), 0);
	if(ret_val < 0){
		perror("ftp_client_list : error reading message from server, waitting server start");
		return -1;
	}

	memset(buf, 0, sizeof(buf));

	unsigned int tip_flag = 0;

	/* 接收服务器传来的数据 */
	while((num_recvd = recv(sock_fd, buf, MAXSIZE, 0)) > 0){
		if(tip_flag == 0){
			printf("ftp_client_list : recv msg from server :\n");
			tip_flag++;
		}
		printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}

	if(num_recvd < 0){
		perror("ftp_client_list : recv() error");
	}

	/* 等待服务器完成的消息 */
	ret_val = recv(sock_conn, &tmp, sizeof(tmp), 0);
	if(ret_val < 0){
		perror("ftp_client_list : error reading message from server, waitting server done");
		return -1;
	}

	return 0;
}

/* 输入含有命令(code)和参数(arg)的 command(cmd) 结构,连接 code + arg,并放进一个字符串，然后发送给服务器 */
int ftp_client_send_username_password(struct command *cmd){
	char buf[MAXSIZE];
	int rc;

	sprintf(buf, "%s", cmd->arg);

	rc = send(sock_control, buf, (int) strlen(buf), 0);
	if(rc < 0){
		perror("ftp_client_send_username_password : Error sending command to server");
		return -1;
	}

	return 0;
}

/* 获取登录信息,发送到服务器认证 */
void ftp_client_login(){
	struct command cmd;
	char user[256];
	char *pass;

	int count = LOGIN_COUNT;
	while(count > 0){
		bzero(user, 256);

		/* 从终端获取用户名 */
		printf("User Name: ");
		fflush(stdout);
		read_input(user, 256);

		char *encrypt_str;
		int encrypt_len = encrypt_string(user, &encrypt_str);

		/* 发送用户名到服务器 */
		strcpy(cmd.code, "USER");
		//strcpy(cmd.arg, user);
		strncpy(cmd.arg, encrypt_str, encrypt_len);
		ftp_client_send_username_password(&cmd);

		free(encrypt_str);

		/* 等待应答码 331 */
		int wait;
		recv(sock_control, &wait, sizeof(wait), 0);

		/* 从终端获得密码 */
		fflush(stdout);
		pass = getpass("Password: ");
		//printf("ftp_client_login : input pass is : %s\n", pass);

		encrypt_len = encrypt_string(pass, &encrypt_str);

		/* 发送密码到服务器 */
		strcpy(cmd.code, "PASS");
		//strcpy(cmd.arg, pass);
		strncpy(cmd.arg, encrypt_str, encrypt_len);
		ftp_client_send_username_password(&cmd);

		free(encrypt_str);

		/* 等待响应 */
		int rc = read_reply();
		switch(rc){
			case 430:
				printf("ftp_client_login : Invalid username/password.\n");
				count--;
				if(count > 0){
					printf("ftp_client_login : Try Again, You can also try %d times\n", count);
				}
				continue;
			case 230:
				printf("ftp_client_login : Successful login.\n");
				count = -1;//登录成功，count置为 -1
				break;
			default:
				perror("ftp_client_login : error reading msg from server");
				exit(EXIT_FAILURE);
				break;
		}
	}
	if(count == 0){
		printf("ftp_client_login : Sorry, Please confirm the correctness of the aoccount and password.\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]){
	setenv("MALLOC_TRACE", "./bin/mem_trace_result", 1);
	mtrace();
	//	int sock_fd, rc, s;

	int sock_fd, rc;
	char buf[MAXSIZE];
	struct command cmd;
	//	struct addrinfo hints, *res, *rp;
	struct sockaddr_in server_addr;

	if(argc != 3){
		printf("Usage : %s <hostname> <port>\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	char *host = argv[1];//所要连接的服务器主机名
	char *port = argv[2];//所要链接到服务器程序端口号
#if 0
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	s = getaddrinfo(host, port, &hints, &res);
	if(s != 0){
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* 找到对应的服务器地址并连接 */
	for(rp = res; rp != NULL; rp = rp->ai_next){
		sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sock_control < 0){
			continue;
		}

		if(connect(sock_control, res->ai_addr, res->ai_addrlen) == 0){
			break;
		}else{
			perror("connecting stream socket");
			exit(EXIT_FAILURE);
		}
		close(sock_control);
	}

	freeaddrinfo(rp);
#endif
	sock_control = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_control < 0){
		perror("main : socket() error");
		exit(EXIT_FAILURE);
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(host);
	server_addr.sin_port = htons(atoi(port));

	socklen_t server_addr_len = sizeof(server_addr);
	if(connect(sock_control, (struct sockaddr *)&server_addr, server_addr_len) < 0){
		perror("main : connect() error");
		close(sock_control);
		exit(EXIT_FAILURE);
	}

	/* 连接成功，输出提示信息 */
	printf("main : Connected to %s\n", host);
	print_reply(read_reply());

	/* 获取用户的名字、密码 */
	ftp_client_login();

	while(1){//循环，直到用户输入 quit
		/* 得到用户输入的命令 */
		if(ftp_client_read_cmd(buf, sizeof(buf), &cmd) < 0){
			printf("main : Invalid command\n");
			continue;
		}

		/* 发送命令到服务器 */
		if(send(sock_control, buf, (int)strlen(buf), 0) < 0){
			close(sock_control);
			exit(EXIT_FAILURE);
		}

		rc = read_reply();//读取服务器响应（服务器是否可以支持该命令）

		/* 221--Service closing control connection. 控制连接关闭 */
		if(rc == 221){//退出命令
			print_reply(221);
			break;
		}

		/* 502--Command not implemented. 命令未被执行*/
		if(rc == 502){
			printf("main : %d Invalid command\n", rc);//非法的输入，输出错误信息
		}else{//命令合法
			sock_fd = ftp_client_open_conn(sock_control);
			if(sock_fd < 0){
				perror("main : ftp_client_open_conn() error");
				exit(EXIT_FAILURE);
			}

			if(strcmp(cmd.code, "LIST") == 0){
				ftp_client_list(sock_fd, sock_control);
			}else if(strcmp(cmd.code, "RETR") == 0){
				if(read_reply() == 550){/* 请求操作未被执行，文件不可用 */
					print_reply(550);
					close(sock_fd);
					continue;
				}

				ftp_client_get(sock_fd, sock_control, cmd.arg);
				print_reply(read_reply());
			}

			close(sock_fd);
		}
	}

	close(sock_control);
	muntrace();
	return 0;
}
