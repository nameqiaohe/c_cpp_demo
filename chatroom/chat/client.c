#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <curses.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include <curses.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#define SERVPORT 2345     //�˿ں�
#define MAXDATASIZE 1024  //����ṹ������������ֽ�

typedef struct
{

  int flag;
  char name[10];
  //char sex;
  char password[20];
}REGMESSAGE;

typedef struct msgNode
{
	char attrstr[56];
	char msgstr[56];
	struct msgNode *next;
}msgNode,*msgList;

typedef struct dNode{
	int type;
	int src_client_fd;
	int des_client_fd;
	char buf[MAXDATASIZE];
}dataNode,*dataPtr;

typedef struct QNode
{
	dataNode data;
	struct QNode *next;
}QNode,*QueuePtr;

typedef struct
{
	QueuePtr front;//��ͷָ��
	QueuePtr rear;//��βָ��
}linkQueue;

/* ����ȫ�ֱ���*/
//�ͻ�����Ϣ
int sockfd;
int myfd;
char myname[10];
//��־
int insetflag = 0;//���Ͳ�������
int wrlockflag = 0;//����д��
//ʱ�����
time_t timep;
//���д���ָ��
WINDOW *inputWin,*inputMsgWin,*inputDesWin,*inputTypeWin,*inputCallWin; 
WINDOW *msgWin,*msgCntWin;
WINDOW *userListWin,*userListCntWin; 
msgList mlist;//��Ϣ����ָ��
linkQueue sendQueue,recvQueue;//���� ���Ͷ���

/* ��ʼ������ */
void InitQueue(linkQueue *Q)
{
	Q->front = (QueuePtr)malloc(sizeof(QNode));
	Q->rear = Q->front;
	Q->front->next = NULL;
	//Q.rear->next = NULL;
}
/* ���� �������� */
void EnQueue(linkQueue *Q,dataNode e)
{
	QueuePtr p;
	p = (QueuePtr)malloc(sizeof(QNode));
	p->data.type = e.type;
	p->data.src_client_fd = e.src_client_fd;
	p->data.des_client_fd = e.des_client_fd;
	memcpy(p->data.buf,e.buf,MAXDATASIZE);
	p->next = NULL;
	Q->rear->next = p;
	Q->rear = p;
}

/* ���� ���ݳ��� */
void DeQueue(linkQueue *Q,dataNode *e)
{
	QueuePtr p;
	p = Q->front->next;
	e->type = p->data.type;
	e->src_client_fd = p->data.src_client_fd;
	e->des_client_fd = p->data.des_client_fd;
	memcpy(e->buf,p->data.buf,MAXDATASIZE);
	Q->front->next = p->next;
	if(Q->rear == p)
		Q->rear=Q->front;
	free(p);
}

/* �ж϶����Ƿ�Ϊ�� */
int EmpQueue(linkQueue Q)
{
	if(Q.front == Q.rear)
		return 1;
	else
		return 0;
}

/* ��ʼ����Ϣ����,��ͷ���ѭ����������Ϊ8��� */
void msgList_init(void)//
{
	msgNode *p,*s;
	mlist = NULL;
	int i;
	for(i = 0; i < 8; i++)
	{
		p = (msgNode *)malloc(sizeof(msgNode));
		strcpy(p->attrstr,"");
		strcpy(p->msgstr,"");
		p->next = NULL;
		if(i == 0)
		{
			mlist = p;
			s = mlist;
		}
		else
		{
			s->next = p;
			s = s->next;
		}
	}
	s->next = mlist;
}

/* ������Ϣ����mlistָ�������յ�����Ϣ */
void msgList_updata(char* attr,char *msg)
{
	strcpy(mlist->attrstr,attr);
	strcpy(mlist->msgstr,msg);
	mlist = mlist->next;
}

