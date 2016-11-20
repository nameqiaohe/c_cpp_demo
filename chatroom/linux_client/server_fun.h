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

/*定义用户信息结构体*/
typedef struct
{
    char name[10];/*用户名*/
    char passwd[10];/*用户密码*/
    char spasswd[10];/*用户密码*/
    int cmd;/*命令标示符*/
    char duff[1024];
}suser;
/*定义保存用户连接信息*/
typedef struct
{
    char name[20];/*用于存放对应用户连接的套接字描述符的姓名*/
    int clieid; /*用户连接的套接字描述符*/
    int speak;/*1可以发消息，0不可以*/
}current_client;

/*************************************************
  Name:        isonline
  Function:    查看用户是否在线
  Calls:       无
  Called By:   sendto_one，kickone，banndsomeone，ifsend_file，rsend_file
  Input:       client,name
  Output:      无
  Return:      0:不在线，其他就是被查用户的sfd
*************************************************/
int isonline(current_client client[],char name[])
{
    int i;
    for(i = 0;i < FD_SETSIZE;i ++)
    {
       // printf("id = %d,name=%s\n",client[i].clieid,client[i].name);
        if((client[i].clieid >= 0)&&(strcmp(name,client[i].name) == 0))
        {
         //   printf("id = %d,name=%s\n",client[i].clieid,client[i].name);
            return client[i].clieid;
        }
    }
    return 0;
}

/*************************************************
  Name:        sregest
  Function:    注册函数,并保持在user.txt文件中
  Calls:       无
  Called By:   main
  Input:       sfd,suser_mes
  Output:      无
  Return:      无
*************************************************/
void sregest(int new_fd,suser suser_mes)
{
    FILE *fr,*fd;
    int targ=0;
    char name[10];
    suser_mes.cmd=0;

    if(strcmp(suser_mes.passwd,suser_mes.spasswd)==0)
    {
        /*将用户名和密码保存在文件中*/
        if((fr = fopen("user.txt","at+")) == NULL)
        {
            printf("Open error!\n");
        }
        strcat(suser_mes.name,"\n");
        strcat(suser_mes.passwd,"\n");

        while(fgets(name,10,fr))
        {
            if(strcmp(name,suser_mes.name)==0)
             {
                 targ=1;
                 strcpy(suser_mes.duff,"The user is existed\n");
             }
            fgets(name,10,fr);
        }
        fclose(fr);

        if(targ==0)
        {
            if((fd = fopen("user.txt","at+")) == NULL)
            {
                printf("Open error!\n");
            }
            fputs(suser_mes.name, fd);
            fputs(suser_mes.passwd, fd);

            suser_mes.cmd=0;
            strcpy(suser_mes.duff,"Regist success!\n");
            fclose(fd);
        }
    }
    else
        strcpy(suser_mes.duff,"Two pwd diffent\n");

    write(new_fd,&suser_mes,sizeof(suser_mes));
}

/*************************************************
  Name:        sload
  Function:    登录函数
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void sload(int new_fd,current_client client[],suser suser_mes)
{
    int i;
    int temp=0;
    char ch[20];
    char name[20],passwd[20];
    FILE *fd;

    /*另存用户名和密码*/
    strcpy(name,suser_mes.name);
    strcpy(passwd,suser_mes.passwd);

    /*是否超级用户登录*/
    if((strcmp(name,"admin") == 0) && (strcmp(passwd,"admin") == 0))
    {
        suser_mes.cmd=1;
        strcpy(suser_mes.duff,"admin");
    }
    else
    {
        for(i = 0;i < FD_SETSIZE;i ++)
        {
            /*遍历client查对应的sfd*/
            if((client[i].clieid >= 0)&&(client[i].clieid != new_fd))
            {
                /*用户是否已经登录*/
                if(strcmp(name,client[i].name) == 0)
                {
                    printf("two same name %s\n",client[i].name);
                    temp = 1;
                    strcpy(suser_mes.duff,"loaded");
                    break;
                }
            }
        }
        /*核对user.txt查看用户登录名和密码是否正确*/
        if(temp == 0)
        {
            suser_mes.cmd=1;
            /*对用户名和密码添加回车符*/
            strcat(name,"\n");
            strcat(passwd,"\n");
            strcpy(suser_mes.duff,"Load error!\n");

            fd=fopen("user.txt","r");
            while(fgets(ch,1024,fd))
            {
                if(strcmp(ch,name) == 0)
                {
                    fgets(ch,1024,fd);
                    if(strcmp(ch,passwd)==0)
                    {
                        strcpy(suser_mes.duff,"sucess");
                        for(i = 0;i < FD_SETSIZE;i ++)
                        {
                            if((client[i].clieid >= 0)&&(client[i].clieid ==new_fd))
                            {
                                strcpy(client[i].name,suser_mes.name);
                            }
                        }
                    }
                }
            }
            fclose(fd);
        }
    }
    write(new_fd,&suser_mes,sizeof(suser));

}

