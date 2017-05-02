/*####################################################
# File Name: ftp_client.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-27 18:14:57
# Last Modified: 2017-05-03 00:44:05
####################################################*/
#ifndef FTP_CLIENT_H
#define FTP_CLIENT_H

#include "../common/common.h"

int read_reply();

void print_reply(int rc);

int ftp_client_read_cmd(char *buf, int size, struct command *cmd_str);

int ftp_client_get(int sock_fd, int sock_control, char *arg);

int ftp_client_open_conn(int sock_fd);

int ftp_client_list(int sock_fd, int sock_conn);

int ftp_client_send_username_password(struct command *cmd_str);

void ftp_client_login();

#endif
