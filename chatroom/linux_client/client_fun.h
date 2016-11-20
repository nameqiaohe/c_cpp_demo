#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*定义结构体*/
typedef struct
{
    char name[10];/*用户名*/
    char passwd[10];/*用户密码*/
    char spasswd[10];/*用户密码*/
    int cmd;/*命令标示符*/
    char duff[1024];/*执行结果信息*/
}cuser;

/*************************************************
  Name:        load
  Function:    登录函数
  Calls:       无
  Called By:   main
  Input:       sockfd,cuser_lod
  Output:      无
  Return:      无
*************************************************/
void load(int sockfd,cuser cuser_lod)
{
    char mess[1024];
    printf("Enter your name:");
    scanf("%s",&cuser_lod.name);
    printf("Enter your passwd:");
    scanf("%s",&cuser_lod.passwd);
    cuser_lod.cmd=1;/*登陆标记符*/
    write(sockfd,&cuser_lod,sizeof(cuser_lod));
}

/*************************************************
  Name:        regest
  Function:    注册函数
  Calls:       无
  Called By:   main
  Input:       sockfd,cuser_reg
  Output:      无
  Return:      无
*************************************************/
void regest(int sockfd,cuser cuser_reg)
{
    printf("Enter your name:");
    scanf("%s",&cuser_reg.name);
    printf("Enter your passwd:");
    scanf("%s",&cuser_reg.passwd);
    printf("Enter your passwd second:");
    scanf("%s",&cuser_reg.spasswd);
    cuser_reg.cmd=0;/*注册的标记符*/
    // regist(sockfd,reg_user);
    write(sockfd,&cuser_reg,sizeof(cuser_reg));
   // printf("%s\n",cuser_reg.name);
}



