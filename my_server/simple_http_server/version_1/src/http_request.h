/*####################################################
# File Name: http_request.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 19:39:15
# Last Modified: 2017-05-19 17:43:03
####################################################*/
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <time.h>
#include "list_wrapper.h"
#include "util.h"

#define ST_AGAIN EAGAIN

#define ST_HTTP_PARSE_INVALID_METHOD 10
#define ST_HTTP_PARSE_INVALID_REQUEST 11
#define ST_HTTP_PARSE_INVALID_HEADER 12

#define ST_HTTP_UNKNOWN 0x0001
#define ST_HTTP_GET 0x0002
#define ST_HTTP_HEAD 0x0003
#define ST_HTTP_POST 0x0004
#define ST_HTTP_UNKNOWN 0x0001

#define ST_HTTP_OK 200
#define ST_HTTP_NOT_MODIFIED 304
#define ST_HTTP_NOT_FOUND 404

//#define MAX_BUF 8192
#define MAX_BUF 8192

typedef struct st_http_request_t{
	void *root;
	int fd;
	int epfd;
	char buf[MAX_BUF];//环形缓冲区

	size_t pos, last;

	int state;

	void *request_start;
	void *method_end;
	int method;
	void *uri_start;
	void *uri_end;
	void *path_start;
	void *path_end;
	void *query_start;
	void *query_end;
	int http_major;
	int http_minor;
	void *request_end;

	struct list_head list;

	void *cur_header_key_start;
	void *cur_header_key_end;

	void *cur_header_value_start;
	void *cur_header_value_end;

	void *timer;
}st_http_request_t;

typedef struct{
	int fd;
	int keep_alive;
	time_t mtime;//文件修改时间
	int modified;//文件是否被修改

	int status;
}st_http_out_t;

typedef struct st_http_header_t{
	void *key_start;
	void *key_end;

	void *value_start;
	void *value_end;

	list_head list;
}st_http_header_t;

//st_http_header_handler_pt中的 pt是否代表 process thread
typedef int (*st_http_header_handler_pt)(st_http_request_t *rq, st_http_out_t *out, char *data, int len);

typedef struct{
	char *name;
	st_http_header_handler_pt handler;
}st_http_header_handler_t;

void st_http_handle_header(st_http_request_t *rq, st_http_out_t *out);

int st_http_close_conn(st_http_request_t *rq);

int st_init_request_t(st_http_request_t *rq, int fd, int epfd, st_conf_t *cf);
int st_free_request_t(st_http_request_t *rq);

int st_init_out_t(st_http_out_t *out, int fd);
int st_free_out_t(st_http_out_t *out);

const char *get_msg_from_status_code(int status_code);

extern st_http_header_handler_t st_http_header_in[];

//这三个函数 对应于上边的 st_http_header_handler_pt指针
int st_http_process_ignore(st_http_request_t *rq, st_http_out_t *out, char *data, int len);
int st_http_process_connection(st_http_request_t *rq, st_http_out_t *out, char *data, int len);
int st_http_process_if_modified_since(st_http_request_t *rq, st_http_out_t *out, char *data, int len);


#endif
