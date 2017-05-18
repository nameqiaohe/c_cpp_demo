/*####################################################
# File Name: http_parse.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-19 00:47:40
# Last Modified: 2017-05-19 00:50:29
####################################################*/
#ifndef HTTP_PARSE_H
#define HTTP_PARSE_H

#include "http_request.h"

#define CR '\r'
#define LF '\n'
#define CRLFCRLF '\r\n\r\n'

int st_http_parse_request_line(st_http_request_t *rq);

int st_http_parse_request_body(st_http_request_t *rq);

#endif
