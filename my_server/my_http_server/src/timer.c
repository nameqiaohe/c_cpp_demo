/*####################################################
# File Name: timer.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 22:52:38
# Last Modified: 2017-05-19 17:46:02
####################################################*/
#include "timer.h"
#include <sys/time.h>

/*
 * 返回值：
 *		1 表示 arg1 小于 arg2
 *		0 表示 arg1 大于 arg2
 * */
int timer_comp(void *ti, void *tj){
    st_timer_node *timeri = (st_timer_node *)ti;
    st_timer_node *timerj = (st_timer_node *)tj;

    return (timeri->key < timerj->key) ? 1 : 0;
}

void st_time_update(){
	// there is only one thread calling st_time_update, no need to lock?
	struct timeval tv;
	int rc;

	rc = gettimeofday(&tv, NULL);
	check(rc == 0, "st_time_update() : gettimeofday() error");

	st_current_msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	debug("st_time_update(), time = %zu", st_current_msec);
}

int st_timer_init(){
	int rc = st_pq_init(&st_timer, timer_comp, PQ_DEFAULT_SIZE);
	check(rc == ST_OK, "st_timer_init() : st_pq_init() error");

	st_time_update();
	return ST_OK;
}

int st_find_timer(){
	st_timer_node *timer_node;
	int time = ST_TIMER_INFINITE;
	int rc;

	while(!st_pq_is_empty(&st_timer)){
		debug("st_find_timer()");

		st_time_update();

		timer_node = (st_timer_node *)st_pq_min(&st_timer);
		check(timer_node != NULL, "st_find_timer() : st_pq_min() error");

		if(timer_node->deleted){
			rc = st_pq_del_min(&st_timer);
			check(rc == 0, "st_find_timer() : st_pq_del_min()");

			free(timer_node);
			continue;
		}

		time = (int)(timer_node->key - st_current_msec);
		debug("st_find_timer() : key = %zu, cur = %zu", timer_node->key, st_current_msec);

		time = (time > 0 ? time : 0);
		break;
	}

	return time;
}

void st_handle_expire_timers(){
	debug("st_handle_expire_timers()");

	st_timer_node *timer_node;
	int rc;

	while(!st_pq_is_empty(&st_timer)){
		debug("st_handle_expire_timers() : size = %zu", st_pq_size(&st_timer));
		st_time_update();

		timer_node = (st_timer_node *)st_pq_min(&st_timer);
		check(timer_node != NULL, "st_handle_expire_timers() : st_pq_min() error");

		if(timer_node->deleted){
			rc = st_pq_del_min(&st_timer);
			check(rc == 0, "st_handle_expire_timers() : st_pq_del_min() error");

			free(timer_node);
			continue;
		}

		if(timer_node->key > st_current_msec){
			return;
		}

		if(timer_node->handler){
			timer_node->handler(timer_node->rq);
		}

		rc = st_pq_del_min(&st_timer);
		check(rc == 0, "st_handle_expire_timers() : st_pq_del_min() error");
		free(timer_node);
	}
}

void st_add_timer(st_http_request_t *rq, size_t timeout, timer_handler_pt handler){
	int rc;
	st_timer_node *timer_node = (st_timer_node *)malloc(sizeof(st_timer_node));
	check(timer_node != NULL, "st_add_timer() : malloc error");

	st_time_update();

	rq->timer = timer_node;
	timer_node->key = st_current_msec + timeout;
	debug("st_add_timer() : key = %zu", timer_node->key);

	timer_node->deleted = 0;
	timer_node->handler = handler;
	timer_node->rq = rq;

	rc = st_pq_insert(&st_timer, timer_node);
	check(rc == 0, "st_add_timer() : st_pq_insert() error");
}

void st_del_timer(st_http_request_t *rq){
	debug("st_del_timer()");

	st_time_update();

	st_timer_node *timer_node = rq->timer;
	check(timer_node != NULL, "st_del_timer() : rq->timer is NULL");

	timer_node->deleted = 1;
}
