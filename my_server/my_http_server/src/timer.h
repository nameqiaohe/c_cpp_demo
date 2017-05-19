/*####################################################
# File Name: timer.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 22:37:15
# Last Modified: 2017-05-19 17:46:43
####################################################*/
#ifndef ST_TIMER_H
#define ST_TIMER_H

#include "priority_queue.h"
#include "http_request.h"

#define ST_TIMER_INFINITE -1
#define TIMEOUT_DEFAULT 500	// ms

//timer_handler_pt 中的 pt指 process thread
typedef int (*timer_handler_pt)(st_http_request_t *rq);

typedef struct st_timer_node{
	size_t key;
	int deleted;
	timer_handler_pt handler;
	st_http_request_t *rq;
}st_timer_node;

int st_timer_init();
int st_find_timer();

void st_handle_expire_timers();

st_pq_t st_timer;
size_t st_current_msec;

void st_add_timer(st_http_request_t *rq, size_t timeout, timer_handler_pt handler);

void st_del_timer(st_http_request_t *rq);

int timer_comp(void *ti, void *tj);

void st_time_update();

#endif
