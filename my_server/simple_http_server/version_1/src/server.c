/*####################################################
# File Name: server.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-19 10:50:01
# Last Modified: 2017-05-19 18:48:00
####################################################*/
#include <stdint.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "util.h"
#include "timer.h"
#include "http.h"
#include "epoll_wrapper.h"
#include "threadpool.h"

#define CONF "server.conf"
#define PROGRAM_VERSION "0.1"

extern struct epoll_event *events;

static const struct option long_options[] = {
	{"help", no_argument, NULL, '?'},
	{"version", no_argument, NULL, 'V'},
	{"conf", required_argument, NULL, 'c'},
	{NULL, 0, NULL,0}
};

static void usage(){
	fprintf(stderr, 
			"server [option]...\n"
			"	-c|--conf <config file> Specify config file. Default ./server.conf.\n"
			"	-?|-h|--help			Show help information.\n"
			"	-V|--version			Display program version.\n");
}

int main(int argc, char *argv[]){
	int rc;
	int opt = 0;
	int options_index = 0;
	char *conf_file = CONF;

	/*
	 * parse argv 
	 * more detail visit: http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	 * */
	if(argc == 1){
		usage();
		return 0;
	}

	while((opt = getopt_long(argc, argv, "Vc:?h", long_options, &options_index)) != EOF){
		switch(opt){
			case 0:
				break;
			case 'c':
				conf_file = optarg;
				break;
			case 'V':
				printf(PROGRAM_VERSION"\n");
				return 0;
			case ':':
			case 'h':
			case '?':
				usage();
				return 0;
		}
	}
	debug("server() : conf_file = %s", conf_file);

	if(optind < argc){
		log_error("server() : non-option ARGV-elements: ");
		while(optind < argc){
			log_error("server() : %s ", argv[optind++]);
		}
		return 0;
	}

	/* read config file */
	char conf_buf[BUF_LEN];
	st_conf_t cf;
	rc = read_conf(conf_file, &cf, conf_buf, BUF_LEN);
	//check(rc == ST_CONF_OK, "server() : read config file error");
	check(rc == ST_CONF_ERROR, "server() : read config file error");

	/*
	 * install signal handle for SIGPIPE
	 * when a fd is closed by remote, writing to this fd will cause system send
	 * SIGPIPE to this process, which exit the program
	 * */
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, NULL)){
		log_error("server() : install signal handler for SIGPIPE failed");
		return 0;
	}

	/* 初始化监听socket */
	int listen_fd;
	struct sockaddr_in client_addr;
	/* 初始化 client_addr、inlen，用于解决 "accept Invalid argument" 错误 */
	socklen_t inlen = 1;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));

	listen_fd = open_listen_fd(cf.port);
	rc = make_socket_nonblocking(listen_fd);
	check(rc == 0, "server() : make_socket_nonblocking()");

	/* 创建 epoll，并且将 listen_fd添加到 epoll中 */
	int epfd = epoll_create_wrapper(0);

	struct epoll_event event;

	st_http_request_t *rq = (st_http_request_t *)malloc(sizeof(st_http_request_t));
	st_init_request_t(rq, listen_fd, epfd, &cf);

	event.data.ptr = (void *)rq;
	event.events = EPOLLIN | EPOLLET;
	epoll_add_wrapper(epfd, listen_fd, &event);

#if 0
	/* 创建线程池 */
	threadpool_t *tp = threadpool_init(cf.thread_num);
	check(tp != NULL, "server() : threadpool_init() error");
#endif
	
	/* 初始化 timer */
	st_timer_init();

	log_info("server() : server started");

	int n;
	int i, fd;
	int time;

	/* epoll wait loop */
	while(1){
		time = st_find_timer();
		debug("server() : wait time = %d", time);

		n = epoll_wait_wrapper(epfd, events, MAX_EVENTS, time);
		st_handle_expire_timers();

		for(i = 0; i < n; ++i){
			st_http_request_t *rq_rq = (st_http_request_t *)events[i].data.ptr;
			fd = rq_rq->fd;

			if(listen_fd == fd){
				int infd;
				while(1){
					infd = accept(listen_fd, (struct sockaddr *)&client_addr, &inlen);
					if(infd < 0){
						if(errno == EAGAIN || errno == EWOULDBLOCK){
							break;
						}else{
							log_error("server() : accept()");
							break;
						}
					}

					rc = make_socket_nonblocking(infd);
					check(rc == 0, "server() : make_socket_nonblocking()");
					log_info("server() : new connection fd %d", infd);

					st_http_request_t *request = (st_http_request_t *)malloc(sizeof(st_http_request_t));
					if(request == NULL){
						log_error("server() : malloc(sizeof(st_http_request_t))");
						break;
					}

					st_init_request_t(request, infd, epfd, &cf);
					event.data.ptr = (void *)request;
					event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

					epoll_add_wrapper(epfd, infd, &event);
					st_add_timer(request, TIMEOUT_DEFAULT, st_http_close_conn);
				}
			}else{
				if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))){
					log_error("server() : epoll error fd : %d", rq_rq->fd);
					close(fd);
					continue;
				}

				log_info("server() : new data from fd %d", fd);

#if 0
				rc = threadpool_add_worker(tp, do_request, events[i].data.ptr);
				check(rc == 0, "server() : threadpool_add_worker()");
#endif

				do_request(events[i].data.ptr);
			}
		}
	}

#if 0
	if(threadpool_destroy(tp, 1) < 0){
		log_error("server() : threadpool_destroy() failed");
	}
#endif

	return 0;
}