/* ��ȡʱ�亯�� */
void gettime(char *str)
{
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	sprintf(str,"%02d:%02d:%02d",p->tm_hour+8,p->tm_min,p->tm_sec);
}
/* �޸���ʾ��Ϣ */
void setcall(int num)
{
	char callstr[58];
	wclear(inputCallWin);
	switch(num)
	{
		case 1:	//������Ϣ
			strcpy(callstr,"��ʾ����������Ϣ����Enter�����ͣ�Ctrl+C���ķ��Ͳ�����");
			break;
		case 2:	//������Ϣ ������Ϣ
			strcpy(callstr,"������ϢΪ��,����������!Ctrl+C���ķ��Ͳ���!");
			break;
		case 3:	//�����û���� 
			strcpy(callstr,"��ʾ��������Ҳ��û��б�����Ŀ���û����,Ⱥ�����Ϊ 0.");
			break;
		case 4:	//�����û���� ������Ϣ
			strcpy(callstr,"���󣺱�Ŵ���! ������Ҳ��û��б�,Ⱥ�����Ϊ 0 .");
			break;
		case 5:	//��������
			strcpy(callstr,"��ʾ�������뷢�����ͱ�� - ��Ϣ : 0  �ļ� : 1 ");
			break;
		case 6:	//�������� ������Ϣ
			strcpy(callstr,"�����������,�����뷢�����ͱ�� - ��Ϣ : 0  �ļ� : 1 ");
			break;		
		case 7:
			strcpy(callstr,"��ʾ����Enter������������ã�");
		default:
			break;
	}
	mvwaddstr(inputCallWin,0,0,callstr);
	wrefresh(inputCallWin);
}

/*���봰����ʾ��������ʾ��ˢ�´�������*/
void inputWin_display(void)
{
	wclear(inputWin);
	box(inputWin,0,0);//������
	mvwaddstr(inputWin,0,25,"  Input ");
	mvwaddstr(inputWin,1,2,"���Ͷ�����:[");
	mvwaddstr(inputWin,1,20,"]");
	mvwaddstr(inputWin,1,42,"����:[");
	mvwaddstr(inputWin,1,58,"]");	
	mvwaddstr(inputWin,2,2,"Content:");
	mvwaddstr(inputWin,3,2,"[");
	mvwaddstr(inputWin,3,58,"]");
	//�Ӵ�������
	wrefresh(inputWin);
}
/* ˢ����Ϣ��ʾ */
void msgCnt_display(void)
{
	msgNode *p = mlist;
	int i = 1;

	wclear(msgCntWin);
	printf("%s\n","abc:");
	for(i = 0; i <= 15; i = i + 2)
	{
		mvwprintw(msgCntWin,i,2,"%s",p->attrstr);
		mvwprintw(msgCntWin,i+1,2,"%s",p->msgstr);
		p = p->next;
	}
	wrefresh(msgCntWin);
}
/*��Ϣ������ʾ��������ʾ��ˢ�´�������*/
void msgWin_display(void)
{
	wclear(msgWin);
	box(msgWin,0,0);//������
	mvwaddstr(msgWin,0,25,"  Message ");
	wrefresh(msgWin);
	msgCnt_display();
}

/*�û��б�����ʾ��������ʾ��ˢ�´�������*/
void userListWin_display(void)
{
	wclear(userListWin);
	box(userListWin,0,0);//������
	mvwaddstr(userListWin,0,5,"  UserList ");
	//�Ӵ�������
	mvwprintw(userListWin,1,3,"���ͻ��˱��:%d",myfd);
	wrefresh(userListWin);
}
/*��Ļ��ʼ�������������������ڼ����Ӵ��ڲ����ƽ���*/
void screen_init(void)
{
	inputWin = newwin(6,60,18,0);//�������봰�ڼ��Ӵ��� 
	inputDesWin = subwin(inputWin,1,3,19,16); //Ŀ��
	inputTypeWin = subwin(inputWin,1,10,19,48);//����
	inputMsgWin = subwin(inputWin,1,53,21,3);//��Ϣ 
	inputCallWin = subwin(inputWin,1,56,22,2);//��ʾ 
	msgWin = newwin(18,60,0,0);//������Ϣ���ڼ��Ӵ��� 
	msgCntWin = subwin(msgWin,16,58,1,1);//���� 
	userListWin = newwin(24,20,0,60);//�û��б��ڼ��Ӵ��� 
	userListCntWin = subwin(userListWin,22,18,1,1);//���� 
	//��ʾ��������	
	msgWin_display();
	inputWin_display();
	userListWin_display();
}

/* �Զ����źŴ����������������л� */
void input_set(int sign_no)
{
	if(sign_no == SIGINT)
	{	
		insetflag = 1;
		setcall(7);//����������ʾ
	}
}

