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

#define SERVPORT 2345     //端口号
#define MAXDATASIZE 1024  //传输结构体数据域最大字节

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
	QueuePtr front;//队头指针
	QueuePtr rear;//队尾指针
}linkQueue;

/* 定义全局变量*/
//客户端信息
int sockfd;
int myfd;
char myname[10];
//标志
int insetflag = 0;//发送参数设置
int wrlockflag = 0;//队列写锁
//时间变量
time_t timep;
//所有窗口指针
WINDOW *inputWin,*inputMsgWin,*inputDesWin,*inputTypeWin,*inputCallWin; 
WINDOW *msgWin,*msgCntWin;
WINDOW *userListWin,*userListCntWin; 
msgList mlist;//消息链表指针
linkQueue sendQueue,recvQueue;//接受 发送队列

/* 初始化队列 */
void InitQueue(linkQueue *Q)
{
	Q->front = (QueuePtr)malloc(sizeof(QNode));
	Q->rear = Q->front;
	Q->front->next = NULL;
	//Q.rear->next = NULL;
}
/* 队列 数据入列 */
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

/* 队列 数据出列 */
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

/* 判断队列是否为空 */
int EmpQueue(linkQueue Q)
{
	if(Q.front == Q.rear)
		return 1;
	else
		return 0;
}

/* 初始化消息内容,无头结点循环链表，长度为8结点 */
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

/* 更新消息链表，mlist指向最先收到的消息 */
void msgList_updata(char* attr,char *msg)
{
	strcpy(mlist->attrstr,attr);
	strcpy(mlist->msgstr,msg);
	mlist = mlist->next;
}

/* 获取时间函数 */
void gettime(char *str)
{
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	sprintf(str,"%02d:%02d:%02d",p->tm_hour+8,p->tm_min,p->tm_sec);
}
/* 修改提示信息 */
void setcall(int num)
{
	char callstr[58];
	wclear(inputCallWin);
	switch(num)
	{
		case 1:	//输入消息
			strcpy(callstr,"提示：请输入消息，按Enter键发送！Ctrl+C更改发送参数！");
			break;
		case 2:	//输入消息 错误信息
			strcpy(callstr,"错误：消息为空,请重新输入!Ctrl+C更改发送参数!");
			break;
		case 3:	//输入用户编号 
			strcpy(callstr,"提示：请参照右侧用户列表输入目标用户编号,群发编号为 0.");
			break;
		case 4:	//输入用户编号 错误信息
			strcpy(callstr,"错误：编号错误! 请参照右侧用户列表,群发编号为 0 .");
			break;
		case 5:	//输入类型
			strcpy(callstr,"提示：请输入发送类型编号 - 消息 : 0  文件 : 1 ");
			break;
		case 6:	//输入类型 错误信息
			strcpy(callstr,"错误：输入错误,请输入发送类型编号 - 消息 : 0  文件 : 1 ");
			break;		
		case 7:
			strcpy(callstr,"提示：按Enter键进入参数设置！");
		default:
			break;
	}
	mvwaddstr(inputCallWin,0,0,callstr);
	wrefresh(inputCallWin);
}

/*输入窗口显示函数，显示和刷新窗口内容*/
void inputWin_display(void)
{
	wclear(inputWin);
	box(inputWin,0,0);//画盒子
	mvwaddstr(inputWin,0,25,"  Input ");
	mvwaddstr(inputWin,1,2,"发送对象编号:[");
	mvwaddstr(inputWin,1,20,"]");
	mvwaddstr(inputWin,1,42,"类型:[");
	mvwaddstr(inputWin,1,58,"]");	
	mvwaddstr(inputWin,2,2,"Content:");
	mvwaddstr(inputWin,3,2,"[");
	mvwaddstr(inputWin,3,58,"]");
	//子窗口内容
	wrefresh(inputWin);
}
/* 刷新消息显示 */
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
/*消息窗口显示函数，显示和刷新窗口内容*/
void msgWin_display(void)
{
	wclear(msgWin);
	box(msgWin,0,0);//画盒子
	mvwaddstr(msgWin,0,25,"  Message ");
	wrefresh(msgWin);
	msgCnt_display();
}

