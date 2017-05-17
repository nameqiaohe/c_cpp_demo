/*####################################################
# File Name: priority_queue.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 00:24:22
# Last Modified: 2017-05-18 01:18:31
####################################################*/
#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "dbg.h"
#include "error.h"

#define PQ_DEFAULT_SIZE 10

typedef int (*pq_comp_t)(void *pi, void *pj);

typedef struct{
	void **queue;
	size_t n_alloc;
	size_t size;
	pq_comp_t comp;
}pq_t;

int pq_init(pq_t *pq, pq_comp_t comp, size_t size);

int pq_is_empty(pq_t *pq);

size_t pq_size(pq_t *pq);

void *pq_min(pq_t *pq);

int pq_del_min(pq_t *pq);

//优先队列 插入操作，调用 swim()
int pq_insert(pq_t *pq, void *item);

//优先队列向下调整--删除
int pq_sink(pq_t *pq, size_t cur);

//重设大小
int resize(pq_t *pq, size_t new_size);

//交换
void exch(pq_t *pq, size_t index_i, size_t index_j);

//向上调整
void swim(pq_t *pq, size_t index);

//向下调整
size_t sink(pq_t *pq, size_t cur);

#endif