void client_quit(int sign_no)
{
	dataNode databuf;
	int sendbytes;
	if(sign_no == SIGQUIT)
	{
		databuf.type = 8;
		if((sendbytes = send(sockfd,&databuf,sizeof(dataNode),0)) == -1)
		{  
			perror("�����쳣");
			exit(1);
		}
		endwin();
		printf("�ͻ��������˳���\n");
		execl("/bin/clear","clear",NULL);
		exit(0);
	}
}
/* ���ݷ����߳� */
void send_thread(void)
{
	dataNode databuf;
	while(1)
	{
		if(EmpQueue(sendQueue) == 0)
		{
			DeQueue(&sendQueue,&databuf);
			if(send(sockfd,&databuf,sizeof(dataNode),0) == -1)
			{  
				perror("�����쳣");
				exit(1);
			}
		}
	}
}
/* ���ݽ����߳� */
void recv_thread(void)
{
	dataNode databuf;
	while(1)
	{
		if(recv(sockfd,&databuf,sizeof(dataNode),0) == -1)
		{
			perror("�����쳣");
			exit(1);
		}
		EnQueue(&recvQueue,databuf);
	}
}
/* ���ݴ����߳� */
void hand_thread(void)
{
	msgNode msgbuf;
	dataNode databuf;
	int i;
	while(1)
	{
		if(EmpQueue(recvQueue) == 0)//��Ϣ����
		{
			DeQueue(&recvQueue,&databuf);
			if(databuf.type == 0)
			{
				memcpy(&msgbuf,databuf.buf,sizeof(msgNode));
				msgList_updata(msgbuf.attrstr,msgbuf.msgstr);
				msgCnt_display();
				wrefresh(inputMsgWin);
			}
			else if(databuf.type == 1)//�ļ�����
			{
				//�ļ����մ���
			}
			else if(databuf.type == 2)//�û��б���
			{
				wclear(userListWin);
				userListWin_display();
				for(i = 0; i < databuf.src_client_fd; i++)
				{
					mvwaddstr(userListWin,3+i,3,(databuf.buf)+15*i);
				}
				wrefresh(userListWin);
				wrefresh(inputMsgWin);
			}
		}
	}
}
/* ��������߳� */
void inputcnl_thread(void)
{
	/* ��ʼ��������� */
	dataNode databuf;
	msgNode msgbuf;
	char timestr[10];
	int type = 0,desnum = 0;
	int num;
	int inerrflag = 0;//��������־
	insetflag == 0;
	while(1)
	{
		//����������ת
		if(insetflag == 1)
		{
			//Ŀ��������
			while(1)
			{
				if(inerrflag == 0)
					setcall(3);//������ʾ
				else
					setcall(4);//������ʾ
				wclear(inputDesWin);
				mvwscanw(inputDesWin,0,0,"%d",&num);
				wclear(inputDesWin);
				if(num >= 0 && num < 99)
				{
					mvwprintw(inputDesWin,0,0,"%3d",num);
					wrefresh(inputDesWin);
					inerrflag = 0;
					desnum = num;
					break;
				}
				else
				{
					inerrflag = 1;
				}			
			}			
			//���ͱ������
			while(1)
			{
				if(inerrflag == 0)
					setcall(5);//������ʾ
				else
					setcall(6);//������ʾ
				wclear(inputTypeWin);
				mvwscanw(inputTypeWin,0,0,"%d",&num);
				wclear(inputTypeWin);
				if(num == 0)
				{
					mvwaddstr(inputTypeWin,0,2,"��Ϣ");
					wrefresh(inputTypeWin);
					inerrflag = 0;
					type = 0;
					break;
				}
				else if(num == 1)
				{
					mvwaddstr(inputTypeWin,0,2,"�ļ�");
					wrefresh(inputTypeWin);
					inerrflag = 0;
					type = 1;
					break;
				}
				else
				{
					inerrflag = 1;
					continue;
				}
			}
			insetflag = 0;
		}
		//������ʾ����
		mvwprintw(inputDesWin,0,0,"%3d",desnum);
		if(type == 0)
			mvwaddstr(inputTypeWin,0,2,"��Ϣ");
		else if(type == 1)
			mvwaddstr(inputTypeWin,0,2,"�ļ�");	
		wrefresh(inputDesWin);
		wrefresh(inputTypeWin);
		//��Ϣ��������
		if(type == 0)
		{
			if(inerrflag == 0)
				setcall(1);//������Ϣ������ʾ
			else
				setcall(2);//������Ϣ������ʾ
			wclear(inputMsgWin);
			mvwgetstr(inputMsgWin,0,0,msgbuf.msgstr);//��ȡ��Ϣ�ַ���
			if(insetflag == 1) continue;
			if(strlen(msgbuf.msgstr) < 1)
			{
				inerrflag = 1;	//����ѭ��
				continue;
			}				
			else
			{
				inerrflag = 0;
				// ����Ϣװ��ṹ��
				databuf.type = type;
				databuf.src_client_fd = myfd;//////
				databuf.des_client_fd = desnum;/////
				gettime(timestr);//д�뷢�ͱ�ʾ�ַ���
				if(desnum == 0)
					sprintf(msgbuf.attrstr,"[Ⱥ]%-10s   %s",myname,timestr);
				else
					sprintf(msgbuf.attrstr,"[˽]%-10s   %s",myname,timestr);
				memcpy(databuf.buf,&msgbuf,sizeof(msgNode));//����Ϣ������������
				//msgList_updata(msgbuf.attrstr,msgbuf.msgstr);
				//msgCnt_display();
				EnQueue(&sendQueue,databuf);
			}
		}
		else if(type == 1)
		{
			//�����ļ�
			type = 0;
		}
	
	}
}

