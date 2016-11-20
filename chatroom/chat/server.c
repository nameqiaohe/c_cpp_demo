#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>

#define  SERVPORT         2345  
#define  MAXCLIENT        10   
#define  MAXDATASIZE      1024  

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

typedef struct UserNode
{  
	int fd; 
	char name[10];
	struct UserNode *next;  
}UserNode, *UserList; 


/* ȫ�ֱ��� */
UserList  ulist;//�û�����ָ��

/* --�û���������-- */
/* �û������ʼ�� */
void userlist_init(UserList *L) 
{
	*L=(UserList)malloc(sizeof(UserNode)); 
	(*L)->next=NULL;      
} 

/* �û������󳤶� */
int userlist_length(UserList L)
{
	int lenth=0;
	UserList  p;
	p = L->next;
	if(p == NULL)
		return 0;
	else
	{
		while(p)
		{
			p=p->next;
			lenth++;
		}
		return lenth;
	}
}

/* �û���������û� */
void userlist_add(UserList *L,int userfd,char username[10])
{
	UserList s;
	s = (UserList) malloc ( sizeof (UserNode));
	s->fd = userfd;
	strcpy(s->name,username);
	s->next = (*L)->next;
	(*L)->next = s;
}

/* �û�����ɾ���û� */
int userlist_del(UserList *L,int userfd)
{
	UserList p,q; 
	p = *L;
	while ( p->next != NULL) 
	{ 
		if(p->next->fd == userfd)
		{   
			q = p->next;
			p->next = p->next->next; 
			userfd = p->fd;    
			free(q);
			return 0;
		}
		else
			p = p->next;
	}
	return 1; 
}

/* �û���������û� */
int userlist_search(UserList L,int userfd) 
{
	 UserList p;
	 p = L->next;
	 while(p != NULL)
	 {  
	   if(p->fd == userfd)
		   return p->fd;
	   else
		   p = p->next;
	 }
	 return  -1;
}

/* ���û�����������ݽ�� */
void userlist_tobuf(UserList L,dataNode *databuf)
{
	UserList p = L->next;
	char *local = databuf->buf;
	while(p)
	{
		sprintf(local,"[%2d]%-10s",p->fd,p->name);
		local +=15;
		p = p->next;
	}
	databuf->type = 2;
	databuf->src_client_fd = userlist_length(L);//����û�������
}


