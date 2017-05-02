/*####################################################
# File Name: common.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 17:42:47
# Last Modified: 2017-05-03 01:43:58
####################################################*/
#include "common.h"

int encrypt_string(char *input_str, char **encrypt_str){
	AES_KEY aes;
	unsigned char key[AES_BLOCK_SIZE];//AES_BLOCK_SIZE = 16
	unsigned char iv[AES_BLOCK_SIZE];
	unsigned int len;// encrypt length (in multiple of AES_BLOCK_SIZE)
	unsigned int i;

	const unsigned char *temp_input_str = (const unsigned char *)input_str;
	unsigned char *temp_encrypt_str;

	len = 0;
	if((strlen(input_str) + 1) % AES_BLOCK_SIZE == 0){
		len = strlen(input_str) + 1;
	}else{
		len = ((strlen(input_str) + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	}

	//generate AES 128-bit key
	for(i = 0; i < AES_BLOCK_SIZE; ++i){
		key[i] = 32 + i;
	}

	for(i = 0; i < AES_BLOCK_SIZE; ++i){
		iv[i] = 0;
	}

	if(AES_set_encrypt_key(key, 128, &aes) < 0){
		fprintf(stderr, "Unable to set encryption key in AES\n");
		exit(EXIT_FAILURE);
	}

	//*encrypt_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	temp_encrypt_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	if (temp_encrypt_str == NULL) {
		fprintf(stderr, "Unable to allocate memory for encrypt_string\n");
		exit(-1);
	}
	*encrypt_str = (char *)temp_encrypt_str;

	AES_cbc_encrypt(temp_input_str, temp_encrypt_str, len, &aes, iv, AES_ENCRYPT);

	return len;
}

void decrypt_string(char *encrypt_str, char **decrypt_str, int len){
	unsigned char key[AES_BLOCK_SIZE];        // AES_BLOCK_SIZE = 16
	unsigned char iv[AES_BLOCK_SIZE];        // init vector
	AES_KEY aes;
	int i;

	const unsigned char *temp_encrypt_str = (const unsigned char *)encrypt_str;
	unsigned char *temp_decrypt_str;

	for (i = 0; i < AES_BLOCK_SIZE; ++i) {
		key[i] = 32 + i;
	}

	//*decrypt_str = (unsigned char*)calloc(len, sizeof(unsigned char));
	temp_decrypt_str = (unsigned char*)calloc(len, sizeof(unsigned char));
	if (temp_decrypt_str == NULL) {
		fprintf(stderr, "Unable to allocate memory for decrypt_string\n");
		exit(-1);
	}
	*decrypt_str = (char *)temp_decrypt_str;

	for (i = 0; i < AES_BLOCK_SIZE; ++i) {
		iv[i] = 0;
	}

	if (AES_set_decrypt_key(key, 128, &aes) < 0) {
		fprintf(stderr, "Unable to set decryption key in AES\n");
		exit(-1);
	}

	AES_cbc_encrypt(temp_encrypt_str, temp_decrypt_str, len, &aes, iv, AES_DECRYPT);
}

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

	ret_val = listen(sock_fd, BACK_LOG);
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
	socklen_t client_len = sizeof(client_addr);

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
	memset(&server_addr, 0, sizeof(server_addr));
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
	bzero(buf, size);

	if(fgets(buf, size, stdin) != NULL){
		index = strchr(buf, '\n');
		if(index != NULL){
			*index = '\0';// 出现换行符，则将该位置字符置'\0'（字符串结尾）
		}
	}
}