int main(int argc,char *argv[])
{
	pthread_t inputthid,sendthid,recvthid,handthid;
	int ret;
	int sendbytes,recvbytes,len,yesorno;
	struct hostent *host;
	REGMESSAGE useinfo;
	struct sockaddr_in serv_addr;
	dataNode databuf;

	//��ȡ��������
 if(argc < 2)
  {
  fprintf(stderr,"�������������ַ!\n");
  exit(1);
  }
	//if((host = gethostbyname("127.0.0.1")) == NULL)
	if((host = gethostbyname(argv[1])) == NULL)
	{
		perror("��ȡ������������");
		exit(1);
	}

	//����socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("����socketʧ��");
		exit(1);
	}
	
	//���� sockaddr_in �ṹ������ز���
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVPORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(serv_addr.sin_zero,8);

	//���ӷ�����
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("���Ӵ���");
		exit(1);
	}
	else
	{
		printf("���ӷ������ɹ�!\n");
	}

	//��ȡ�û���
	puts("���û�ѡ�� 0��¼  1ע��  ");
	scanf("%d",&useinfo.flag);
	puts("�������û���");
	scanf("%s",useinfo.name);
	puts("����������");
	scanf("%s",useinfo.password);
	
  strcpy(myname,useinfo.name);
	printf("�û���:%s\n",myname);

//---------------------------------------------
	if(send(sockfd,&useinfo,sizeof(useinfo), 0) == -1)
		{
			perror("�����쳣");
			exit(1);	
		}
	if((recvbytes = recv(sockfd,&yesorno,sizeof(int), 0)) == -1)
		{
			perror("recv()");
			exit(1);
		}
		
	if(useinfo.flag == 1)
		{
			if(yesorno == 0)
			{
				puts("ע��ɹ�");
				exit(0);	
			}	
		}
	else
		{
			if(yesorno == 0)
			{
				puts("�û��������벻��ȷ");
				exit(0);	
			}
		}


//---------------------------------------------

	//���ܷ���������Ŀͻ���fd
	if((recvbytes = recv(sockfd,&myfd,sizeof(int), 0)) == -1)
	{
		perror("recv()");
		exit(1);
	}
	printf("�ͻ��˱��: %d\n",myfd);
	//�����û���������
	if((sendbytes = send(sockfd,myname,10, 0)) == -1)
	{
		perror("send()");
		exit(1);
	}

	//getchar();
	//��ʼ����Ϣ����
	msgList_init();
	//��ʼ������
	InitQueue(&sendQueue);
	InitQueue(&recvQueue);
	//��ʼ��ͼ�ν���
	initscr();
	screen_init(); 
	//����SIGINT�ź��������� 
	signal(SIGINT,input_set);
	signal(SIGQUIT,client_quit);
	ret = pthread_create(&inputthid,NULL,(void *)inputcnl_thread,NULL);
	if(ret == -1)
	{
		perror("Create inputcnl_thread");
		exit(1);
	}
	ret = pthread_create(&sendthid, NULL, (void *)send_thread,NULL);
	if(ret == -1)
	{
		perror("Create send_thread");	
		exit(1);
	}
	ret = pthread_create(&recvthid, NULL, (void *)recv_thread,NULL);
	if(ret == -1)
	{
		perror("Create recv_thread");	
		exit(1);
	}
	ret = pthread_create(&handthid, NULL, (void *)hand_thread,NULL);
	if(ret == -1)
	{
		perror("Create recv_thread");	
		exit(1);
	}
	getchar();
	//�ȴ��߳̽���
	pthread_join(inputthid,NULL);
	pthread_join(sendthid,NULL);
	pthread_join(recvthid,NULL);
	//�ر�ͼ�ν���
	endwin();

	return 0;
}
