/*####################################################
# File Name: http.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 20:45:01
# Last Modified: 2017-05-19 17:42:29
####################################################*/
#include "http.h"

#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "http_parse.h"
#include "epoll_wrapper.h"
#include "timer.h"

static char *ROOT = NULL;

mime_type_t st_mime[] = {
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rtf", "application/rtf"},
	{".pdf", "application/pdf"},
	{".word", "application/msword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{".css", "text/css"},
	{NULL ,"text/plain"}
};

void do_request(void *ptr){
	st_http_request_t *rq = (st_http_request_t *)ptr;

	int fd = rq->fd;
	int rc, n;

	char filename[SHOT_LINE];
	struct stat stat_buf;//用来描述一个linux文件系统中的文件属性的结构

	ROOT = rq->root;
	char *p_last = NULL;
	size_t remain_size;

	st_del_timer(rq);
	for(;;){
		p_last = &rq->buf[rq->last % MAX_BUF];
		remain_size = MIN(MAX_BUF - (rq->last - rq->pos) - 1, MAX_BUF - rq->last % MAX_BUF);

		n = read(fd, p_last, remain_size);
		check(rq->last - rq->pos < MAX_BUF, "do_request() : request buffer overflow!");

		if(n == 0){//EOF
			log_info("do_request() : read return 0, ready to close fd %d, remain_size = %zu", fd, remain_size);

			rc = st_http_close_conn(rq);
			check(rc == 0, "do_request() : st_http_close_conn() error");
		}else if(n < 0){
			if(errno != EAGAIN){
				log_error("do_request() : read() error, errno = %d", errno);
				rc = st_http_close_conn(rq);
				check(rc == 0, "do_request() : st_http_close_conn() error");
			}
			break;
		}

		rq->last += n;
		check(rq->last - rq->pos < MAX_BUF, "do_request() : request buffer overflow!");

		log_info("do_request() : ready to parse request line");

		rc = st_http_parse_request_line(rq);
		if(rc == ST_AGAIN){
			continue;
		}else if(rc != ST_OK){
			log_error("do_request() : st_http_parse_request_line() error");
			rc = st_http_close_conn(rq);
			check(rc == 0, "do_request() : st_http_close_conn() error");
		}

		log_info("do_request() : method = %.*s", (int)(rq->method_end - rq->request_start), (char *)rq->request_start);
		log_info("do_request() : uri = %.*s", (int)(rq->uri_end - rq->uri_start), (char *)rq->uri_start);

		debug("do_request() : ready to parse request body");

		rc = st_http_parse_request_body(rq);
		if(rc == ST_AGAIN){
			continue;
		}else if(rc != ST_OK){
			log_error("do_request() : st_http_parse_request_body() error");
			rc = st_http_close_conn(rq);
			check(rc == 0, "do_request() : st_http_close_conn() error");
		}

		//handle http header
		st_http_out_t *out = (st_http_out_t *)malloc(sizeof(st_http_out_t));
		if(out == NULL){
			log_error("do_request() : malloc space for st_http_out_t failed");
			exit(1);
		}

		rc = st_init_out_t(out, fd);
		check(rc == ST_OK, "do_request() : st_init_out_t() error");

		parse_uri(rq->uri_start, rq->uri_end - rq->uri_start, filename, NULL);
		if(stat(filename, &stat_buf) < 0){
			do_error(fd, filename, "404", "Not Found", "server can not find the file!");
			continue;
		}

		if(!(S_ISREG(stat_buf.st_mode)) || !(S_IRUSR & stat_buf.st_mode)){
			do_error(fd, filename, "403", "Forbidden", "server can not read the file");
			continue;
		}

		out->mtime = stat_buf.st_mtime;

		st_http_handle_header(rq, out);
		check(list_empty(&(rq->list)) == 1, "do_request() : header list should be empty");

		if(out->status == 0){
			out->status = ST_HTTP_OK;
		}

		server_static(fd, filename, stat_buf.st_size, out);

		if(!out->keep_alive){
			log_info("do_request() : no keep_alive! ready to close");
			free(out);

			rc = st_http_close_conn(rq);
			check(rc == 0, "do_request() : st_http_close_conn() error");
		}

		free(out);
	}

	struct epoll_event event;
	event.data.ptr = ptr;
	event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

	epoll_mod_wrapper(rq->epfd, rq->fd, &event);
	st_add_timer(rq, TIMEOUT_DEFAULT, st_http_close_conn);

	return;
}

void parse_uri(char *uri, int uri_length, char *filename, char *query_string){
	check(uri != NULL, "parse_uri() : uri is NULL");

	uri[uri_length] = '\0';

	int filename_length;
	char *question_mark = strchr(uri, '?');
	if(question_mark){
		filename_length = (int)(question_mark - uri);
		debug("parse_uri() : filename_length = %d", filename_length);
	}else{
		filename_length = uri_length;
		debug("parse_uri() : filename_length = %d", filename_length);
	}

	if(query_string){
	
	}

	strcpy(filename, ROOT);

	//uri_length can not be too long
	if(uri_length > (SHOT_LINE >> 1)){
		log_error("parse_uri() : uri too long : %.*s", uri_length, uri);
		return;
	}

	debug("parse_uri() : before strncat, filename = %s, uri = %.*s, filename_length = %d", filename, filename_length, uri, filename_length);
	strncat(filename, uri, filename_length);

	char *last_comp = strrchr(filename, '/');
	char *last_dot = strrchr(last_comp, '.');
	if(last_dot == NULL && filename[strlen(filename)-1] != '/'){
		strcat(filename, "/");
	}

	if(filename[strlen(filename)-1] == '/'){
		strcat(filename, "index.html");
	}

	log_info("parse_uri() : filename = %s", filename);
	return;
}

void do_error(int fd, char *cause, char *err_num, char *short_msg, char *long_msg){
	char header[MAX_LINE];
	char body[MAX_LINE];

	sprintf(body, "<html><title>Server Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\n", body);
	sprintf(body, "%s%s: %s\n", body, err_num, short_msg);
	sprintf(body, "%s<p>%s: %s\n</p>", body, long_msg, cause);
	sprintf(body, "%s<hr><em>Web Server</em>\n</body></html>", body);

	sprintf(header, "HTTP/1.1 %s %s\r\n", err_num, short_msg);
	sprintf(header, "%sServer: Server\r\n", header);
	sprintf(header, "%sContent-type: text/html\r\n", header);
	sprintf(header, "%sConnection: close\r\n", header);
	sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));

	log_info("header = \n%s\n", header);

	rio_write_n(fd, header, strlen(header));
	rio_write_n(fd, body, strlen(body));

	return;
}

