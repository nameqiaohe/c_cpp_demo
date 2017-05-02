/*####################################################
# File Name: ftp_serve.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 16:27:15
# Last Modified: 2017-04-27 16:32:21
####################################################*/
#ifndef FTP_SERVE_H
#define FTP_SERVE_H

#include "../common/common.h"

void ftp_serve_retr(int sock_control, int sock_data, char *filename);

int ftp_serve_list(int sock_data, int sock_control);

int ftp_serve_start_conn(int sock_control);

int ftp_serve_check_user(char *username, char *passwd);

int ftp_serve_login(int sock_control);

int ftp_serve_recv_cmd(int sock_control, char *cmd, char *arg);

void ftp_serve_process(int sock_control);

#endif
