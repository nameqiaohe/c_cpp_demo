/*####################################################
# File Name: ftp_serve.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 16:32:29
# Last Modified: 2017-05-03 01:10:45
####################################################*/
#include "ftp_serve.h"

int main(int argc, char *argv[]){
	int sock_listen, sock_control, port, pid;

	if(argc != 2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	//将命令行传进来的服务器端口号（字符串）转换为整数
	port = atoi(argv[1]);

	//创建监听套接字
	sock_listen = socket_create(port);
	if(sock_listen < 0){
		perror("main : socket_create() error");
		exit(EXIT_FAILURE);
	}

	//循环接受不同的客户机请求
	while(1){
		sock_control = socket_accept(sock_listen);
		if(sock_control < 0){
			perror("main : socket_accept() error");
			break;
		}

		pid = fork();
		if(pid < 0){
			perror("main : fork() error");
		}else if(pid == 0){
			close(sock_listen);//子进程关闭父进程的监听套接字
			ftp_serve_process(sock_control);//子进程调用 ftserve_process 函数与客户端交互
			close(sock_control);//用户请求处理完毕，关闭该套接字
			exit(EXIT_SUCCESS);
		}

		close(sock_control);//父进程关闭子进程的控制套接字
	}

	close(sock_listen);

	return 0;
}

/* 通过数据套接字发送特定的文件，控制信息交互通过控制套接字
 * 处理无效的或者不存在的文件名*/
void ftp_serve_retr(int sock_control, int sock_fd, char *filename){
	FILE *fp = NULL;
	char data[MAXSIZE];
	size_t num_read;

	fp = fopen(filename, "r");
	//if(fp == NULL)
	if(!fp){
		send_response(sock_control, 550);//550--Requested action not taken. File unavailable (e.g., file not found, no access).
	}else{
		send_response(sock_control, 150);//150--File status okay; about to open data connection.
		do{
			num_read = fread(data, 1, MAXSIZE, fp);//读文件内容
			if(num_read < 0){
				printf("ftp_serve_retr : error in fread()\n");
				//读取失败 是否需要退出？？？
			}
			
			if(send(sock_fd, data, num_read, 0) < 0){
				perror("ftp_serve_retr : error sending file");
				//发送失败 是否需要退出？？？
			}
		}while(num_read > 0);

		send_response(sock_control, 226);//226--Closing data connection. Requested file action successful (for example, file transfer or file abort).
		fclose(fp);
	}
}

/* 响应请求：发送当前所在目录的目录项列表，关闭数据连接
 * 返回值：错误返回 -1，正确返回 0 */
int ftp_serve_list(int sock_fd, int sock_control){
	char data[MAXSIZE];
	size_t num_read;
	FILE *fp;

	//利用系统调用函数 system 执行命令，并重定向到 tmp.txt 文件
	int ret_val = system("ls -l | tail -n+2 > tmp.txt");
	if(ret_val < 0){
		exit(EXIT_FAILURE);
	}

	fp = fopen("tmp.txt", "r");
	if(!fp){
		exit(EXIT_FAILURE);
	}

	fseek(fp, SEEK_SET, 0);//文件指针 定位到 文件开始处

	send_response(sock_control, 1);

	memset(data, 0, MAXSIZE);

	while((num_read = fread(data, 1, MAXSIZE, fp)) > 0){
		ret_val = send(sock_fd, data, num_read, 0);
		if(ret_val < 0){
			perror("ftp_serve_list : error sending data");
		}

		memset(data, 0, MAXSIZE);
	}
	fclose(fp);

	//发送应答码 226（关闭数据连接，请求的文件操作成功）
	send_response(sock_control, 226);

	return 0;
}

/* 创建到客户机的一条数据连接
 * 返回值：成功返回数据连接的套接字
 *			失败返回 -1*/
int ftp_serve_start_conn(int sock_control){
	char buf[1024];
	int wait, sock_fd;

	int ret_val = recv(sock_control, &wait, sizeof(wait), 0);
	if(ret_val < 0){
		perror("ftp_serve_start_conn : error while waiting");
		return -1;
	}

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	getpeername(sock_control, (struct sockaddr *)&client_addr, &client_len);//通过控制连接的套接字 获取 客户端的 IP地址
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));//将网络字节序二进制值转换成点分十进制串

	sock_fd = socket_connect(CLIENT_PORT, buf);
	if(sock_fd < 0){
		perror("ftp_serve_start_conn : error socket_connect()");
		return -1;
	}

	return sock_fd;
}

/* 用户资格认证
 * 返回值：认证成功返回 1，否则返回 0*/