void server_static(int fd, char *filename, size_t filesize, st_http_out_t *out){
	char header[MAX_LINE];
	char buf[MAX_LINE];

	size_t n;
	struct tm tm;

	const char *file_type;
	const char *dot_pos = strrchr(filename, '.');
	file_type = get_file_type(dot_pos);

	sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_msg_from_status_code(out->status));

	if(out->keep_alive){
		sprintf(header, "%sConnection: keep-alive\r\n", header);
		sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, TIMEOUT_DEFAULT);
	}

	if(out->modified){
		sprintf(header, "%sContent-type: %s\r\n", header, file_type);
		sprintf(header, "%sContent-length: %zu\r\n", header, filesize);

		localtime_r(&(out->mtime), &tm);
		strftime(buf, SHOT_LINE, "%a, %d %b %Y %H:%M:%S GMT", &tm);

		sprintf(header, "%sLast-Modified: %s\r\n", header, buf);
	}
	
	sprintf(header, "%sServer: Server\r\n", header);
	sprintf(header, "%s\r\n", header);

	n = (size_t)rio_write_n(fd, header, strlen(header));
	check(n == strlen(header), "server_static() : rio_write_n() error, errno = %d", errno);
	if(n != strlen(header)){
		log_error("server_static() : rio_write_n() error, n != strlen(header)");
		return;
	}

	if(!out->modified){
		return;
	}

	int srcfd = open(filename, O_RDONLY, 0);
	check(srcfd > 2, "server_static() : open() error");

	//can use send file
	char *src_addr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	check(src_addr != (void *)-1, "server_static() : mmap() error");
	close(srcfd);

	n = rio_write_n(fd, src_addr, filesize);
	check(n == filesize, "server_static() : rio_write_n() error");

	munmap(src_addr, filesize);
}

const char *get_file_type(const char *type){
	if(type == NULL){
		return "text/plain";
	}

	int i;
	for(i = 0; st_mime[i].type != NULL; ++i){
		if(strcmp(type, st_mime[i].type) == 0){
			return st_mime[i].value;
		}
	}

	return st_mime[i].value;
}