/*�ͻ����շ��߳�*/
void pthread(void *clientsocket)
{
	int recvbytes, sendbytes;
	int client_fd = *((int *)clientsocket);
	int i,usercount;
	int len,temp;///
	UserList p; 
 	dataNode databuf;
	msgNode msgbuf;
	while(1)
	{
		if((recvbytes = recv(client_fd, &databuf, sizeof(dataNode), 0)) == -1)
		{
			perror("�����쳣");
			exit(1);
		}
		if(databuf.type == 0)//��Ϣ
		{
			if(databuf.des_client_fd == 0)//����
			{
				usercount = userlist_length(ulist);
				p = ulist; 
				for(i = 0;i < usercount; i++)
				{  
					if((sendbytes = send(p->next->fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("�����쳣");
						exit(1);	
					}
					p = p->next;
				}
			}
			else//˽��
			{
				
				if(userlist_search(ulist,databuf.des_client_fd) == -1)//Ŀ��ͻ���������
				{
					databuf.type = 0;
					sprintf(msgbuf.attrstr,"[ϵ] ���Ϊ%d�Ŀͻ��������ڻ������ߣ�",databuf.des_client_fd);
					sprintf(msgbuf.msgstr,"     ������Ҳ��û��б�ѡ����ȷ��ţ�");
					memcpy(databuf.buf,&msgbuf,sizeof(msgNode));
					if((sendbytes = send(client_fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("�����쳣");	  
						exit(1);
					}
				}
				else//���͵�Ŀ��ͻ����Լ�Դ�ͻ���
				{
					if((sendbytes = send(databuf.des_client_fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("�����쳣");	  
						exit(1);
					}
					//send to src_client
					if((sendbytes = send(databuf.src_client_fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("�����쳣");	  
						exit(1);
					}
				}
			}
		} 
		else if(databuf.type == 1)//�ļ�
		{
		}
		else if(databuf.type == 8)//�˳�
		{
			userlist_del(&ulist,client_fd);
			userlist_tobuf(ulist,&databuf);
			p = ulist; 
			usercount = userlist_length(ulist);
			printf("�ͻ������%d�˳��������û��б���ǰ�����û�%d��\n",client_fd,usercount);
			if(usercount == 0)
			{
				pthread_exit(0);
			}
			else
			{
				for(i = 0; i < usercount; i++)
				{
					if(send(p->next->fd,&databuf,sizeof(msgNode), 0) == -1)
					{
						perror("�����쳣");
						exit(1);	
					}
					p = p->next; 
				}
				pthread_exit(0);
			}
		}
	}
}
//-------------------------
//ע���û��������뵽�ļ���
static void RregisterUser(REGMESSAGE *Userdata)
{
   int fd;
   if((fd = open("/registerlist.txt",O_CREAT|O_WRONLY|O_APPEND,0666))<0)
   {
        perror("open");
        exit(1);
   }
   else
    write(fd,Userdata,sizeof(REGMESSAGE));
    close(fd);
}

//�ж��û��Ƿ�ע��
int IsHaveRegistered(REGMESSAGE Comparedata)
{

 int fd;
 REGMESSAGE data;
 if((fd = open("/registerlist.txt",O_CREAT|O_RDONLY,0666))<0)
 {
        perror("open");
        exit(1);
 }
 while(read(fd,&data,sizeof(REGMESSAGE)))
 {
  	if((strcmp(Comparedata.name,data.name)==0 )&&(strcmp(Comparedata.password,data.password)==0))return 1;
 }
 		return 0;
}


//-------------------------

int main(void)
{
	struct sockaddr_in server_sockaddr, client_sockaddr;
	int sin_size,sendbytes,yesorno,recvbytes;
	int sockfd, client_fd;
	pthread_t pthreadid;
	dataNode databuf;
	REGMESSAGE useinfo;
	UserNode *p;
	char username[10];
	int i,usercount;
	//��ʼ���û�����
	userlist_init(&ulist);

	//����socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("����socketʧ��");
		exit(1);
	}
	printf("����socket�ɹ���socket�˿ں�Ϊ  %d��\n", sockfd);
  
	//���� sockaddr_in �ṹ������ز���
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero),8);
 
	//��ip�Ͷ˿�
	if(bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))==-1)
	{
		perror("��IPʧ��");
		exit(1);
	}
	printf("��IP�ɹ�! \n");
  
	//��������
	if(listen(sockfd,MAXCLIENT)==-1)
	{
		perror("��������ʧ��");
		exit(1);
	}
	printf("���ڼ�������......\n");

	while(1)
	{
		//�ȴ��ͻ�������
		if((client_fd=accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size))==-1)
		{  
			perror("�ȴ�����ʧ��");
			exit(1);
		}
		
		
		//------------------------------------------
	  if((recvbytes = recv(client_fd, &useinfo, sizeof(REGMESSAGE), 0)) == -1)
		{
			perror("�����쳣");
			exit(1);
		}
		
		//printf("**********\n");
		if(useinfo.flag == 1)  //ע��      
		{
			RregisterUser(&useinfo);
			yesorno = 0;
			if(send(client_fd,&yesorno,sizeof(int), 0) == -1)
			{
				perror("�����쳣");
				exit(1);	
			}		
		}
		
		else
		{
			if(!IsHaveRegistered(useinfo))
			{
			yesorno = 0;
			if(send(client_fd,&yesorno,sizeof(int), 0) == -1)
			{
				perror("�����쳣");
				exit(1);	
			}
			//exit(0);
			//printf("&&&&&&&");
		  }
		  
		  else
	  	{
		 		yesorno = 1;
				if(send(client_fd,&yesorno,sizeof(int), 0) == -1)
				{
					perror("�����쳣");
					exit(1);
				}
				//����client_fd���ͻ���
				if(send(client_fd,&client_fd,sizeof(int), 0) == -1)
				{
					perror("�����쳣");
					exit(1);
				}
				if(recv(client_fd,username,10, 0) == -1)
				{
					perror("�����쳣");
					exit(1);
				}
				printf("�ͻ��˱�� :%d���ӳɹ�!\n",client_fd);
				
				
				userlist_add(&ulist,client_fd,username);//���û������û�����
				userlist_tobuf(ulist,&databuf);//�û��������ݴ��
				p = ulist;
				usercount = userlist_length(ulist);
				printf("�ͻ������%d��½�������û��б���ǰ�����û�%d��\n",client_fd,usercount);
				for(i = 0; i < usercount; i++)
				{
					if(send(p->next->fd,&databuf,sizeof(msgNode), 0) == -1)
					{
						perror("�����쳣");
						exit(1);	
					}
					p = p->next; 
				}
				
				if(pthread_create(&pthreadid, NULL, (void *)pthread, (void *)&client_fd) == -1)
				{
				perror("create pthread error!");
				exit(1);
				}
	  	}
				
		}
	

	}
			//�ر�����
					close(sockfd);
					exit(0);
		//-------------------------------------------

}
