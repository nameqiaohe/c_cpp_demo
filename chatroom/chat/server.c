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


/* 全局变量 */
UserList  ulist;//用户链表指针

/* --用户链表处理函数-- */
/* 用户链表初始化 */
void userlist_init(UserList *L) 
{
	*L=(UserList)malloc(sizeof(UserNode)); 
	(*L)->next=NULL;      
} 

/* 用户链表求长度 */
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

/* 用户链表添加用户 */
void userlist_add(UserList *L,int userfd,char username[10])
{
	UserList s;
	s = (UserList) malloc ( sizeof (UserNode));
	s->fd = userfd;
	strcpy(s->name,username);
	s->next = (*L)->next;
	(*L)->next = s;
}

/* 用户链表删除用户 */
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

/* 用户链表查找用户 */
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

/* 将用户链表存入数据结点 */
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
	databuf->src_client_fd = userlist_length(L);//存放用户链表长度
}


/*客户端收发线程*/
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
			perror("接受异常");
			exit(1);
		}
		if(databuf.type == 0)//消息
		{
			if(databuf.des_client_fd == 0)//公聊
			{
				usercount = userlist_length(ulist);
				p = ulist; 
				for(i = 0;i < usercount; i++)
				{  
					if((sendbytes = send(p->next->fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("发送异常");
						exit(1);	
					}
					p = p->next;
				}
			}
			else//私聊
			{
				
				if(userlist_search(ulist,databuf.des_client_fd) == -1)//目标客户机不存在
				{
					databuf.type = 0;
					sprintf(msgbuf.attrstr,"[系] 编号为%d的客户机不存在或已下线，",databuf.des_client_fd);
					sprintf(msgbuf.msgstr,"     请参照右侧用户列表选择正确编号！");
					memcpy(databuf.buf,&msgbuf,sizeof(msgNode));
					if((sendbytes = send(client_fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("发送异常");	  
						exit(1);
					}
				}
				else//发送到目标客户机以及源客户机
				{
					if((sendbytes = send(databuf.des_client_fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("发送异常");	  
						exit(1);
					}
					//send to src_client
					if((sendbytes = send(databuf.src_client_fd,&databuf,sizeof(dataNode), 0)) == -1)
					{
						perror("发送异常");	  
						exit(1);
					}
				}
			}
		} 
		else if(databuf.type == 1)//文件
		{
		}
		else if(databuf.type == 8)//退出
		{
			userlist_del(&ulist,client_fd);
			userlist_tobuf(ulist,&databuf);
			p = ulist; 
			usercount = userlist_length(ulist);
			printf("客户机编号%d退出，更新用户列表，当前在线用户%d。\n",client_fd,usercount);
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
						perror("发送异常");
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
//注册用户名和密码到文件中
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

//判断用户是否注册
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
	//初始化用户链表
	userlist_init(&ulist);

	//建立socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))==-1){
		perror("建立socket失败");
		exit(1);
	}
	printf("建立socket成功，socket端口号为  %d。\n", sockfd);
  
	//设置 sockaddr_in 结构体中相关参数
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero),8);
 
	//绑定ip和端口
	if(bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))==-1)
	{
		perror("绑定IP失败");
		exit(1);
	}
	printf("绑定IP成功! \n");
  
	//监听连接
	if(listen(sockfd,MAXCLIENT)==-1)
	{
		perror("监听连接失败");
		exit(1);
	}
	printf("正在监听连接......\n");

	while(1)
	{
		//等待客户端连接
		if((client_fd=accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size))==-1)
		{  
			perror("等待连接失败");
			exit(1);
		}
		
		
		//------------------------------------------
	  if((recvbytes = recv(client_fd, &useinfo, sizeof(REGMESSAGE), 0)) == -1)
		{
			perror("接受异常");
			exit(1);
		}
		
		//printf("**********\n");
		if(useinfo.flag == 1)  //注册      
		{
			RregisterUser(&useinfo);
			yesorno = 0;
			if(send(client_fd,&yesorno,sizeof(int), 0) == -1)
			{
				perror("发送异常");
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
				perror("发送异常");
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
					perror("发送异常");
					exit(1);
				}
				//发送client_fd给客户端
				if(send(client_fd,&client_fd,sizeof(int), 0) == -1)
				{
					perror("发送异常");
					exit(1);
				}
				if(recv(client_fd,username,10, 0) == -1)
				{
					perror("接收异常");
					exit(1);
				}
				printf("客户端编号 :%d连接成功!\n",client_fd);
				
				
				userlist_add(&ulist,client_fd,username);//新用户加入用户链表
				userlist_tobuf(ulist,&databuf);//用户链表数据打包
				p = ulist;
				usercount = userlist_length(ulist);
				printf("客户机编号%d登陆，更新用户列表，当前在线用户%d。\n",client_fd,usercount);
				for(i = 0; i < usercount; i++)
				{
					if(send(p->next->fd,&databuf,sizeof(msgNode), 0) == -1)
					{
						perror("发送异常");
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
			//关闭连接
					close(sockfd);
					exit(0);
		//-------------------------------------------

}
