#include "ftserve.h"

/* ��������� */
int main(int argc, char *argv[])
{	
	int sock_listen, sock_control, port, pid;

	/* �����кϷ��Լ�� */
	if (argc != 2)
	{
		printf("usage: ./ftserve port\n");
		exit(0);
	}

	/* �������д������ķ������˿ںţ��ַ�����ת��Ϊ���� */
	port = atoi(argv[1]);

	/* ���������׽��� */
	if ((sock_listen = socket_create(port)) < 0 )
	{
		perror("Error creating socket");
		exit(1);
	}		
	
	/* ѭ�����ܲ�ͬ�Ŀͻ������� */
	while(1) 
	{	
		/* �����׽��ֽ����������󣬵õ������׽��֣����ڴ��ݿ�����Ϣ */
		if ((sock_control = socket_accept(sock_listen))	< 0 )
			break;			
		
		/* �����ӽ��̴����û����� */
		if ((pid = fork()) < 0) 
			perror("Error forking child process");
	
		/* �ӽ��̵��� ftserve_process ������ͻ��˽��� */
		else if (pid == 0)
		{ 
			close(sock_listen);  // �ӽ��̹رո����̵ļ����׽���
			ftserve_process(sock_control);		
			close(sock_control); //�û���������ϣ��رո��׽���
			exit(0);
		}
			
		close(sock_control); // �����̹ر��ӽ��̵Ŀ����׽���
	}

	close(sock_listen);	

	return 0;
}

/**
 * ͨ�������׽��ַ����ض����ļ�
 * ������Ϣ����ͨ�������׽���
 * ������Ч�Ļ��߲����ڵ��ļ���
 */
void ftserve_retr(int sock_control, int sock_data, char* filename)
{	
	FILE* fd = NULL;
	char data[MAXSIZE];
	size_t num_read;									
	fd = fopen(filename, "r"); // ���ļ�

	if (!fd)
		send_response(sock_control, 550); // ���ʹ����� (550 Requested action not taken)
	
	else
	{	
		send_response(sock_control, 150); // ���� okay (150 File status okay)
		do 
		{
			num_read = fread(data, 1, MAXSIZE, fd); // ���ļ�����
			if (num_read < 0) 
				printf("error in fread()\n");

			if (send(sock_data, data, num_read, 0) < 0) // �������ݣ��ļ����ݣ�
				perror("error sending file\n");

		}
		while (num_read > 0);													
			
		send_response(sock_control, 226); // ������Ϣ��226: closing conn, file transfer successful

		fclose(fd);
	}
}

/**
 * ��Ӧ���󣺷��͵�ǰ����Ŀ¼��Ŀ¼���б�
 * �ر���������
 * ���󷵻� -1����ȷ���� 0
 */
int ftserve_list(int sock_data, int sock_control)
{
	char data[MAXSIZE];
	size_t num_read;									
	FILE* fd;

	int rs = system("ls -l | tail -n+2 > tmp.txt"); //����ϵͳ���ú��� system ִ��������ض��� tmp.txt �ļ�
	if ( rs < 0)
	{
		exit(1);
	}
	
	fd = fopen("tmp.txt", "r");	
	if (!fd) 
		exit(1); 
	
	/* ��λ���ļ��Ŀ�ʼ�� */
	fseek(fd, SEEK_SET, 0);

	send_response(sock_control, 1); 

	memset(data, 0, MAXSIZE);

	/* ͨ���������ӣ�����tmp.txt �ļ������� */
	while ((num_read = fread(data, 1, MAXSIZE, fd)) > 0) 
	{
		if (send(sock_data, data, num_read, 0) < 0) 
			perror("err");
	
		memset(data, 0, MAXSIZE);
	}

	fclose(fd);

	send_response(sock_control, 226);	// ����Ӧ���� 226���ر��������ӣ�������ļ������ɹ���

	return 0;	
}

/**
 * �������ͻ�����һ����������
 * �ɹ������������ӵ��׽���
 * ʧ�ܷ��� -1
 */
int ftserve_start_data_conn(int sock_control)
{
	char buf[1024];	
	int wait, sock_data;

	if (recv(sock_control, &wait, sizeof wait, 0) < 0 ) 
	{
		perror("Error while waiting");
		return -1;
	}

	
	struct sockaddr_in client_addr;
	socklen_t len = sizeof client_addr;
	getpeername(sock_control, (struct sockaddr*)&client_addr, &len); // ���������׽��ֹ������ⲿ��ַ���ͻ��˵�ַ��
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	/* �������ͻ������������� */
	if ((sock_data = socket_connect(CLIENT_PORT_ID, buf)) < 0)
		return -1;

	return sock_data;		
}

/**
 * �û��ʸ���֤
 * ��֤�ɹ����� 1�����򷵻� 0 
 */
