/*####################################################
# File Name: http.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 20:37:42
# Last Modified: 2017-05-18 23:20:47
####################################################*/
#ifndef HTTP_H
#define HTTP_H

#include <strings.h>
#include <stdint.h>

#include "rio.h"
#include "list_wrapper.h"
#include "dbg.h"
#include "util.h"
#include "http_request.h"

#define MAX_LINE 8192
#define SHOT_LINE 512

#define zv_str3_cmp(m, c0, c1, c2, c3)                                       \
	    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)
#define zv_str3Ocmp(m, c0, c1, c2, c3)                                       \
	    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

#define zv_str4cmp(m, c0, c1, c2, c3)                                        \
	    *(uint32_t *) m == ((c3 << 24) | (c2 << 16) | (c1 << 8) | c0)

typedef struct mime_type_t{
	const char *type;
	const char *value;
}mime_type_t;

void do_request(void *in_fd);

const char *get_file_type(const char *type);

void parse_uri(char *uri, int length, char *file_name, char *query_string);

void do_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg);

void server_static(int fd, char *file_name, size_t file_size, st_http_out_t *out);

#endif
