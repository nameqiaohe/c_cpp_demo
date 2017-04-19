/*####################################################
# File Name: server_header.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-09 15:04:14
# Last Modified: 2017-04-10 19:51:38
####################################################*/
#ifndef SERVER_HEADER_H
#define SERVER_HEADER_H

#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include "method.h"           /*  our own functions  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*  Global constants  */
#define ECHO_PORT          (8888)
#define MAX_LINE           (1000)

#endif
