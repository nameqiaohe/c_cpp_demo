/*####################################################
# File Name: priority_queue.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 00:24:22
# Last Modified: 2017-05-18 22:44:50
####################################################*/
#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "dbg.h"
#include "error.h"

#define PQ_DEFAULT_SIZE 10

typedef int (*st_pq_comp_t)(void *pi, void *pj);

typedef struct{
	void **queue;
	size_t n_alloc;
	size_t size;
	st_pq_comp_t comp;
}st_pq_t;

int st_pq_init(st_pq_t *pq, st_pq_comp_t comp, size_t size);

int st_pq_is_empty(st_pq_t *pq);

size_t st_pq_size(st_pq_t *pq);

void *st_pq_min(st_pq_t *pq);

int st_pq_del_min(st_pq_t *pq);

//优先队列 插入操作，调用 swim()
int st_pq_insert(st_pq_t *pq, void *item);

//优先队列向下调整--删除
int st_pq_sink(st_pq_t *pq, size_t cur);

//重设大小
int resize(st_pq_t *pq, size_t new_size);

//交换
void exch(st_pq_t *pq, size_t index_i, size_t index_j);

//向上调整
void swim(st_pq_t *pq, size_t index);

//向下调整
size_t sink(st_pq_t *pq, size_t cur);

#endif
