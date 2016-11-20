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

/*�����û���Ϣ�ṹ��*/
typedef struct
{
    char name[10];/*�û���*/
    char passwd[10];/*�û�����*/
    char spasswd[10];/*�û�����*/
    int cmd;/*�����ʾ��*/
    char duff[1024];
}suser;
/*���屣���û�������Ϣ*/
typedef struct
{
    char name[20];/*���ڴ�Ŷ�Ӧ�û����ӵ��׽���������������*/
    int clieid; /*�û����ӵ��׽���������*/
    int speak;/*1���Է���Ϣ��0������*/
}current_client;

/*************************************************
  Name:        isonline
  Function:    �鿴�û��Ƿ�����
  Calls:       ��
  Called By:   sendto_one��kickone��banndsomeone��ifsend_file��rsend_file
  Input:       client,name
  Output:      ��
  Return:      0:�����ߣ��������Ǳ����û���sfd
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
  Function:    ע�ắ��,��������user.txt�ļ���
  Calls:       ��
  Called By:   main
  Input:       sfd,suser_mes
  Output:      ��
  Return:      ��
*************************************************/
void sregest(int new_fd,suser suser_mes)
{
    FILE *fr,*fd;
    int targ=0;
    char name[10];
    suser_mes.cmd=0;

    if(strcmp(suser_mes.passwd,suser_mes.spasswd)==0)
    {
        /*���û��������뱣�����ļ���*/
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
  Function:    ��¼����
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
*************************************************/
void sload(int new_fd,current_client client[],suser suser_mes)
{
    int i;
    int temp=0;
    char ch[20];
    char name[20],passwd[20];
    FILE *fd;

    /*����û���������*/
    strcpy(name,suser_mes.name);
    strcpy(passwd,suser_mes.passwd);

    /*�Ƿ񳬼��û���¼*/
    if((strcmp(name,"admin") == 0) && (strcmp(passwd,"admin") == 0))
    {
        suser_mes.cmd=1;
        strcpy(suser_mes.duff,"admin");
    }
    else
    {
        for(i = 0;i < FD_SETSIZE;i ++)
        {
            /*����client���Ӧ��sfd*/
            if((client[i].clieid >= 0)&&(client[i].clieid != new_fd))
            {
                /*�û��Ƿ��Ѿ���¼*/
                if(strcmp(name,client[i].name) == 0)
                {
                    printf("two same name %s\n",client[i].name);
                    temp = 1;
                    strcpy(suser_mes.duff,"loaded");
                    break;
                }
            }
        }
        /*�˶�user.txt�鿴�û���¼���������Ƿ���ȷ*/
        if(temp == 0)
        {
            suser_mes.cmd=1;
            /*���û�����������ӻس���*/
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
  Function:    �鿴���ߵ�ȫ���û�
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
*************************************************/
void look_user(int sfd,current_client client[],suser suser_mes)
{
    int i;
    char name[1024];
    suser_mes.cmd=2;
    strcat(name,"All user online:\n");

    /*�����������ߵ��û���*/
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
    bzero(name,sizeof(name));/*����name*/
}

/*************************************************
  Name:        sendto_one
  Function:    ��ĳ�˷�����Ϣ
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      0
*************************************************/
int sendto_one(int sfd,current_client client[],suser suser_mes)
{
    char toname[20],result[20];
    int nfd,i;
    suser to;/*�����µĽṹ�����*/

    /*���½��Ľ���������ֵ*/
    to.cmd=32;
    strcpy(to.name,suser_mes.spasswd);
    strcpy(to.spasswd,suser_mes.name);
    strcpy(to.duff,suser_mes.duff);
    strcpy(toname,suser_mes.spasswd);

    for(i = 0;i < FD_SETSIZE;i ++)
    {
        if((client[i].clieid >= 0)&&(strcmp(client[i].name,suser_mes.name) == 0))
        {
            /*�Ƿ񱻽���*/
            if(client[i].speak == 0)
            {
                printf("You haved bannded!\n");
                strcpy(suser_mes.duff,"band");
                write(sfd,&suser_mes,sizeof(suser));
                return 0;
            }
        }
    }
    if((nfd = isonline(client,toname)) == 0)/*������*/
    {
        strcpy(suser_mes.duff,"noline");
        write(sfd,&suser_mes,sizeof(suser));
    }
    else
    {
        /*����������Ϣ*/
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
  Function:    �������˷�����Ϣ
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
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
  Function:    �߳�ĳ��
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
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
  Function:    ����ĳ��
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
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
  Function:    �ظ�����Ȩ
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
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
  Function:    �������ļ�
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      0
*************************************************/
int send_file(int sfd,current_client client[],suser suser_mes)
{
    char toname[20],result[20],nfile[1024];
    int nfd,i;
    suser to;/*�����µĽṹ�����*/

    /*���½��Ľ���������ֵ*/
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
  Function:    �Ƿ��Ӧ�����ļ�
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
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
  Function:    �����ļ�
  Calls:       ��
  Called By:   main
  Input:       sfd,client,suser_mes
  Output:      ��
  Return:      ��
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
