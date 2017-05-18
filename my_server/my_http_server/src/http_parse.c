/*####################################################
# File Name: http_parse.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-19 00:50:35
# Last Modified: 2017-05-19 00:57:55
####################################################*/
#include "http_parse.h"
#include "http.h"
#include "error.h"

int st_http_parse_request_line(st_http_request_t *rq){
	u_char ch, *p, *m;
	size_t pos;

	enum{
		st_start = 0,
		st_method,
		st_spaces_before_uri,
		st_after_slash_in_uri,
		st_http,
		st_http_H,
		st_http_HT,
		st_http_HTT,
		st_http_HTTP,
		st_first_major_digit,
		st_major_digit,
		st_first_minor_digit,
		st_minor_digit,
		st_spaces_after_digit,
		st_almost_done
	}state;

	state = rq->state;

	for(pos = rq->pos; pos < rq->last; ++pos){
		p = (u_char *)&rq->buf[pos % MAX_BUF];
		ch = *p;

		switch(state){
			/* HTTP methods: GET, HEAD, POST */
			case st_start:
				rq->request_start = p;
				if(ch == CR || ch == LF){
					break;
				}
		}
	}
}

