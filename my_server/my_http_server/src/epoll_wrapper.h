/*####################################################
# File Name: epoll_wrapper.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 20:38:56
# Last Modified: 2017-05-16 20:46:38
####################################################*/
#ifndef EPOLL_WRAPPER_H
#define EPOLL_WRAPPER_H

#include <sys/epoll.h>

#define MAX_EVENTS 1024

struct epoll_event *events;

int epoll_create_wrapper(int flags);

void epoll_add_wrapper(int epfd, int fd, struct epoll_event *event);

void epoll_mod_wrapper(int epfd, int fd, struct epoll_event *event);

void epoll_del_wrapper(int epfd, int fd, struct epoll_event *event);

void epoll_wait_wrapper(int epfd, struct epoll_event *event, int max_events, int timeout);

#endif
