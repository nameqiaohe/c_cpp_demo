#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include "client_fun.h"

#define portnumber 8000

int sockfd;
pthread_t p_thread;/*定义全局的线程变量*/
int thr_id;/* 线程id */
cuser msg;/*定义全局用户信息变量*/
pthread_mutex_t mutex; /*定义一个互斥锁在多线程中使用 */
char filename[20];/*发送的文件名*/

/*************************************************
  Name:        pthread
  Function:    线程接受消息后的输出结果函数
  Calls:       无
  Called By:   recemess
  Input:       messer
  Output:      相应的输出
  Return:      无
*************************************************/
void pthread(cuser messer)
{
    FILE *fp;
    char note[1024];
    time_t t;

    switch(messer.cmd)
    {
        case 2:/*显示当前在线的全部用户*/
        {
            printf("%s\n",messer.duff);
            break;
        }
        case 31:/*单向某人发消息结果*/
        {
            if(strcmp(messer.duff,"noline")==0)
            {
                printf("Send error %s is no online!\n",messer.spasswd);
                break;
            }
            else if(strcmp(messer.duff,"band")==0)
            {
                printf("Send error!You have banded!\n");
                break;
            }
            else/*显示发送消息成功并保存聊天记录*/
            {
                time(&t);
                printf("%s:I say to %s %s\n",ctime(&t),messer.spasswd,messer.duff);

                strcat(note,ctime(&t));
                strcat(note,messer.name);
                strcat(note,"  :  ");
                strcat(note,messer.duff);
                strcat(note, "\n");
                fp = fopen(messer.name,"a+");
                fputs(note, fp);
                fclose(fp);
                break;
            }
        }
        case 32:/*单发消息，接收端显示并保持聊天记录*/
        {
            time(&t);
            printf("%s%s say to me %s\n",ctime(&t),messer.spasswd,messer.duff);

            strcat(note,ctime(&t));
            strcat(note,messer.spasswd);
            strcat(note,"  :  ");
            strcat(note,messer.duff);
            strcat(note, "\n");
            fp = fopen(messer.name,"a+");
            fputs(note, fp);
            fclose(fp);
            break;
        }
        case 4:/*群发消息*/
        {
            if(strcmp(messer.duff,"band") == 0)
            {
                printf("You have banded!\n");
                break;
            }
            else/*显示群发消息的内容并保持聊天记录*/
            {
                time(&t);
                printf("%s%s say to me %s!\n",ctime(&t),messer.name,messer.duff);

                strcat(note,ctime(&t));
                strcat(note,messer.name);
                strcat(note,"  :  ");
                strcat(note,messer.duff);
                strcat(note, "\n");
                fp = fopen(msg.name,"a+");
                fputs(note, fp);
                fclose(fp);
                break;
            }
        }
        case 44:/*踢出某人*/
        {
            if(strcmp(messer.duff,"noline") == 0)
            {
                printf("%s is no oneline!\n",messer.spasswd);
            }
            else if(strcmp(messer.duff,"sucess") == 0)
            {
                printf("%s is kickout!\n",messer.spasswd);
            }
            else if(strcmp(messer.duff,"kill") == 0)
            {
                printf("You haved kick out!\n");
                exit(1);
            }
            break;
        }
        case 5:/*对某人禁言*/
        {
            if(strcmp(messer.duff,"noline") == 0)
            {
                printf("%s is no oneline!\n",messer.spasswd);
            }
            else if(strcmp(messer.duff,"sucess") == 0)
            {
                printf("%s is banded!\n",messer.spasswd);
            }
            else if(strcmp(messer.duff,"band") == 0)
            {
                printf("You haved banded !\n");
            }
            break;
        }
        case 6:/*回复某人的发消息的权力*/
        {
            if(strcmp(messer.duff,"sucess") == 0)
            {
                printf("%s can send message again!\n",messer.spasswd);
            }
            else if(strcmp(messer.duff,"regain") == 0)
            {
                printf("You can send message again!\n");
            }
            break;
        }
        case 71:/*请求发送文件结果*/
        {
            FILE *fp;
            char filem[1024];
            if(strcmp(messer.duff,"noline") == 0)
            {
                printf("Send error %s is no online!\n",messer.spasswd);
                break;
            }
            else if(strcmp(messer.duff,"band") == 0)
            {
                printf("Send error!You have banded!\n");
                break;
            }
            printf("Wait %s's answer!\n",messer.spasswd);
            break;
        }
        case 72:/*相应是否让其发送文件*/
        {
            if(strcmp(messer.passwd,"sendfile") == 0)
            {
                printf("%s want send file to you!(yes(11)\no(10))\n",messer.spasswd);
//                break;
            }
            else if(strcmp(messer.duff,"dawnline" == 0))
            {
                printf("%s is no online!\n",messer.spasswd);
            }
            else if(strcmp(messer.duff,"wait") == 0)
            {
                printf("Waiting!%s is sending!\n",messer.spasswd);
            }
            break;
        }
        case 73:/*发还是不发*/
        {
            /*可以发送了，下面就打开文件，循环发送吧*/
            if(strcmp(messer.duff,"yes") == 0)
            {
                printf("Now you can send file to %s!\n",messer.spasswd);

                if((fp = fopen(filename,"r")) == NULL)
                {
                    printf("Opne file error,no such file!\n");
                    break;
                }

                pthread_mutex_lock(&mutex); /* 锁住线程 */
                while(fgets(messer.duff,1024,fp))
                {
                    if((write(sockfd, &messer, sizeof(cuser))) == -1)
                    {
                        fprintf(stderr, "Write Error:%s\n", strerror(errno));
                        exit(1);
                    }
                }
                pthread_mutex_unlock(&mutex); /* 解锁线程 */
                fclose(fp);
                printf("Send sucess!\n");
                break;
            }
            if(strcmp(messer.duff,"no") == 0)
            {
                /*对方不然发，我们退出*/
                printf("%s not want!\n",messer.spasswd);
                break;
            }
 //           break;
        }
        case 74:/*保存对方发来的文件*/
        {
            if((fp = fopen("newfile","a+")) == NULL)
            {
                printf("Opne file error,no such file!\n");
                break;
            }
            fputs(messer.duff,fp);
            fclose(fp);
            printf("Receive OK!\n");
            break;
        }
        default:
        {
            break;
            exit(1);
        }
    }
}