/*************************************************
  Name:        look_user
  Function:    查看在线的全部用户
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void look_user(int sfd,current_client client[],suser suser_mes)
{
    int i;
    char name[1024];
    suser_mes.cmd=2;
    strcat(name,"All user online:\n");

    /*遍历查找在线的用户名*/
    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if(client[i].clieid >= 0)
        {
            strcat(name,client[i].name);
            strcat(name,"\n");
        }
    }
    strcpy(suser_mes.duff,name);
    write(sfd,&suser_mes,sizeof(suser));
    bzero(name,sizeof(name));/*清零name*/
}

/*************************************************
  Name:        sendto_one
  Function:    向某人发送消息
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      0
*************************************************/
int sendto_one(int sfd,current_client client[],suser suser_mes)
{
    char toname[20],result[20];
    int nfd,i;
    suser to;/*创建新的结构体变量*/

    /*对新建的结果体变量赋值*/
    to.cmd=32;
    strcpy(to.name,suser_mes.spasswd);
    strcpy(to.spasswd,suser_mes.name);
    strcpy(to.duff,suser_mes.duff);
    strcpy(toname,suser_mes.spasswd);

    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if((client[i].clieid >= 0)&&(strcmp(client[i].name,suser_mes.name) == 0))
        {
            /*是否被禁言*/
            if(client[i].speak == 0)
            {
                printf("You haved bannded!\n");
                strcpy(suser_mes.duff,"band");
                write(sfd,&suser_mes,sizeof(suser));
                return 0;
            }
        }
    }
    if((nfd = isonline(client,toname)) == 0)/*不在线*/
    {
        strcpy(suser_mes.duff,"noline");
        write(sfd,&suser_mes,sizeof(suser));
    }
    else
    {
        /*特殊聊天信息*/
        if(strcmp(suser_mes.duff,"welcome") == 0)
        {
            strcpy(suser_mes.duff,suser_mes.name);
            strcat(suser_mes.duff," hero,welcome to our chat room!");
            strcpy(to.duff,suser_mes.duff);
        }
        else if(strcmp(suser_mes.duff,"xxx") == 0)
        {
            strcpy(suser_mes.duff,"big smile");
            strcpy(to.duff,suser_mes.duff);
        }
        write(sfd,&suser_mes,sizeof(suser));
        write(nfd,&to,sizeof(suser));
    }

    return 0;
}

/*************************************************
  Name:        ssendto_everyone
  Function:    向所有人发生消息
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      0
*************************************************/
int ssendto_everyone(int sfd,current_client client[],suser suser_mes)
{
    int i;
    char temp[20];
    suser_mes.cmd = 4;

    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if((client[i].clieid >= 0)&&(strcmp(client[i].name,suser_mes.name) == 0))
        {
            if(client[i].speak == 0)
            {
                printf("You haved bannded!\n");
                strcpy(suser_mes.duff,"band");
                write(sfd,&suser_mes,sizeof(suser));
                return 0;
            }
        }
    }

    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if(client[i].clieid >= 0)
        {
            if(strcmp(suser_mes.duff,"welcome") == 0)
            {
                strcpy(suser_mes.duff,suser_mes.name);
                strcat(suser_mes.duff," hero,welcome to our chat room!");
            }
            else if(strcmp(suser_mes.duff,"xxx") == 0)
            {
                strcpy(suser_mes.duff,"big smile");
            }
            write(client[i].clieid,&suser_mes,sizeof(suser));
        }
    }
    return 0;
}

