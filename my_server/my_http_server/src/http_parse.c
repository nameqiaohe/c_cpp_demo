/*####################################################
# File Name: http_parse.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-19 00:50:35
# Last Modified: 2017-05-19 13:10:04
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

				if((ch < 'A' || ch > 'Z') && ch != '_'){
					return ST_HTTP_PARSE_INVALID_METHOD;
				}

				state = st_method;
				break;
			case st_method:
				if(ch == ' '){
					rq->method_end = p;
					m = rq->request_start;

					switch(p - m){
						case 3:
							if(st_str3_cmp(m, 'G', 'E', 'T', ' ')){
								rq->method = ST_HTTP_GET;
								break;
							}
							break;
						case 4:
							if(st_str3Ocmp(m, 'P', 'O', 'S', 'T')){
								rq->method = ST_HTTP_POST;
								break;
							}
							
							if(st_str4cmp(m, 'H', 'E', 'A', 'D')){
								rq->method = ST_HTTP_HEAD;
								break;
							}

							break;
						default:
							rq->method = ST_HTTP_UNKNOWN;
							break;
					}
					state = st_spaces_before_uri;
					break;
				}
				if((ch < 'A' || ch > 'Z') && ch != '_'){
					return ST_HTTP_PARSE_INVALID_METHOD;
				}

				break;
			case st_spaces_before_uri:
				if(ch == '/'){
					rq->uri_start = p;
					state = st_after_slash_in_uri;
					break;
				}

				switch(ch){
					case ' ':
						break;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;

			case st_after_slash_in_uri:
				switch(ch){
					case ' ':
						rq->uri_end = p;
						state = st_http;
						break;
					default:
						break;
				}
				break;
		
			case st_http:
				switch(ch){
					case ' ':
						break;
					case 'H':
						state = st_http_H;
						break;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case st_http_H:
				switch(ch){
					case 'T':
						state = st_http_HT;
						break;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case st_http_HT:
				switch(ch){
					case 'T':
						state = st_http_HTT;
						break;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case st_http_HTT:
				switch(ch){
					case 'P':
						state = st_http_HTTP;
						break;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			case st_http_HTTP:
				switch(ch){
					case '/':
						state = st_first_major_digit;
						break;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			/* first digit of major HTTP version */
			case st_first_major_digit:
				if(ch < '1' || ch > '9'){
					return ST_HTTP_PARSE_INVALID_REQUEST;
				}

				rq->http_major = ch - '0';
				state = st_major_digit;
				break;
			/* major HTTP version or dot */
			case st_major_digit:
				if(ch == '.'){
					state = st_first_minor_digit;
					break;
				}

				if(ch < '0' || ch > '9'){
					return ST_HTTP_PARSE_INVALID_REQUEST;
				}

				rq->http_major = rq->http_major * 10 + ch - '0';
				break;
			/* first digit of minor HTTP version */
			case st_first_minor_digit:
				if(ch < '0' || ch > '9'){
					return ST_HTTP_PARSE_INVALID_REQUEST;
				}

				rq->http_minor = ch - '0';
				state = st_minor_digit;
				break;
			case st_minor_digit:
				if(ch == CR){
					state = st_almost_done;
					break;
				}

				if(ch == LF){
					rq->pos = pos + 1;
					if(rq->request_end == NULL){
						rq->request_end = p;
					}

					rq->state = st_start;

					return ST_OK;
				}

				if(ch == ' '){
					state = st_spaces_after_digit;
					break;
				}

				if(ch < '0' || ch > '9'){
					return ST_HTTP_PARSE_INVALID_REQUEST;
				}

				rq->http_minor = rq->http_minor * 10 + ch - '0';
				break;
			case st_spaces_after_digit:
				switch(ch){
					case ' ':
						break;
					case CR:
						state = st_almost_done;
						break;
					case LF:
						rq->pos = pos + 1;
						if(rq->request_end == NULL){
						    rq->request_end = p;
						}          
						           
						rq->state = st_start;
						           
						return ST_OK;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
				break;
			/* end of request line */
			case st_almost_done:
				rq->request_end = p - 1;
				switch(ch){
					case LF:
						rq->pos = pos + 1;
						if(rq->request_end == NULL){
						    rq->request_end = p;
						}          
						           
						rq->state = st_start;
						           
						return ST_OK;
					default:
						return ST_HTTP_PARSE_INVALID_REQUEST;
				}
		}
	}

	rq->pos = pos;
	rq->state = state;

	return ST_AGAIN;
}

int st_http_parse_request_body(st_http_request_t *rq){
	u_char ch, *p;
	size_t pos;

	enum{
		st_start = 0,
		st_key,
		st_spaces_before_colon,
		st_spaces_after_colon,
		st_value,
		st_cr,
		st_crlf,
		st_crlfcr
	}state;

	state = rq->state;
	check(state == 0, "st_http_parse_request_body() : state should be 0");

	st_http_header_t *header;
	for(pos = rq->pos; pos < rq->last; ++pos){
		p = (u_char *)&rq->buf[pos % MAX_BUF];
		ch = *p;

		switch(state){
			case st_start:
				if(ch == CR || ch == LF){
					break;
				}

				rq->cur_header_key_start = p;
				state = st_key;
				break;
			case st_key:
				if(ch == ' '){
					rq->cur_header_key_end = p;
					state = st_spaces_before_colon;
					break;
				}

				if(ch == ':'){
					rq->cur_header_key_end = p;
					state = st_spaces_after_colon;
					break;
				}
				break;
			case st_spaces_before_colon:
				if(ch == ' '){
					break;
				}else if(ch == ':'){
					state = st_spaces_after_colon;
					break;
				}else{
					return ST_HTTP_PARSE_INVALID_HEADER;
				}
			case st_spaces_after_colon:
				if(ch == ' '){
					break;
				}

				state = st_value;
				rq->cur_header_value_start = p;
				break;
			case st_value:
				if(ch == CR){
					rq->cur_header_value_end = p;
					state = st_cr;
				}

				if(ch == LF){
					rq->cur_header_value_end = p;
					state = st_crlf;
				}
				break;
			case st_cr:
				if(ch == LF){
					state = st_crlf;
					/* save the current http header */
					header = (st_http_header_t *)malloc(sizeof(st_http_header_t));
					header->key_start = rq->cur_header_key_start;
					header->key_end = rq->cur_header_key_end;
					header->value_start = rq->cur_header_value_start;
					header->value_end = rq->cur_header_value_end;

					list_add(&(header->list), &(rq->list));

					break;
				}else{
					return ST_HTTP_PARSE_INVALID_HEADER;
				}
			case st_crlf:
				if(ch == CR){
					state = st_crlfcr;
				}else{
					rq->cur_header_key_start = p;
					state = st_key;
				}
				break;
			case st_crlfcr:
				switch(ch){
					case LF:
						rq->pos = pos + 1;
						rq->state = st_start;

						return ST_OK;
					default:
						return ST_HTTP_PARSE_INVALID_HEADER;
				}
				break;
		}
	}

	rq->pos = pos;
	rq->state = state;

	return ST_AGAIN;
}
