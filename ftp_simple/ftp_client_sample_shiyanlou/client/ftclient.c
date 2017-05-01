#include "ftclient.h"
	
int sock_control; 

/**
 * ���շ�������Ӧ
 * ���󷵻� -1����ȷ����״̬��
 */
int read_reply()
{
	int retcode = 0;
	if (recv(sock_control, &retcode, sizeof retcode, 0) < 0) 
	{
		perror("client: error reading message from server\n");
		return -1;
	}	
	return ntohl(retcode);
}

/**
 * ��ӡ��Ӧ��Ϣ
 */
void print_reply(int rc) 
{
	switch (rc)
	{
		case 220:
			printf("220 Welcome, server ready.\n");
			break;
		case 221:
			printf("221 Goodbye!\n");
			break;
		case 226:
			printf("226 Closing data connection. Requested file action successful.\n");
			break;
		case 550:
			printf("550 Requested action not taken. File unavailable.\n");
			break;
	}
}

/**
 * ���������е��ṹ��
 */ 
int ftclient_read_command(char* buf, int size, struct command *cstruct)
{
	memset(cstruct->code, 0, sizeof(cstruct->code));
	memset(cstruct->arg, 0, sizeof(cstruct->arg));
	
	printf("ftclient> ");	// ������ʾ��	
	fflush(stdout); 	
	read_input(buf, size); // �ȴ��û���������
	char *arg = NULL;
	arg = strtok (buf," ");
	arg = strtok (NULL, " ");

	if (arg != NULL)
		strncpy(cstruct->arg, arg, strlen(arg));

	if (strcmp(buf, "list") == 0) 
		strcpy(cstruct->code, "LIST");

	else if (strcmp(buf, "get") == 0)
		strcpy(cstruct->code, "RETR");

	else if (strcmp(buf, "quit") == 0) 
		strcpy(cstruct->code, "QUIT");
	
	else 
		return -1; // ���Ϸ�

	memset(buf, 0, 400);
	strcpy(buf, cstruct->code);  // �洢��� buf ��ʼ��

	/* ���������в�����׷�ӵ� buf */
	if (arg != NULL) 
	{
		strcat(buf, " ");
		strncat(buf, cstruct->arg, strlen(cstruct->arg));
	}
	return 0;
}

/**
 * ʵ�� get <filename> ������
 */
int ftclient_get(int data_sock, int sock_control, char* arg)
{
    char data[MAXSIZE];
    int size;
    FILE* fd = fopen(arg, "w"); // ������������Ϊ arg ���ļ�

	/* �����������������ݣ��ļ����ݣ�д�뱾�ؽ������ļ� */
    while ((size = recv(data_sock, data, MAXSIZE, 0)) > 0) 
		fwrite(data, 1, size, fd); 

    if (size < 0) 
		perror("error\n");

    fclose(fd);
    return 0;
}

/**
 * ����������
 */
int ftclient_open_conn(int sock_con)
{
	int sock_listen = socket_create(CLIENT_PORT_ID);

	/* �ڿ��������Ϸ���һ�� ACK ȷ�� */
	int ack = 1;
	if ((send(sock_con, (char*) &ack, sizeof(ack), 0)) < 0) 
	{
		printf("client: ack write error :%d\n", errno);
		exit(1);
	}		

	int sock_conn = socket_accept(sock_listen);
	close(sock_listen);
	return sock_conn;
}

/** 
 * ʵ�� list ����
 */
int ftclient_list(int sock_data, int sock_con)
{
	size_t num_recvd;			
	char buf[MAXSIZE];			
	int tmp = 0;

	/* �ȴ���������������Ϣ */ 
	if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) 
	{
		perror("client: error reading message from server\n");
		return -1;
	}
	
	memset(buf, 0, sizeof(buf));

	/* ���շ��������������� */
	while ((num_recvd = recv(sock_data, buf, MAXSIZE, 0)) > 0) 
	{
		printf("%s", buf);
		memset(buf, 0, sizeof(buf));
	}
	
	if (num_recvd < 0) 
		perror("error");
	
	/* �ȴ���������ɵ���Ϣ */ 
	if (recv(sock_con, &tmp, sizeof tmp, 0) < 0) 
	{
		perror("client: error reading message from server\n");
		return -1;
	}
	return 0;
}

/**
 * ���뺬������(code)�Ͳ���(arg)�� command(cmd) �ṹ
 * ���� code + arg,���Ž�һ���ַ�����Ȼ���͸�������
 */