/*************************************************
  Name:        kickone
  Function:    踢出某人
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void kickone(int sfd,current_client client[],suser suser_mes)
{
    int nfd,i;
    char kiname[20];
    suser to;
    to.cmd = 44;
    suser_mes.cmd = 44;
    strcpy(kiname,suser_mes.spasswd);

    if((nfd = isonline(client,kiname)) == 0)
    {
       // printf("nfd=%d\n",nfd);
        strcpy(suser_mes.duff,"noline");
        write(sfd,&suser_mes,sizeof(suser));
    }
    else
    {
        for(i = 0;i < FD_SETSIZE;i ++)
        {

            if((client[i].clieid >= 0)&&(strcmp(kiname,client[i].name) == 0))
            {
                strcpy(suser_mes.duff,"sucess");
                strcpy(to.duff,"kill");
                break;
            }
        }
        write(sfd,&suser_mes,sizeof(suser));
        write(nfd,&to,sizeof(suser));
    }
}

/*************************************************
  Name:        banndsomeone
  Function:    禁言某人
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void banndsomeone(int sfd,current_client client[],suser suser_mes)
{
    int nfd,i;
    char banname[20];
    suser to;
    to.cmd = 5;
    suser_mes.cmd = 5;
    strcpy(banname,suser_mes.spasswd);

    if((nfd = isonline(client,banname)) == 0)
    {
        strcpy(suser_mes.duff,"noline");
        write(sfd,&suser_mes,sizeof(suser));
    }
    else
    {
        for(i = 0;i < FD_SETSIZE;i ++)
        {
            if((client[i].clieid >= 0)&&(strcmp(banname,client[i].name) == 0))
            {
                client[i].speak = 0;
                strcpy(suser_mes.duff,"sucess");
                strcpy(to.duff,"band");

                break;
            }
        }
        write(sfd,&suser_mes,sizeof(suser));
        write(nfd,&to,sizeof(suser));
    }
}

/*************************************************
  Name:        regain
  Function:    回复发言权
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void regain(int sfd,current_client client[],suser suser_mes)
{
    int nfd,i;
    char rename[20];
    suser to;
    to.cmd = 6;
    suser_mes.cmd = 6;
    strcpy(rename,suser_mes.spasswd);

    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if((client[i].clieid >= 0)&&(strcmp(rename,client[i].name) == 0))
        {
            client[i].speak = 1;
            strcpy(suser_mes.duff,"sucess");
            strcpy(to.duff,"regain");
            break;
        }
    }
    write(sfd,&suser_mes,sizeof(suser));
    write(nfd,&to,sizeof(suser));
}

/*************************************************
  Name:        send_file
  Function:    请求发送文件
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      0
*************************************************/
int send_file(int sfd,current_client client[],suser suser_mes)
{
    char toname[20],result[20],nfile[1024];
    int nfd,i;
    suser to;/*创建新的结构体变量*/

    /*对新建的结果体变量赋值*/
    to.cmd=72;
    strcpy(to.name,suser_mes.spasswd);
    strcpy(to.spasswd,suser_mes.name);
    strcpy(toname,suser_mes.spasswd);

    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if((client[i].clieid >= 0)&&(strcmp(client[i].name,suser_mes.name) == 0))
        {
            if(client[i].speak == 0)
            {
                printf("You haved bannded!\n");
                strcpy(suser_mes.duff,"band");
                write(sfd,&suser_mes,sizeof(suser));
                return 0;
            }
        }
    }

    nfd = isonline(client,toname);

    if(nfd == 0)
    {
        //printf("nfd=%d\n",nfd);
        strcpy(suser_mes.duff,"noline");
        write(sfd,&suser_mes,sizeof(suser));
    }
    else
    {
        strcpy(to.passwd,"sendfile");
        write(sfd,&suser_mes,sizeof(suser));
        write(nfd,&to,sizeof(suser));
        bzero(&to.passwd, sizeof(to.passwd));
    }
    return 0;
}

/*************************************************
  Name:        ifsend_file
  Function:    是否答应发送文件
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void ifsend_file(int sfd,current_client client[],suser suser_mes)
{
    suser to;
    int nfd = 0;
    suser_mes.cmd = 72;
    to.cmd = 73;
    strcpy(to.name,suser_mes.spasswd);
    strcpy(to.spasswd,suser_mes.name);

    if(strcmp(suser_mes.duff,"yes") == 0)
    {
        printf("%s can send file !\n",suser_mes.spasswd);
        strcpy(to.duff,"yes");
    }
    else
    {
        printf("%s can not send file !\n",suser_mes.spasswd);
        strcpy(to.duff,"no");
    }
  //  bzero(&suser_mes.passwd,sizeof(suser_mes.passwd));
    strcpy(suser_mes.duff,"wait");

    if((nfd = isonline(client,suser_mes.spasswd)) != 0)
    {
        write(nfd,&to,sizeof(suser));
    }
    else
    {
        strcpy(suser_mes.duff,"dawnline");
    }

    write(sfd,&suser_mes,sizeof(suser));
}

/*************************************************
  Name:        rsend_file
  Function:    发送文件
  Calls:       无
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      无
  Return:      无
*************************************************/
void rsend_file(int sfd,current_client client[],suser suser_mes)
{
    int nfd;
    char temp[20];
    strcpy(temp,suser_mes.spasswd);
    strcpy(suser_mes.name,temp);
    strcpy(suser_mes.spasswd,suser_mes.name);
    suser_mes.cmd = 74;

    if((nfd = isonline(client,suser_mes.name)) != 0)
    {
        write(nfd,&suser_mes,sizeof(suser));
    }
}
