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
pthread_t p_thread;/*����ȫ�ֵ��̱߳���*/
int thr_id;/* �߳�id */
cuser msg;/*����ȫ���û���Ϣ����*/
pthread_mutex_t mutex; /*����һ���������ڶ��߳���ʹ�� */
char filename[20];/*���͵��ļ���*/

/*************************************************
  Name:        pthread
  Function:    �߳̽�����Ϣ�������������
  Calls:       ��
  Called By:   recemess
  Input:       messer
  Output:      ��Ӧ�����
  Return:      ��
*************************************************/
void pthread(cuser messer)
{
    FILE *fp;
    char note[1024];
    time_t t;

    switch(messer.cmd)
    {
        case 2:/*��ʾ��ǰ���ߵ�ȫ���û�*/
        {
            printf("%s\n",messer.duff);
            break;
        }
        case 31:/*����ĳ�˷���Ϣ���*/
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
            else/*��ʾ������Ϣ�ɹ������������¼*/
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
        case 32:/*������Ϣ�����ն���ʾ�����������¼*/
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
        case 4:/*Ⱥ����Ϣ*/
        {
            if(strcmp(messer.duff,"band") == 0)
            {
                printf("You have banded!\n");
                break;
            }
            else/*��ʾȺ����Ϣ�����ݲ����������¼*/
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
        case 44:/*�߳�ĳ��*/
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
        case 5:/*��ĳ�˽���*/
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
        case 6:/*�ظ�ĳ�˵ķ���Ϣ��Ȩ��*/
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
        case 71:/*�������ļ����*/
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
        case 72:/*��Ӧ�Ƿ����䷢���ļ�*/
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
        case 73:/*�����ǲ���*/
        {
            /*���Է����ˣ�����ʹ��ļ���ѭ�����Ͱ�*/
            if(strcmp(messer.duff,"yes") == 0)
            {
                printf("Now you can send file to %s!\n",messer.spasswd);

                if((fp = fopen(filename,"r")) == NULL)
                {
                    printf("Opne file error,no such file!\n");
                    break;
                }

                pthread_mutex_lock(&mutex); /* ��ס�߳� */
                while(fgets(messer.duff,1024,fp))
                {
                    if((write(sockfd, &messer, sizeof(cuser))) == -1)
                    {
                        fprintf(stderr, "Write Error:%s\n", strerror(errno));
                        exit(1);
                    }
                }
                pthread_mutex_unlock(&mutex); /* �����߳� */
                fclose(fp);
                printf("Send sucess!\n");
                break;
            }
            if(strcmp(messer.duff,"no") == 0)
            {
                /*�Է���Ȼ���������˳�*/
                printf("%s not want!\n",messer.spasswd);
                break;
            }
 //           break;
        }
        case 74:/*����Է��������ļ�*/
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
  Function:    �߳̽�����Ϣ
  Calls:       pthread
  Called By:   use_wnd,admin_wnd
  Input:       ��
  Output:      ��
  Return:      ��
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
  Function:    �����������ϢҪ���鵱ǰ���ߵ�ȫ���û�
  Calls:       ��
  Called By:   use_wnd
  Input:       ��
  Output:      ��
  Return:      ��
*************************************************/
void check()
{
    cuser cusermess;
    cusermess.cmd = 2;
    bzero(&cusermess.name, sizeof(cusermess.name));/*��name���*/
    strcpy(cusermess.name, msg.name);/*������Ϣ���Ͷ���д����Ϣ�ṹ����*/
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        sendto_someone
  Function:    ��ĳ�˷�����Ϣ
  Calls:       ��
  Called By:   use_wnd
  Input:       ���ͷ������֣�Ҫ���͵Ļ�
  Output:      ��
  Return:      ��
*************************************************/
void sendto_someone()
{
    char say[1024],toname[20];
    cuser cusermess;
    cusermess.cmd=31;
    bzero(&cusermess.name, sizeof(cusermess.name));
    strcpy(cusermess.name, msg.name);/*������Ϣ���Ͷ���д����Ϣ�ṹ����*/
    printf("Enter to someone name:\n");
    scanf("%s",&toname);
    //getchar();
    printf("Enter your say:");
    scanf("%s",&say);
    //printf("%s I say to %s %s\n",ctime(&t),toname,say);
    strcpy(cusermess.duff,say);/*�����͵���Ϣ����duff��*/
    strcpy(cusermess.spasswd,toname);/*�������˵��������浽spasswd����*/
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        sendto_everyone
  Function:    Ⱥ����Ϣ
  Calls:       ��
  Called By:   use_wnd
  Input:       ���͵�����
  Output:      ��
  Return:      ��
*************************************************/
void sendto_everyone()
{
    char say[20];
    cuser cusermess;
    cusermess.cmd = 4;
    bzero(&cusermess.name, sizeof(cusermess.name));
    strcpy(cusermess.name, msg.name);/*������Ϣ���Ͷ���д����Ϣ�ṹ����*/
    printf("Enter your say:");
    scanf("%s",&say);
    strcpy(cusermess.duff,say);/*�����͵���Ϣ����duff��*/
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        look_notes
  Function:    �鿴��ǰ�û��������¼
  Calls:       ��
  Called By:   use_wnd
  Input:       ��
  Output:      �����¼
  Return:      ��
*************************************************/
void look_notes()
{
    char ch[1024];
    int targ=1;
    FILE *fp;
    fp = fopen(msg.name,"r");/*���ļ�*/
    printf("My chat note:\n");
    while(fgets(ch,1024,fp))/*һ��һ�еĶ�ȡ����*/
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
  Function:    ��ĳ�˷����ļ�
  Calls:       ��
  Called By:   use_wnd
  Input:       ���շ������͵��ļ�·��
  Output:      ��
  Return:      ��
*************************************************/
void send_file()
{
    char toname[20];
    cuser cusermess;
    cusermess.cmd=71;
    bzero(&cusermess.name, sizeof(cusermess.name));
    strcpy(cusermess.name, msg.name);/*������Ϣ���Ͷ���д����Ϣ�ṹ����*/
    printf("Enter to someone name:\n");
    scanf("%s",&toname);
    printf("Enter what you to send:\n");
    scanf("%s",&filename);
    //getchar();strcpy(msg.duff,"yes");
//    printf("%s I say to %s %s\n",ctime(&t),toname,say);
 //   strcpy(cusermess.duff,filem);/*�����͵���Ϣ����duff��*/
    strcpy(cusermess.spasswd,toname);/*�������˵��������浽spasswd����*/
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &cusermess, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        use_wnd
  Function:    ��ͨ�û���¼���Ӧ�ý���
  Calls:       check,sendto_someone,sendto_everyone,
               look_notes,send_file
  Called By:   main
  Input:       ��
  Output:      ������
  Return:      ��
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
            /*�鿴��ǰ�����û�*/
            case 2:
            {
                check();
                sleep(1);
                break;
            }
            /*��ĳ�˷�����Ϣ*/
            case 3:
            {
                printf("send to someone!\n");
                sendto_someone();
                sleep(1);
                break;
            }
            /*����Ⱥ����Ϣ*/
            case 4:
            {
                printf("send to everyone!\n");
                sendto_everyone();
                sleep(1);
                break;
            }
            /*�鿴�����¼*/
            case 5:
            {
                look_notes();
                sleep(1);
                break;
            }
            /*�����ļ�*/
            case 6:
            {
                printf("send file to someone!\n");
                send_file();
                sleep(1);
                break;
            }
            /*�˳�*/
            case 9:
            {
                printf("2.###look all user\n3.###send message to someone \n4.###send message to everyone\n5.###look chat notes\n6.###send file\n9.###help\n0.###exit\n");
                break;
            }
            /*���������ļ�*/
            case 10:
            {
                strcpy(msg.duff,"no");
                msg.cmd = 72;
              //  printf("case10name=%s,spassswd=%s\n",msg.name,msg.spasswd);
                pthread_mutex_lock(&mutex); /* ��ס�߳� */
                if((write(sockfd, &msg, sizeof(cuser))) == -1)
                {
                    fprintf(stderr, "Write Error:%s\n", strerror(errno));
                    exit(1);
                }
                pthread_mutex_unlock(&mutex); /* �����߳� */
                break;
            }
            /*�������ļ�*/
            case 11:
            {
                strcpy(msg.duff,"yes");
                msg.cmd = 72;
               // printf("case11name=%s,spassswd=%s\n",msg.name,msg.spasswd);
                pthread_mutex_lock(&mutex); /* ��ס�߳� */
                if((write(sockfd, &msg, sizeof(cuser))) == -1)
                {
                    fprintf(stderr, "Write Error:%s\n", strerror(errno));
                    exit(1);
                }
                pthread_mutex_unlock(&mutex); /* �����߳� */
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
  Function:    �߳�ĳ��
  Calls:       ��
  Called By:   admin_wnd
  Input:       ���߷�����
  Output:      ��
  Return:      ��
*************************************************/
void kickout()
{
    printf("Enter you want to kick name:");
    scanf("%s",msg.spasswd);
    msg.cmd = 44;
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &msg, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        banndone
  Function:    ����ĳ��
  Calls:       ��
  Called By:   admin_wnd
  Input:       ����������
  Output:      ��
  Return:      ��
*************************************************/
void banndone()
{
    printf("Enter you want to bannd name:");
    scanf("%s",msg.spasswd);
    msg.cmd = 5;
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &msg, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        regainone
  Function:    �ظ�ĳ�˵ķ���Ȩ
  Calls:       ��
  Called By:   admin_wnd
  Input:       Ҫ�ָ�����
  Output:      ��
  Return:      ��
*************************************************/
void regainone()
{
    printf("Enter you want to regain name:");
    scanf("%s",msg.spasswd);
    msg.cmd = 6;
    pthread_mutex_lock(&mutex); /* ��ס�߳� */
    if((write(sockfd, &msg, sizeof(cuser))) == -1)
    {
        fprintf(stderr, "Write Error:%s\n", strerror(errno));
        exit(1);
    }
    pthread_mutex_unlock(&mutex); /* �����߳� */
}

/*************************************************
  Name:        admin_wnd
  Function:    �����û���¼��Ĺ���
  Calls:       kickout,banndone,regainone
  Called By:   main
  Input:       ���߷�����
  Output:      ��Ӧ���
  Return:      ��
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
  Function:    ע���û��͵�¼�û�����ʾ���Ĺ���
  Calls:       load,regest
  Called By:   main
  Input:       ���߷�����
  Output:      ��Ӧ���
  Return:      ��
*************************************************/
int main(int argc, char *argv[])
{
	int change;
	struct sockaddr_in server_addr;
	struct hostent *host;
	cuser cusermess,cuser_lod,cuser_reg;

    /* ʹ��hostname��ѯhost ���� */
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

	/* �ͻ�����ʼ���� sockfd������ */
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{
		fprintf(stderr,"Socket Error:%s\a\n",strerror(errno));
		exit(1);
	}

	/* �ͻ�����������˵����� */
	bzero(&server_addr,sizeof(server_addr)); // ��ʼ��,��0
	server_addr.sin_family=AF_INET;          // IPV4
	server_addr.sin_port=htons(portnumber);  // (���������ϵ�short����ת��Ϊ�����ϵ�short����)�˿ں�
	server_addr.sin_addr=*((struct in_addr *)host->h_addr); // IP��ַ

	/* �ͻ��������������� */
	if(connect(sockfd,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Connect Error:%s\a\n",strerror(errno));
		exit(1);
	}
	while(1)
	{
        printf("1.####load  2.####register  0.####exit \n");
        scanf("%d",&change);
        if(change==1)/*��¼ģ��*/
        {
            load(sockfd,cuser_lod);
        }
        else if(change==2)/*ע��ģ��*/
        {
            regest(sockfd,cuser_reg);
        }
        else
        {
            close(sockfd);
            exit(0);
        }

        read(sockfd,&msg,sizeof(cuser));/*��ȡע���¼�Ľ��*/

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
                else if(strcmp(msg.duff,"sucess")==0)/*��ͨ�û���¼�ɹ�*/
                {
                    printf("Load sucess!\n");
                    thr_id = pthread_create(&p_thread, NULL, recemess, NULL);
                    printf("2.###look all user\n3.###send message to someone \n4.###send message to everyone\n5.###look chat notes\n6.###send file\n9.###help\n0.###exit\n");
                    use_wnd();
                    break;
                }
                else if(strcmp(msg.duff,"admin")==0)/*�����û���¼�ɹ�*/
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

	/* ����ͨѶ */
	close(sockfd);
	exit(0);
}