/*************************************************
  Name:        recemess
  Function:    线程接受消息
  Calls:       pthread
  Called By:   use_wnd,admin_wnd
  Input:       无
  Output:      无
  Return:      无
*************************************************/
void recemess()
{
    int rfd=0;
   // bzero(&msg.passwd,sizeof(msg.passwd));
    while(1)
    {
        if(rfd = read(sockfd, &msg, sizeof(cuser)) > 0)
        {
            pthread(msg);
        }
        else
        {
            printf("server is out!\n");
            exit(1);
        }
    }
}

/*************************************************
  Name:        check
  Function:    向服务器发消息要求检查当前在线的全部用户
  Calls:       无
  Called By:   use_wnd
  Input:       无
  Output:      无
  Return:      无
*************************************************/
void check()
{
    cuser cusermess;
    cusermess.cmd = 2;
    bzero(&cusermess.name, sizeof(cusermess.name));/*对name清空*/
    strcpy(cusermess.name, msg.name);/*将该消息发送对象写入消息结构体中*/
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        sendto_someone
  Function:    向某人发生消息
  Calls:       无
  Called By:   use_wnd
  Input:       发送方的名字，要发送的话
  Output:      无
  Return:      无
*************************************************/
void sendto_someone()
{
    char say[1024],toname[20];
    cuser cusermess;
    cusermess.cmd=31;
    bzero(&cusermess.name, sizeof(cusermess.name));
    strcpy(cusermess.name, msg.name);/*将该消息发送对象写入消息结构体中*/
    printf("Enter to someone name:\n");
    scanf("%s",&toname);
    //getchar();
    printf("Enter your say:");
    scanf("%s",&say);
    //printf("%s I say to %s %s\n",ctime(&t),toname,say);
    strcpy(cusermess.duff,say);/*将发送的消息放在duff中*/
    strcpy(cusermess.spasswd,toname);/*将收信人的姓名保存到spasswd里面*/
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        sendto_everyone
  Function:    群发消息
  Calls:       无
  Called By:   use_wnd
  Input:       发送的内容
  Output:      无
  Return:      无
*************************************************/
void sendto_everyone()
{
    char say[20];
    cuser cusermess;
    cusermess.cmd = 4;
    bzero(&cusermess.name, sizeof(cusermess.name));
    strcpy(cusermess.name, msg.name);/*将该消息发送对象写入消息结构体中*/
    printf("Enter your say:");
    scanf("%s",&say);
    strcpy(cusermess.duff,say);/*将发送的消息放在duff中*/
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        look_notes
  Function:    查看当前用户的聊天记录
  Calls:       无
  Called By:   use_wnd
  Input:       无
  Output:      聊天记录
  Return:      无
*************************************************/
void look_notes()
{
    char ch[1024];
    int targ=1;
    FILE *fp;
    fp = fopen(msg.name,"r");/*打开文件*/
    printf("My chat note:\n");
    while(fgets(ch,1024,fp))/*一行一行的读取内容*/
    {
        targ = 0;
        printf("%s",ch);
    }
    if (targ == 1)
    {
        printf("You havend say anything! \n");
    }
    fclose(fp);
}

/*************************************************
  Name:        send_file
  Function:    向某人发送文件
  Calls:       无
  Called By:   use_wnd
  Input:       接收方，发送的文件路径
  Output:      无
  Return:      无
*************************************************/
void send_file()
{
    char toname[20];
    cuser cusermess;
    cusermess.cmd=71;
    bzero(&cusermess.name, sizeof(cusermess.name));
    strcpy(cusermess.name, msg.name);/*将该消息发送对象写入消息结构体中*/
    printf("Enter to someone name:\n");
    scanf("%s",&toname);
    printf("Enter what you to send:\n");
    scanf("%s",&filename);
    //getchar();strcpy(msg.duff,"yes");
//    printf("%s I say to %s %s\n",ctime(&t),toname,say);
 //   strcpy(cusermess.duff,filem);/*将发送的消息放在duff中*/
    strcpy(cusermess.spasswd,toname);/*将收信人的姓名保存到spasswd里面*/
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        use_wnd
  Function:    普通用户登录后的应用界面
  Calls:       check,sendto_someone,sendto_everyone,
               look_notes,send_file
  Called By:   main
  Input:       无
  Output:      相关输出
  Return:      无
*************************************************/
void use_wnd()
{

    int changes;
    char toname;
//    cuser cusermess;

    while(1)
    {
        printf("Enter your change:");
        scanf("%d",&changes);
        switch (changes)
        {
            /*查看当前在线用户*/
            case 2:
            {
                check();
                sleep(1);
                break;
            }
            /*向某人发送消息*/
            case 3:
            {
                printf("send to someone!\n");
                sendto_someone();
                sleep(1);
                break;
            }
            /*向大家群发消息*/
            case 4:
            {
                printf("send to everyone!\n");
                sendto_everyone();
                sleep(1);
                break;
            }
            /*查看聊天记录*/
            case 5:
            {
                look_notes();
                sleep(1);
                break;
            }
            /*发送文件*/
            case 6:
            {
                printf("send file to someone!\n");
                send_file();
                sleep(1);
                break;
            }
            /*退出*/
            case 9:
            {
                printf("2.###look all user\n3.###send message to someone \n4.###send message to everyone\n5.###look chat notes\n6.###send file\n9.###help\n0.###exit\n");
                break;
            }
            /*不允许发送文件*/
            case 10:
            {
                strcpy(msg.duff,"no");
                msg.cmd = 72;
              //  printf("case10name=%s,spassswd=%s\n",msg.name,msg.spasswd);
                pthread_mutex_lock(&mutex); /* 锁住线程 */
                if((write(sockfd, &msg, sizeof(cuser))) == -1)
                {
                    fprintf(stderr, "Write Error:%s\n", strerror(errno));
                    exit(1);
                }
                pthread_mutex_unlock(&mutex); /* 解锁线程 */
                break;
            }
            /*允许发送文件*/
            case 11:
            {
                strcpy(msg.duff,"yes");
                msg.cmd = 72;
               // printf("case11name=%s,spassswd=%s\n",msg.name,msg.spasswd);
                pthread_mutex_lock(&mutex); /* 锁住线程 */
                if((write(sockfd, &msg, sizeof(cuser))) == -1)
                {
                    fprintf(stderr, "Write Error:%s\n", strerror(errno));
                    exit(1);
                }
                pthread_mutex_unlock(&mutex); /* 解锁线程 */
                break;
            }
            default :
            {
                exit(0);
                break;
            }
        }
    }
}

/*************************************************
  Name:        kickout
  Function:    踢出某人
  Calls:       无
  Called By:   admin_wnd
  Input:       被踢方名字
  Output:      无
  Return:      无
*************************************************/
void kickout()
{
    printf("Enter you want to kick name:");
    scanf("%s",msg.spasswd);
    msg.cmd = 44;
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &msg, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        banndone
  Function:    禁言某人
  Calls:       无
  Called By:   admin_wnd
  Input:       被禁方名字
  Output:      无
  Return:      无
*************************************************/
void banndone()
{
    printf("Enter you want to bannd name:");
    scanf("%s",msg.spasswd);
    msg.cmd = 5;
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &msg, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        regainone
  Function:    回复某人的发言权
  Calls:       无
  Called By:   admin_wnd
  Input:       要恢复人名
  Output:      无
  Return:      无
*************************************************/
void regainone()
{
    printf("Enter you want to regain name:");
    scanf("%s",msg.spasswd);
    msg.cmd = 6;
    pthread_mutex_lock(&mutex); /* 锁住线程 */
    if((write(sockfd, &msg, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* 解锁线程 */
}

/*************************************************
  Name:        admin_wnd
  Function:    超级用户登录后的功能
  Calls:       kickout,banndone,regainone
  Called By:   main
  Input:       被踢方名字
  Output:      相应输出
  Return:      无
*************************************************/
void admin_wnd()
{
    int change;

    while(1)
    {
        printf("Enter your change:");
        scanf("%d",&change);
        switch(change)
        {
            case 2:
            {
                printf("Look all user!\n");
                check();
                sleep(1);
                break;
            }
            case 3:
            {
                printf("Kick out someone!\n");
                kickout();
                sleep(1);
                break;
            }
            case 4:
            {
                printf("Bannd someone!\n");
                banndone();
                sleep(1);
                break;
            }
            case 5:
            {
                printf("Regain someone!\n");
                regainone();
                sleep(1);
                break;
            }
            default :
            {
                exit(1);
                break;
            }
        }
    }
}

/*************************************************
  Name:        main
  Function:    注册用户和登录用户并显示其后的功能
  Calls:       load,regest
  Called By:   main
  Input:       被踢方名字
  Output:      相应输出
  Return:      无
*************************************************/
int main(int argc, char *argv[])
{
	int change;
	struct sockaddr_in server_addr;
	struct hostent *host;
	cuser cusermess,cuser_lod,cuser_reg;

    /* 使用hostname查询host 名字 */
	if(argc!=2)
	{
		fprintf(stderr,"Usage:%s hostname \a\n",argv[0]);
		exit(1);
	}

	if((host=gethostbyname(argv[1]))==NULL)
	{
		fprintf(stderr,"Gethostname error\n");
		exit(1);
	}

	/* 客户程序开始建立 sockfd描述符 */
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
		exit(1);
	}

	/* 客户程序填充服务端的资料 */
	bzero(&server_addr,sizeof(server_addr)); // 初始化,置0
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber);  // (将本机器上的short数据转化为网络上的short数据)端口号
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); // IP地址

	/* 客户程序发起连接请求 */
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
		exit(1);
	}
	while(1)
	{
        printf("1.####load  2.####register  0.####exit \n");
        scanf("%d",&change);
        if(change==1)/*登录模块*/
        {
            load(sockfd,cuser_lod);
        }
        else if(change==2)/*注册模块*/
        {
            regest(sockfd,cuser_reg);
        }
        else
        {
            close(sockfd);
            exit(0);
        }

        read(sockfd,&msg,sizeof(cuser));/*读取注册登录的结果*/

        switch(msg.cmd)
        {
            case 0:
            {
                printf("%s\n",msg.duff);
                break;
            }
            case 1:
            {
                if(strcmp(msg.duff,"loaded")==0)
                {
                    printf("You have loaded!\n");
                    break;
                }
                else if(strcmp(msg.duff,"sucess")==0)/*普通用户登录成功*/
                {
                    printf("Load sucess!\n");
                    thr_id = pthread_create(&p_thread, NULL, recemess, NULL);
                    printf("2.###look all user\n3.###send message to someone \n4.###send message to everyone\n5.###look chat notes\n6.###send file\n9.###help\n0.###exit\n");
                    use_wnd();
                    break;
                }
                else if(strcmp(msg.duff,"admin")==0)/*超级用户登录成功*/
                {
                    printf("Welcome Administrator to loading!\n");
                    thr_id = pthread_create(&p_thread, NULL, recemess, NULL);
                    printf("2.###look all user\n3.###kick out someone \n4.###banned to post someone \n5.###regain someone\n0.###exit!\n");
                    admin_wnd();
                    break;
                }
                else
                {
                     printf("Load error!\n");
                     break;
                }
            }
            default :
            {
                close(sockfd);
                exit(0);
                break;
            }
        }
    }

	/* 结束通讯 */
	close(sockfd);
	exit(0);
}