/*用户列表窗口显示函数，显示和刷新窗口内容*/
void userListWin_display(void)
{
	wclear(userListWin);
	box(userListWin,0,0);//画盒子
	mvwaddstr(userListWin,0,5,"  UserList ");
	//子窗口内容
	mvwprintw(userListWin,1,3,"本客户端编号:%d",myfd);
	wrefresh(userListWin);
}
/*屏幕初始化函数，建立各个窗口及其子窗口并绘制界面*/
void screen_init(void)
{
	inputWin = newwin(6,60,18,0);//创建输入窗口及子窗口 
	inputDesWin = subwin(inputWin,1,3,19,16); //目的
	inputTypeWin = subwin(inputWin,1,10,19,48);//类型
	inputMsgWin = subwin(inputWin,1,53,21,3);//消息 
	inputCallWin = subwin(inputWin,1,56,22,2);//提示 
	msgWin = newwin(18,60,0,0);//创建消息窗口及子窗口 
	msgCntWin = subwin(msgWin,16,58,1,1);//内容 
	userListWin = newwin(24,20,0,60);//用户列表窗口及子窗口 
	userListCntWin = subwin(userListWin,22,18,1,1);//内容 
	//显示窗口内容	
	msgWin_display();
	inputWin_display();
	userListWin_display();
}

