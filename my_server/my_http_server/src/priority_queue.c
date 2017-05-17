/*####################################################
# File Name: priority_queue.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 00:34:44
# Last Modified: 2017-05-18 01:18:23
####################################################*/
#include "priority_queue.h"

int pq_init(pq_t *pq, pq_comp_t comp, size_t size){
	pq->queue = (void **)malloc(sizeof(void *) * (size+1));//多分配一个，用于存储 岗哨
	if(pq->queue == NULL){
		log_error("pq_init() : malloc falied");
		return -1;
	}

	pq->n_alloc = 0;
	pq->size = size + 1;
	pq->comp = comp;

	return RC_OK;
}

/*
 * 返回值：
 *		1	队列空
 *		0	队列不为空
 * */
int pq_is_empty(pq_t *pq){
	return (pq->n_alloc == 0) ? 1 : 0;
}

size_t pq_size(pq_t *pq){
	return pq->n_alloc;
}

/*
 * 队列中元素存储从 下标为 1 开始
 * */
void *pq_min(pq_t *pq){
	if(pq_is_empty(pq)){//pq_is_empty()返回值不为 0，表示队列为空
		retuen NULL;
	}

	return pq->queue[1];
}

//重设队列的大小
int resize(pq_t *pq, size_t new_size){
	if(new_size <= pq->n_alloc){
		log_error("resize() : new_size is too small");
		return -1;
	}

	void **new_ptr = (void **)malloc(sizeof(void *) * new_size);
	if(new_ptr == NULL){
		log_error("resize() : malloc falied");
		return -1;
	}

	//将旧的队列中的数据 拷贝至 新申请的内存
	memcpy(new_ptr, pq->queue, sizeof(void *) * (pq->n_alloc + 1));
	free(pq->queue);//释放旧的队列所占内存

	pq->queue = new_ptr;//让 queue指向新的内存
	pq->size = new_size;//改变大小

	return RC_OK;
}

//交换两个节点的值
void exch(pq_t *pq, size_t index_i, size_t index_j){
	void *tmp = pq->queue[index_i];

	pq->queue[index_i] = pq->queue[index_j];
	pa->queue[index_j] = tmp;
}

void swim(pq_t *pq, size_t index){
	while(index > 1 && pq->comp(pq->queue[index], pq->queue[index/2])){
		exch(pq, index, index/2);
		index /= 2;
	}
}

/*
 * cur 表示当前比较值的 索引
 * */
size_t sink(pq_t *pq, size_t cur){
	size_t l_index;//左孩子的索引
	size_t n_alloc = pq->n_alloc;

	while(2*index <= n_alloc){
		l_index = 2 * cur;

		//找到左右孩子中较大的值
		if(l_index < n_alloc && pq->comp(pq->queue[l_index + 1], pq->queue[l_index])){
			l_index++;
		}

		if(!pq->comp(pq->queue[l_index], pq->queue[cur])){
			break;
		}

		exch(pq, l_index, cur);
		cur = l_index;
	}

	return cur;
}

int pq_del_min(pq_t *pq){
	if(pq_is_empty(pq)){//pq_is_empty() 返回值不为 0，表示队列为空
		log_info("pq_del_min() : queue is empty");
		return RC_OK;
	}

	exch(pq, 1, pq->n_alloc);
	pq->n_alloc--;
	sink(pq, 1);
	if(pq->n_alloc > 0 && pq->n_alloc <= (pq->size - 1)/4){
		if(resize(pq, pq->size/2) < 0){
			return -1;
		}
	}

	return RC_OK;
}

int pq_insert(pq_t *pq, void *item){
	if(pq->n_alloc + 1 == pq->size){
		if(resize(pq, pq->size * 2) < 0){
			return -1;
		}
	}

	pq->queue[++pq->n_alloc] = item;
	swim(pq, pq->n_alloc);

	return RC_OK;
}

int pq_sink(pq_t *pq, size_t cur){
	return sink(pq, cur);
}
