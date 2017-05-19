/*####################################################
# File Name: http_request.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 20:05:28
# Last Modified: 2017-05-19 17:13:28
####################################################*/

/* why define _GNU_SOURCE? http://stackoverflow.com/questions/15334558/compiler-gets-warnings-when-using-strptime-function-ci */
#ifndef _GUN_SOURCE
#define _GUN_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#include "http_request.h"
#include "error.h"
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "dbg.h"

st_http_header_handler_t st_http_headers_in[] = {
	{"Host", st_http_process_ignore},
	{"Connection", st_http_process_connection},
	{"If-Modified-Since", st_http_process_if_modified_since},
	{"", st_http_process_ignore}
};

int st_init_request_t(st_http_request_t *rq, int fd, int epfd, st_conf_t *cf){
	rq->fd = fd;
	rq->epfd = epfd;
	rq->pos = rq->last = 0;

	rq->state = 0;
	rq->root = cf->root;

	INIT_LIST_HEAD(&(rq->list));

	return ST_OK;
}

int st_free_request_t(st_http_request_t *rq){
	//TODO
	(void)rq;

	return ST_OK;
}

int st_init_out_t(st_http_out_t *out, int fd){
	out->fd = fd;
	out->keep_alive = 0;
	out->modified = 1;//表示未修改
	out->status = 0;

	return ST_OK;
}

int st_free_out_t(st_http_out_t *out){
	//TODO
	(void)out;

	return ST_OK;
}

void st_http_handle_header(st_http_request_t *rq, st_http_out_t *out){
	list_head *pos;
	st_http_header_t *header;
	st_http_header_handler_t *header_in;
	int len;

	list_for_each(pos, &(rq->list)){
		header = list_entry(pos, st_http_header_t, list);// pos用来保存 list的地址( list 是 st_http_header_t的成员 )，便于计算 st_http_header_t的地址

		for(header_in = st_http_headers_in; strlen(header_in->name) > 0; header_in++){
			//若 header->key_start 中的字符串 和 header_in中的字符串一样
			if(strncmp(header->key_start, header_in->name, header->key_end - header->key_start) == 0){
				len = header->value_end - header->value_start;

				(*(header_in->handler))(rq, out, header->value_start, len);
				break;
			}
		}

		list_del(pos);
		free(header);
	}
}

int st_http_close_conn(st_http_request_t *rq){
	// NOTICE: closing a file descriptor will cause it to be removed from all epoll sets automatically
	// http://stackoverflow.com/questions/8707601/is-it-necessary-to-deregister-a-socket-from-epoll-before-closing-it
	close(rq->fd);;
	free(rq);
	
	return ST_OK;
}

int st_http_process_ignore(st_http_request_t *rq, st_http_out_t *out, char *data, int len){
	(void)rq;
	(void)out;
	(void)data;
	(void)len;

	return ST_OK;
}

int st_http_process_connection(st_http_request_t *rq, st_http_out_t *out, char *data, int len){
	(void)rq;

	if(strncasecmp("keep-alive", data, len) == 0){
		out->keep_alive = 1;
	}

	return ST_OK;
}

int st_http_process_if_modified_since(st_http_request_t *rq, st_http_out_t *out, char *data, int len){
	(void)rq;
	(void)len;

	struct tm tm;
	if(strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm) == (char *)NULL){
		return ST_OK;
	}

	time_t client_time = mktime(&tm);
	double time_diff = difftime(out->mtime, client_time);
	if(fabs(time_diff) < 1e-6){
		log_info("content not modified client_time = %d, mtime = %d\n", (int)client_time, (int)out->mtime);
		out->modified = 0;
		out->status = ST_HTTP_NOT_MODIFIED;
	}

	return ST_OK;
}

const char *get_msg_from_status_code(int status_code){
	/* for code to msg mapping, please check: 
	 * http://users.polytech.unice.fr/~buffa/cours/internet/POLYS/servlets/Servlet-Tutorial-Response-Status-Line.html
	 * */
	if(status_code == ST_HTTP_OK){
		return "OK";
	}

	if(status_code == ST_HTTP_NOT_MODIFIED){
		return "Not Modified";
	}

	if(status_code == ST_HTTP_NOT_FOUND){
		return "Not Found";
	}

	return "Unknown";
}