int ftclient_send_cmd(struct command *cmd)
{
	char buffer[MAXSIZE];
	int rc;

	sprintf(buffer, "%s %s", cmd->code, cmd->arg);
	
	/* ���������ַ����������� */ 
	rc = send(sock_control, buffer, (int)strlen(buffer), 0);	
	if (rc < 0) 
	{
		perror("Error sending command to server");
		return -1;
	}
	
	return 0;
}

/**
 * ��ȡ��¼��Ϣ
 * ���͵���������֤
 */
void ftclient_login()
{
	struct command cmd;
	char user[256];
	memset(user, 0, 256);

	/* ��ȡ�û��� */ 
	printf("Name: ");	
	fflush(stdout); 		
	read_input(user, 256);

	/* �����û����������� */ 
	strcpy(cmd.code, "USER");
	strcpy(cmd.arg, user);
	ftclient_send_cmd(&cmd);
	
	/* �ȴ�Ӧ���� 331 */
	int wait;
	recv(sock_control, &wait, sizeof wait, 0);

	/* ������� */
	fflush(stdout);	
	char *pass = getpass("Password: ");	

	/* �������뵽������ */ 
	strcpy(cmd.code, "PASS");
	strcpy(cmd.arg, pass);
	ftclient_send_cmd(&cmd);
	
	/* �ȴ���Ӧ */ 
	int retcode = read_reply();
	switch (retcode) 
	{
		case 430:
			printf("Invalid username/password.\n");
			exit(0);
		case 230:
			printf("Successful login.\n");
			break;
		default:
			perror("error reading message from server");
			exit(1);		
			break;
	}
}

/* ��������� */
int main(int argc, char* argv[]) 
{		
	int data_sock, retcode, s;
	char buffer[MAXSIZE];
	struct command cmd;	
	struct addrinfo hints, *res, *rp;

	/* �����в����Ϸ��Լ�� */
	if (argc != 3)
	{
		printf("usage: ./ftclient hostname port\n");
		exit(0);
	}

	char *host = argv[1]; //��Ҫ���ӵķ�����������
	char *port = argv[2]; //��Ҫ���ӵ�����������˿ں�

	/* ��úͷ�������ƥ��ĵ�ַ */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	s = getaddrinfo(host, port, &hints, &res);
	if (s != 0) 
	{
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(1);
	}
	
	/* �ҵ���Ӧ�ķ�������ַ������ */ 
	for (rp = res; rp != NULL; rp = rp->ai_next) 
	{
		sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); // ���������׽���

		if (sock_control < 0)
			continue;

		if(connect(sock_control, res->ai_addr, res->ai_addrlen)==0)   // �ͷ���������
			break;
		
		else 
		{
			perror("connecting stream socket");
			exit(1);
		}
		close(sock_control);
	}
	freeaddrinfo(rp);


	/* ���ӳɹ�����ӡ��Ϣ */
	printf("Connected to %s.\n", host);
	print_reply(read_reply()); 
	

	/* ��ȡ�û������ֺ����� */
	ftclient_login();

	while (1) 
	{ // ѭ����ֱ���û����� quit

		/* �õ��û���������� */ 
		if ( ftclient_read_command(buffer, sizeof buffer, &cmd) < 0)
		{
			printf("Invalid command\n");
			continue;	// ��������ѭ����������һ������
		}

		/* ������������� */ 
		if (send(sock_control, buffer, (int)strlen(buffer), 0) < 0 )
		{
			close(sock_control);
			exit(1);
		}

		retcode = read_reply();	//��ȡ��������Ӧ���������Ƿ����֧�ָ������

		if (retcode == 221)  // �˳�����
		{
			print_reply(221);		
			break;
		}
		
		if (retcode == 502) 
			printf("%d Invalid command.\n", retcode);// ���Ϸ������룬��ʾ������Ϣ

		else 
		{			
			// ����Ϸ� (RC = 200),��������
		
			/* ���������� */
			if ((data_sock = ftclient_open_conn(sock_control)) < 0) 
			{
				perror("Error opening socket for data connection");
				exit(1);
			}			
			
			/* ִ������ */
			if (strcmp(cmd.code, "LIST") == 0) 
				ftclient_list(data_sock, sock_control);
			
			else if (strcmp(cmd.code, "RETR") == 0) 
			{
				if (read_reply() == 550) // �ȴ��ظ�
				{
					print_reply(550);		
					close(data_sock);
					continue; 
				}
				ftclient_get(data_sock, sock_control, cmd.arg);
				print_reply(read_reply()); 
			}
			close(data_sock);
		}

	} // ѭ���õ�������û�����

	close(sock_control); // �ر��׽��ֿ�������
    return 0;  
}
