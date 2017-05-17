/*####################################################
# File Name: threadpool.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-17 09:38:00
# Last Modified: 2017-05-17 10:07:29
####################################################*/
#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "dbg.h"

#define THREAD_NUM 8

//要执行的任务链表
typedef struct task_t{
	void (*func)(void *);//回调函数，任务运行时会调用此函数
	void *arg;// 回调函数的参数
	struct task_t *next;//指向下一个任务
}task_t;

typedef struct threadpool_t{
	pthread_mutex_t lock;//互斥锁
	pthread_cond_t cond;//条件变量

	pthread_t *threads_id;//保存线程ID，需手动分配内存

	task_t *head;//任务链表/队列

	int thread_count;
	int queue_size;//当前任务链表中的任务数目
	int shutdown;//是否销毁线程池
	int started;
}threadpool_t;

typedef enum{
	tp_invalid = -1,
	tp_lock_fail = -2,
	tp_already_shutdown = -3,
	tp_cond_broadcast = -4,
	tp_thread_fail = -5,
}threadpool_error_t;

typedef enum{
	immediate_shutdown = 1,
	graceful_shutdown = 2,
}threadpool_shutdown_t;

//初始化线程池
threadpool_t *threadpool_init(int thread_num);

//向线程池的任务链表中添加一个任务
int threadpool_add_worker(threadpool_t *pool, void (*func)(void *), void *arg);

//销毁线程池
int threadpool_destory(threadpool_t *pool, int shutdown_mode);

int threadpool_free(threadpool_t *pool);

void *threadpool_worker(void *arg);

#ifdef __cplusplus
}

#endif
