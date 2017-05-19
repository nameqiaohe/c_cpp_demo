/*####################################################
# File Name: priority_queue.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 00:34:44
# Last Modified: 2017-05-19 17:44:15
####################################################*/
#include "priority_queue.h"

int st_pq_init(st_pq_t *pq, st_pq_comp_t comp, size_t size){
	pq->queue = (void **)malloc(sizeof(void *) * (size+1));//多分配一个，用于存储 岗哨
	if(pq->queue == NULL){
		log_error("pq_init() : malloc falied");
		return -1;
	}

	pq->n_alloc = 0;
	pq->size = size + 1;
	pq->comp = comp;

	return ST_OK;
}

/*
 * 返回值：
 *		1	队列空
 *		0	队列不为空
 * */
int st_pq_is_empty(st_pq_t *pq){
	return (pq->n_alloc == 0) ? 1 : 0;
}

size_t st_pq_size(st_pq_t *pq){
	return pq->n_alloc;
}

/*
 * 队列中元素存储从 下标为 1 开始
 * */
void *st_pq_min(st_pq_t *pq){
	if(st_pq_is_empty(pq)){//pq_is_empty()返回值不为 0，表示队列为空
		return NULL;
	}

	return pq->queue[1];
}

//重设队列的大小
int resize(st_pq_t *pq, size_t new_size){
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

	return ST_OK;
}

//交换两个节点的值
void exch(st_pq_t *pq, size_t index_i, size_t index_j){
	void *tmp = pq->queue[index_i];

	pq->queue[index_i] = pq->queue[index_j];
	pq->queue[index_j] = tmp;
}

void swim(st_pq_t *pq, size_t index){
	//comp函数 若返回值不等于 0,表示 参数1 小于 参数2，具体见timer.c
	while(index > 1 && pq->comp(pq->queue[index], pq->queue[index/2])){//index/2 表示父节点索引
		exch(pq, index, index/2);
		index /= 2;
	}
}

/*
 * cur 表示当前比较值的 索引
 * */
size_t sink(st_pq_t *pq, size_t cur){
	size_t l_index;//左孩子的索引
	size_t n_alloc = pq->n_alloc;

	while(2*cur <= n_alloc){
		l_index = 2 * cur;

		//找到左右孩子中较大的值
		//comp函数 若返回值不等于 0,表示 参数1 小于 参数2，具体见timer.c
		if(l_index < n_alloc && pq->comp(pq->queue[l_index + 1], pq->queue[l_index])){
			l_index++;
		}

		// 拿 当前节点的值 和 左右孩子中较大的值比较
		//comp函数 若返回值不等于 0,表示 参数1 小于 参数2，具体见timer.c
		if(!pq->comp(pq->queue[l_index], pq->queue[cur])){
			break;
		}

		exch(pq, l_index, cur);
		cur = l_index;
	}

	return cur;
}

int st_pq_del_min(st_pq_t *pq){
	if(st_pq_is_empty(pq)){//st_pq_is_empty() 返回值不为 0，表示队列为空
		log_info("pq_del_min() : queue is empty");
		return ST_OK;
	}

	exch(pq, 1, pq->n_alloc);
	pq->n_alloc--;
	sink(pq, 1);
	//这个分支是什么意思？？？
	if(pq->n_alloc > 0 && pq->n_alloc <= (pq->size - 1)/4){
		if(resize(pq, pq->size/2) < 0){
			return -1;
		}
	}

	return ST_OK;
}

int st_pq_insert(st_pq_t *pq, void *item){
	if(pq->n_alloc + 1 == pq->size){
		if(resize(pq, pq->size * 2) < 0){
			return -1;
		}
	}

	pq->queue[++pq->n_alloc] = item;
	swim(pq, pq->n_alloc);

	return ST_OK;
}

int st_pq_sink(st_pq_t *pq, size_t cur){
	return sink(pq, cur);
}