/* 自定义信号处理函数，用于输入切换 */
void input_set(int sign_no)
{
	if(sign_no == SIGINT)
	{	
		insetflag = 1;
		setcall(7);//参数设置提示
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
			perror("发送异常");
			exit(1);
		}
		endwin();
		printf("客户端正常退出！\n");
		execl("/bin/clear","clear",NULL);
		exit(0);
	}
}
/* 数据发送线程 */
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
				perror("发送异常");
				exit(1);
			}
		}
	}
}
/* 数据接受线程 */
void recv_thread(void)
{
	dataNode databuf;
	while(1)
	{
		if(recv(sockfd,&databuf,sizeof(dataNode),0) == -1)
		{
			perror("接受异常");
			exit(1);
		}
		EnQueue(&recvQueue,databuf);
	}
}
/* 数据处理线程 */
void hand_thread(void)
{
	msgNode msgbuf;
	dataNode databuf;
	int i;
	while(1)
	{
		if(EmpQueue(recvQueue) == 0)//消息处理
		{
			DeQueue(&recvQueue,&databuf);
			if(databuf.type == 0)
			{
				memcpy(&msgbuf,databuf.buf,sizeof(msgNode));
				msgList_updata(msgbuf.attrstr,msgbuf.msgstr);
				msgCnt_display();
				wrefresh(inputMsgWin);
			}
			else if(databuf.type == 1)//文件处理
			{
				//文件接收处理
			}
			else if(databuf.type == 2)//用户列表处理
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
/* 输入控制线程 */
void inputcnl_thread(void)
{
	/* 初始化输入控制 */
	dataNode databuf;
	msgNode msgbuf;
	char timestr[10];
	int type = 0,desnum = 0;
	int num;
	int inerrflag = 0;//输入错误标志
	insetflag == 0;
	while(1)
	{
		//参数设置跳转
		if(insetflag == 1)
		{
			//目标编号输入
			while(1)
			{
				if(inerrflag == 0)
					setcall(3);//正常提示
				else
					setcall(4);//错误提示
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
			//类型编号输入
			while(1)
			{
				if(inerrflag == 0)
					setcall(5);//正常提示
				else
					setcall(6);//错误提示
				wclear(inputTypeWin);
				mvwscanw(inputTypeWin,0,0,"%d",&num);
				wclear(inputTypeWin);
				if(num == 0)
				{
					mvwaddstr(inputTypeWin,0,2,"消息");
					wrefresh(inputTypeWin);
					inerrflag = 0;
					type = 0;
					break;
				}
				else if(num == 1)
				{
					mvwaddstr(inputTypeWin,0,2,"文件");
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
		//参数显示处理
		mvwprintw(inputDesWin,0,0,"%3d",desnum);
		if(type == 0)
			mvwaddstr(inputTypeWin,0,2,"消息");
		else if(type == 1)
			mvwaddstr(inputTypeWin,0,2,"文件");	
		wrefresh(inputDesWin);
		wrefresh(inputTypeWin);
		//消息内容输入
		if(type == 0)
		{
			if(inerrflag == 0)
				setcall(1);//输入信息正常提示
			else
				setcall(2);//输入信息错误提示
			wclear(inputMsgWin);
			mvwgetstr(inputMsgWin,0,0,msgbuf.msgstr);//获取信息字符串
			if(insetflag == 1) continue;
			if(strlen(msgbuf.msgstr) < 1)
			{
				inerrflag = 1;	//错误循环
				continue;
			}				
			else
			{
				inerrflag = 0;
				// 将消息装入结构体
				databuf.type = type;
				databuf.src_client_fd = myfd;//////
				databuf.des_client_fd = desnum;/////
				gettime(timestr);//写入发送表示字符串
				if(desnum == 0)
					sprintf(msgbuf.attrstr,"[群]%-10s   %s",myname,timestr);
				else
					sprintf(msgbuf.attrstr,"[私]%-10s   %s",myname,timestr);
				memcpy(databuf.buf,&msgbuf,sizeof(msgNode));//将消息结点放入数据区
				//msgList_updata(msgbuf.attrstr,msgbuf.msgstr);
				//msgCnt_display();
				EnQueue(&sendQueue,databuf);
			}
		}
		else if(type == 1)
		{
			//发送文件
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

	//获取服务器名
 if(argc < 2)
  {
  fprintf(stderr,"请输入服务器地址!\n");
  exit(1);
  }
	//if((host = gethostbyname("127.0.0.1")) == NULL)
	if((host = gethostbyname(argv[1])) == NULL)
	{
		perror("获取服务器名错误");
		exit(1);
	}

	//建立socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("建立socket失败");
		exit(1);
	}
	
	//设置 sockaddr_in 结构体中相关参数
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERVPORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(serv_addr.sin_zero,8);

	//连接服务器
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1)
	{
		perror("连接错误");
		exit(1);
	}
	else
	{
		printf("连接服务器成功!\n");
	}

	//获取用户名
	puts("请用户选择： 0登录  1注册  ");
	scanf("%d",&useinfo.flag);
	puts("请输入用户名");
	scanf("%s",useinfo.name);
	puts("请输入密码");
	scanf("%s",useinfo.password);
	
  strcpy(myname,useinfo.name);
	printf("用户名:%s\n",myname);

//---------------------------------------------
	if(send(sockfd,&useinfo,sizeof(useinfo), 0) == -1)
		{
			perror("发送异常");
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
				puts("注册成功");
				exit(0);	
			}	
		}
	else
		{
			if(yesorno == 0)
			{
				puts("用户名和密码不正确");
				exit(0);	
			}
		}


//---------------------------------------------

	//接受服务器分配的客户端fd
	if((recvbytes = recv(sockfd,&myfd,sizeof(int), 0)) == -1)
	{
		perror("recv()");
		exit(1);
	}
	printf("客户端编号: %d\n",myfd);
	//发送用户给服务器
	if((sendbytes = send(sockfd,myname,10, 0)) == -1)
	{
		perror("send()");
		exit(1);
	}

	//getchar();
	//初始化消息链表
	msgList_init();
	//初始化队列
	InitQueue(&sendQueue);
	InitQueue(&recvQueue);
	//初始化图形界面
	initscr();
	screen_init(); 
	//更改SIGINT信号量处理函数 
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
	//等待线程结束
	pthread_join(inputthid,NULL);
	pthread_join(sendthid,NULL);
	pthread_join(recvthid,NULL);
	//关闭图形界面
	endwin();

	return 0;
}