int ftserve_check_user(char*user, char*pass)
{
	char username[MAXSIZE];
	char password[MAXSIZE];
	char *pch;
	char buf[MAXSIZE];
	char *line = NULL;
	size_t num_read;									
	size_t len = 0;
	FILE* fd;
	int auth = 0;
	
	fd = fopen(".auth", "r"); //����֤�ļ�����¼�û��������룩
	if (fd == NULL) 
	{
		perror("file not found");
		exit(1);
	}	

	/* ��ȡ".auth" �ļ��е��û��������룬��֤�û���ݵĺϷ��� */
	while ((num_read = getline(&line, &len, fd)) != -1) 
	{
		memset(buf, 0, MAXSIZE);
		strcpy(buf, line);
		
		pch = strtok (buf," ");
		strcpy(username, pch);

		if (pch != NULL)
		{
			pch = strtok (NULL, " ");
			strcpy(password, pch);
		}

		/* ȥ���ַ����еĿո�ͻ��з� */
		trimstr(password, (int)strlen(password));

		if ((strcmp(user,username)==0) && (strcmp(pass,password)==0)) 
		{
			auth = 1; // ƥ��ɹ�����־���� auth = 1��������
			break;
		}		
	}
	free(line);	
	fclose(fd);	
	return auth;
}

/* �û���¼*/
int ftserve_login(int sock_control)
{	
	char buf[MAXSIZE];
	char user[MAXSIZE];
	char pass[MAXSIZE];	
	memset(user, 0, MAXSIZE);
	memset(pass, 0, MAXSIZE);
	memset(buf, 0, MAXSIZE);
	
	/* ��ÿͻ��˴������û��� */
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) 
	{
		perror("recv error\n"); 
		exit(1);
	}	

	int i = 5;
	int n = 0;
	while (buf[i] != 0) //buf[0-4]="USER"
		user[n++] = buf[i++];
	
	/* �û�����ȷ��֪ͨ�û��������� */
	send_response(sock_control, 331);					
	
	/* ��ÿͻ��˴��������� */
	memset(buf, 0, MAXSIZE);
	if ( (recv_data(sock_control, buf, sizeof(buf)) ) == -1) 
	{
		perror("recv error\n"); 
		exit(1);
	}
	
	i = 5;
	n = 0;
	while (buf[i] != 0) // buf[0 - 4] = "PASS"
		pass[n++] = buf[i++];
	
	return (ftserve_check_user(user, pass)); // �û�����������֤��������
}

/* ���տͻ��˵������Ӧ��������Ӧ�� */
int ftserve_recv_cmd(int sock_control, char*cmd, char*arg)
{	
	int rc = 200;
	char buffer[MAXSIZE];
	
	memset(buffer, 0, MAXSIZE);
	memset(cmd, 0, 5);
	memset(arg, 0, MAXSIZE);
		
	/* ���ܿͻ��˵����� */
	if ((recv_data(sock_control, buffer, sizeof(buffer)) ) == -1) 
	{
		perror("recv error\n"); 
		return -1;
	}
	
	/* �������û�������Ͳ��� */
	strncpy(cmd, buffer, 4);
	char *tmp = buffer + 5;
	strcpy(arg, tmp);
	
	if (strcmp(cmd, "QUIT")==0) 
		rc = 221;

	else if ((strcmp(cmd, "USER") == 0) || (strcmp(cmd, "PASS") == 0) || (strcmp(cmd, "LIST") == 0) || (strcmp(cmd, "RETR") == 0))
		rc = 200;

	else 
		rc = 502; // ��Ч������

	send_response(sock_control, rc);	
	return rc;
}

/* ����ͻ������� */
void ftserve_process(int sock_control)
{
	int sock_data;
	char cmd[5];
	char arg[MAXSIZE];

	send_response(sock_control, 220); // ���ͻ�ӭӦ����

	/* �û���֤ */
	if (ftserve_login(sock_control) == 1)  // ��֤�ɹ�
		send_response(sock_control, 230);
	else 
	{
		send_response(sock_control, 430);	// ��֤ʧ��
		exit(0);
	}	
	
	/* �����û������� */
	while (1) 
	{
		/* �������������,�������Ͳ��� */
		int rc = ftserve_recv_cmd(sock_control, cmd, arg);
		
		if ((rc < 0) || (rc == 221))  // �û��������� "QUIT"
			break;
		
		if (rc == 200 ) 
		{
			/* �����Ϳͻ��˵��������� */
			if ((sock_data = ftserve_start_data_conn(sock_control)) < 0) 
			{
				close(sock_control);
				exit(1); 
			}

			/* ִ��ָ�� */
			if (strcmp(cmd, "LIST")==0) 
				ftserve_list(sock_data, sock_control);
			
			else if (strcmp(cmd, "RETR")==0) 
				ftserve_retr(sock_control, sock_data, arg);

			close(sock_data);// �ر�����
		} 
	}
}