int ftp_serve_check_user(char *user, char *pass){
	char username[MAXSIZE];
	char passwd[MAXSIZE];
	char *token;
	char buf[MAXSIZE];
	char *line = NULL;
	
	size_t num_read;
	size_t len = 0;

	FILE *fp ;
	int auth = 0;

	//打开认证文件（记录用户名和密码）
	fp = fopen(".auth", "r");
	if(fp == NULL){
		perror("ftp_serve_check_user : .auth file not found");
		exit(EXIT_FAILURE);
	}

	/* 读取".auth" 文件中的用户名和密码，验证用户身份的合法性 */
	while((num_read = getline(&line, &len, fp)) != -1){// getline() 具体用法参看 man getline
		memset(buf, 0, MAXSIZE);	
		strcpy(buf, line);

		token = strtok(buf, " ");
		strcpy(username, token);

		if(token != NULL){
			token = strtok(NULL, " ");
			strcpy(passwd, token);
		}

		trim_str(passwd, (int)strlen(passwd));/* 去除字符串中的空格和换行符 */

		if((strncmp(user, username, strlen(user)) == 0) && strncmp(pass, passwd, strlen(pass)) == 0){
			auth = 1;//匹配成功，标志变量 auth = 1，并返回
			break;
		}
	}
	free(line);
	fclose(fp);

	return auth;
}

int ftp_serve_login(int sock_control){
	char buf[MAXSIZE];
	char username[MAXSIZE];
	char temp_username[MAXSIZE];
	char passwd[MAXSIZE];
	char temp_passwd[MAXSIZE];
	bzero(buf, MAXSIZE);
	bzero(username, MAXSIZE);
	bzero(passwd, MAXSIZE);
	bzero(temp_username, MAXSIZE);
	bzero(temp_passwd, MAXSIZE);

	int ret_val = recv_data(sock_control, buf, MAXSIZE);
	if(ret_val == -1){
		perror("ftp_serve_login : recv_data() error");
		exit(EXIT_FAILURE);
	}

	char *decrypt_str = NULL;
	decrypt_string(buf, &decrypt_str, ret_val);
	strcpy(username, decrypt_str);
	//printf("decrypt username : %s\n", username);

	send_response(sock_control, 331);//User name okay, need password.

	bzero(buf, MAXSIZE);
	ret_val = recv_data(sock_control, buf, MAXSIZE);
	if(ret_val == -1){
		perror("ftp_serve_login : recv_data() error");
		exit(EXIT_FAILURE);
	}

	decrypt_string(buf, &decrypt_str, ret_val);
	strcpy(passwd, decrypt_str);
	//printf("decrypt passwd : %s\n", passwd);

	//// 用户名和密码验证
	ret_val = ftp_serve_check_user(username, passwd);

	return ret_val;
}

/* 接收客户端的命令并响应，返回响应码 */
int ftp_serve_recv_cmd(int sock_control, char *cmd, char *arg){
	int rc = 200;
	char buf[MAXSIZE];

	bzero(buf, MAXSIZE);
	bzero(cmd, 5);
	bzero(arg, MAXSIZE);

	/* 接受客户端的命令 */
	int ret_val = recv_data(sock_control, buf, sizeof(buf));
	if(ret_val == -1){
		perror("ftp_serve_recv_cmd : recv_data() error");
		return -1;
	}

	/* 解析出用户的命令和参数 */
	strncpy(cmd, buf, 4);
	char *tmp = buf + 5;
	strcpy(arg, tmp);

	if(strcmp(cmd, "QUIT") == 0){
		rc = 221;//控制连接关闭
	}else if((strcmp(cmd, "USER") == 0) || strcmp(cmd, "PASS") == 0 || strcmp(cmd, "LIST") == 0 || strcmp(cmd, "RETR") == 0){
		rc = 200;//命令执行正常结束.
	}else{
		rc = 502;//命令未被执行
	}

	send_response(sock_control, rc);

	return rc;
}

void ftp_serve_process(int sock_control){
	int sock_fd;
	char cmd[5];
	char arg[MAXSIZE];

	send_response(sock_control, 220);// 发送欢迎应答码，新连接的用户的服务已就绪

	//用户认证
	int count = LOGIN_COUNT;
	while(count >= 0){
		if(ftp_serve_login(sock_control) == 1){
			send_response(sock_control, 230);//认证成功
			break;
		}else{
			send_response(sock_control, 430);//认证失败
			//exit(EXIT_SUCCESS);
			count--;
		}
	}

	//while循环中处理用户请求
	while(1){
		int rc = ftp_serve_recv_cmd(sock_control, cmd, arg);

		if(rc == 200){
			sock_fd = ftp_serve_start_conn(sock_control);
			if(sock_fd < 0){
				perror("ftp_serve_process : ftp_serve_start_conn() error");
				close(sock_control);
				exit(EXIT_FAILURE);
			}

			if(strcmp(cmd, "LIST") == 0){
				ftp_serve_list(sock_fd, sock_control);
			}else if(strcmp(cmd, "RETR") == 0){
				ftp_serve_retr(sock_control, sock_fd, arg);
			}

			close(sock_fd);
		}
	}
}
