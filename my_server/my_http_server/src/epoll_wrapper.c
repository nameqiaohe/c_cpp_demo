/*####################################################
# File Name: epoll_wrapper.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 20:44:55
# Last Modified: 2017-05-16 21:16:42
####################################################*/
#include "epoll_wrapper.h"
#include "dbg.h"

int epoll_create_wrapper(int flags){
	int fd = epoll_create1(flags);
	check(fd > 0, "epoll_create_wrapper : epoll_create1()");

	events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAX_EVENTS);
	check(events != NULL, "epoll_create_wrapper : malloc");

	return fd;
}

void epoll_add_wrapper(int epfd, int fd, struct epoll_event *event){
	int rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);// fd 表示需要监听的fd; event 表示需要监听的事件
	check(rc == 0, "epoll_add_wrapper : epoll_ctl()");

	return;
}

void epoll_mod_wrapper(int epfd, int fd, struct epoll_event *event){
	int rc = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);// fd 表示需要监听的fd; event 表示需要监听的事件
	check(rc == 0, "epoll_mod_wrapper : epoll_ctl()");

	return;	
}

void epoll_del_wrapper(int epfd, int fd, struct epoll_event *event){
	int rc = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event);// fd 表示需要监听的fd; event 表示需要监听的事件
	check(rc == 0, "epoll_del_wrapper : epoll_ctl()");

	return;	
}

int epoll_wait_wrapper(int epfd, struct epoll_event *events, int max_events, int timeout){
	int rc = epoll_wait(epfd, events, max_events, timeout);
	check(rc >= 0, "epoll_wait_wrapper : epoll_wait()");

	return rc;	
}
