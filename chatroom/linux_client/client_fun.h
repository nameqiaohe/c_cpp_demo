#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

/*����ṹ��*/
typedef struct
{
    char name[10];/*�û���*/
    char passwd[10];/*�û�����*/
    char spasswd[10];/*�û�����*/
    int cmd;/*�����ʾ��*/
    char duff[1024];/*ִ�н����Ϣ*/
}cuser;

/*************************************************
  Name:        load
  Function:    ��¼����
  Calls:       ��
  Called By:   main
  Input:       sockfd,cuser_lod
  Output:      ��
  Return:      ��
*************************************************/
void load(int sockfd,cuser cuser_lod)
{
    char mess[1024];
    printf("Enter your name:");
    scanf("%s",&cuser_lod.name);
    printf("Enter your passwd:");
    scanf("%s",&cuser_lod.passwd);
    cuser_lod.cmd=1;/*��½��Ƿ�*/
    write(sockfd,&cuser_lod,sizeof(cuser_lod));
}

/*************************************************
  Name:        regest
  Function:    ע�ắ��
  Calls:       ��
  Called By:   main
  Input:       sockfd,cuser_reg
  Output:      ��
  Return:      ��
*************************************************/
void regest(int sockfd,cuser cuser_reg)
{
    printf("Enter your name:");
    scanf("%s",&cuser_reg.name);
    printf("Enter your passwd:");
    scanf("%s",&cuser_reg.passwd);
    printf("Enter your passwd second:");
    scanf("%s",&cuser_reg.spasswd);
    cuser_reg.cmd=0;/*ע��ı�Ƿ�*/
    // regist(sockfd,reg_user);
    write(sockfd,&cuser_reg,sizeof(cuser_reg));
   // printf("%s\n",cuser_reg.name);
}



