#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<ctype.h>
#include"server_fun.h"

/* 宏定义端口号 */
#define portnumber 8000

#define MAX_LINE 80

/*************************************************
  Name:        main
  Function:    服务器函数
  Calls:       sregest，sload，look_user，sendto_one，ssendto_everyone
               kickone，banndsomeone，regain，send_file，ifsend_file
               rsend_file
  Called By:   main
  Input:       sockfd,cuser_reg
  Output:      无
  Return:      无
*************************************************/
int main(void)
{
	int  lfd;
	int cfd;
	int sfd;
	int rdy;

	suser suser_mes;
    current_client client[FD_SETSIZE];/* 客户端连接的套接字描述符数组 */
    int t=0;

	struct sockaddr_in sin;
	struct sockaddr_in cin;

	int maxi;
	int maxfd;                        /* 最大连接数 */

	fd_set rset;
	fd_set allset;

	socklen_t addr_len;         /* 地址结构长度 */

	char buffer[MAX_LINE];

	int i;
	int n;
	int len;
	int opt = 1;   /* 套接字选项 */

	char addr_p[20];



	/* 对server_addr_in  结构进行赋值  */
	bzero(&sin,sizeof(struct sockaddr_in));   /* 先清零 */
	sin.sin_family=AF_INET;                 //
	sin.sin_addr.s_addr=htonl(INADDR_ANY);  //表示接受任何ip地址   将ip地址转换成网络字节序
	sin.sin_port=htons(portnumber);         //将端口号转换成网络字节序

	/* 调用socket函数创建一个TCP协议套接口 */
	if((lfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:IPV4;SOCK_STREAM:TCP
	{
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
		exit(1);
	}


	/*设置套接字选项 使用默认选项*/
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	/* 调用bind函数 将serer_addr结构绑定到sockfd上  */
	if(bind(lfd,(struct sockaddr *)(&sin),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
		exit(1);
	}


	/* 开始监听端口   等待客户的请求 */
	if(listen(lfd,20)==-1)
	{
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
		exit(1);
	}

	printf("Accepting connections .......\n");

	maxfd = lfd;                                /*对最大文件描述符进行初始化*/
	maxi = -1;

	/*初始化客户端连接描述符集合*/
	for(i = 0;i < FD_SETSIZE;i++)
	{
                    client[i].clieid = -1;
                    client[i].speak = 1;
	}

	FD_ZERO(&allset);                     /* 清空文件描述符集合 */
	FD_SET(lfd,&allset);                    /* 将监听字设置在集合内 */


	/* 开始服务程序的死循环 */
	while(1)
	{
		rset = allset;

		/*得到当前可以读的文件描述符数*/
		rdy = select(maxfd + 1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(lfd, &rset))
		{
			addr_len = sizeof(sin);

			/* 接受客户端的请求 */
			if((cfd = accept(lfd,(struct sockaddr *)(&cin),&addr_len))==-1)
			{
				fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
				exit(1);
			}

            /*查找一个空闲位置*/
			for(i = 0; i<FD_SETSIZE; i++)
			{       //printf("%d\t",client[i]);
				if(client[i].clieid <= 0)
				{
					client[i].clieid = cfd;   /* 将处理该客户端的连接套接字设置到该位置 */
					//strcpy(client[i].name,);
					//printf("i=%d id=%d\n",i,client[i].clieid);
					//printf("client.clieid i is %d %d\n",i,client[i].clieid);
					break;
				}
			}

		/* 太多的客户端连接   服务器拒绝俄请求  跳出循环 */
			if(i == FD_SETSIZE)
			{
				printf("too many clients");
				exit(1);
			}

			FD_SET(cfd, &allset);     /* 设置连接集合 */

			if(cfd > maxfd)                  /* 新的连接描述符 */
			{
				maxfd = cfd;
			}

			if(i > maxi)
			{
				maxi = i;
			}

			if(--rdy <= 0)                /* 减少一个连接描述符 */
			{
				continue;
			}

		}

		/* 对每一个连接描述符做处理 */
		for(i = 0;i< FD_SETSIZE;i++)
		{
		    //printf("i=%d id=%d\n",i,client[i].clieid);
			if((sfd = client[i].clieid) < 0)
			{
			    printf("client.cliend :%d\n",sfd);
				continue;
			}
//			printf("sfd %d\n",sfd);

			if(FD_ISSET(sfd, &rset))
			{
				n = read(sfd,&suser_mes,sizeof(suser));
//				printf("susername:%s,clientname:%s\n",suser_mes.name,client[i].name);
				if(n == 0)
				{
					printf("the other side has been closed. \n");
					fflush(stdout);                                    /* 刷新 输出终端 */
					close(sfd);

					FD_CLR(sfd, &allset);                        /*清空连接描述符数组*/
					client[i].clieid = -1;
					bzero(client[i].name,sizeof(client[i].name));
				}

				else
				{
					/* 将客户端地址转换成字符串 */
					inet_ntop(AF_INET, &cin.sin_addr, addr_p, sizeof(addr_p));
					addr_p[strlen(addr_p)] = '\0';

					/*打印客户端地址 和 端口号*/
					printf("Client Ip is %s, port is %d\n",addr_p,ntohs(cin.sin_port));
//					printf("server cmd %d\n",suser_mes.cmd);

                    //strcpy(client[i].name,suser_mes.name);
                    switch (suser_mes.cmd)
                    {
                        case 0:
                        {
                            printf("Welcome to regest!\n");
                            sregest(sfd,suser_mes);/*注册函数*/
                            break;
                        }
                        case 1:
                        {
                            printf("Welcome to loading!\n");
                            sload(sfd,client,suser_mes);/*登陆函数*/
                            break;
                        }
                        case 2:
                        {
                            printf("Welcome to check!\n");
                            look_user(sfd,client,suser_mes);/*查看当前全部在线用户*/
                            break;
                        }
                        case 31:
                        {
                            printf("Welcome to send message to someone!\n");
                            sendto_one(sfd,client,suser_mes);/*向某人发送消息*/
                            break;
                        }
                        case 4:
                        {
                            printf("Welcome to send message to everyone!\n");
                            ssendto_everyone(sfd,client,suser_mes);
                            break;
                        }
                        case 44:
                        {
                            printf("Kick out someone!\n");
                            kickone(sfd,client,suser_mes);/*踢出某人*/
                            break;
                        }
                        case 5:
                        {
                            printf("Bannd someone!\n");
                            banndsomeone(sfd,client,suser_mes);/*禁言某人*/
                            break;
                        }
                        case 6:
                        {
                            printf("Regain someone!\n");
                            regain(sfd,client,suser_mes);/*回复被禁言者*/
                            break;
                        }
                        case 71:
                        {
                            printf("Ask send file to someone!\n");
                            send_file(sfd,client,suser_mes);/*请求发送文件*/
                            break;
                        }
                        case 72:
                        {
                            printf("Whether send file\n");
                            ifsend_file(sfd,client,suser_mes);/*回复是否允许发生文件*/
                            break;
                        }
                        case 73:
                        {
                            printf("Readlly to send file!\n");
                            rsend_file(sfd,client,suser_mes);/*发送文件*/
                            break;
                        }
                        default :
                            break;
                    }
				}

				/*如果没有可以读的套接字   退出循环*/
				if(--rdy <= 0)
				{
					break;
				}
			}
		}
	}
	close(lfd);       /* 关闭链接套接字 */
	return 0;
}
