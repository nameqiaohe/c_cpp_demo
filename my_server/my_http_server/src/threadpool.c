/*####################################################
# File Name: threadpool.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-17 10:07:39
# Last Modified: 2017-05-17 12:10:53
####################################################*/
#include "threadpool.h"

threadpool_t *threadpool_init(int thread_num){
	if(thread_num <= 0){
		log_error("threadpool_init() : the thread_num must greater than 0");
		return NULL;
	}

	threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
	if(pool == NULL){
		log_error("threadpool_init() : malloc threadpool failed");
		return NULL;
	}

	//初始化线程池的属性
	pool->thread_count = 0;
	pool->queue_size = 0;
	pool->shutdown = 0;
	pool->started = 0;

	//为 线程ID、任务队列头 申请存放的内存
	pool->threads_id = (pthread_t *)malloc(sizeof(pthread_t) * thread_num));
	pool->head = (task_t *)malloc(sizeof(task_t));
	if(pool->threads_id == NULL || pool->head == NULL){//若申请失败，需释放之前申请的资源
		if(pool){
			threadpool_free(pool);
		}
		return NULL;
	}

	//初始化任务队列
	pool->head->func = NULL;
	pool->head->arg = NULL;
	pool->head->next = NULL;

	//初始化 互斥锁
	int rc = pthread_mutex_init(&(pool->lock), NULL);
	if(rc != 0){
		log_error("threadpool_init() : pthread_mutex_init() failed");
		if(pool){
			threadpool_free(pool);
		}
		return NULL;
	}

	//初始化 条件变量
	rc = pthread_cond_init(&(pool->cond), NULL);
	if(rc != 0){
		pthread_mutex_destory(&(pool->lock));//若 条件变量初始化失败，需释放 互斥锁
		log_error("threadpool_init() : pthread_cond_init() failed");
		if(pool){
			threadpool_free(pool);
		}
		return NULL;
	}

	int i;
	for(i = 0; i < thread_num; ++i){
		rc = pthread_create(&(pool->threads_id[i]), NULL, threadpool_worker, (void *)pool);
		if(rc != 0){
			threadpool_destory(pool, 0);
			return NULL;
		}

		log_info("threadpool_init() : 0x%x started", (uint32_t)pool->threads_id[i]);

		//创建线程成功，需将当前线程数量自增，当前启动的线程数量自增
		pool->thread_count++;
		pool->started++;
	}
	
	return pool;
}

int threadpool_add_worker(threadpool_t *pool, void (*func)(void *), void *arg){
	int rc, err = 0;

	if(pool == NULL || func == NULL){
		log_error("threadpool_add_worker() : pool == NULL or func == NULL");
		return -1;
	}

	rc = pthread_mutex_lock(&(pool->lock));
	if(rc != 0){
		log_error("threadpool_add_worker() : pthread_mutex_lock() failed");
		return -1;
	}

	// shutdown不等于0，其值要么是 1，要么是 2，这两个值都是表示已关闭
	if(pool->shutdown){
		err = tp_already_shutdown;
		rc = pthread_mutex_unlock(&(pool->lock));
		if(rc != 0){
			log_error("threadpool_add_worker() : pthread_mutex_unlock() failed");
			return -1;
		}
	}

	task_t *task = (task_t *)malloc(sizeof(task_t));
	if(task == NULL){
		log_error("threadpool_add_worker() : malloc space for task failed");
		rc = pthread_mutex_unlock(&(pool->lock));
		if(rc != 0){
			log_error("threadpool_add_worker() : pthread_mutex_unlock() failed");
			return -1;
		}
	}

	//给线程池的任务队列中新添加的任务 赋值相关属性
	//任务链表中的头节点是一个空节点，不保存任务
	task->func = func;
	task->arg = arg;
	task->next = pool->head->next;
	pool->head->next = task;

	//添加任务成功之后，任务数量自增
	pool->queue_size++;

	//新添加任务之后，需要唤醒一个等待线程
	rc = pthread_cond_signal(&(pool->cond));
	check(rc == 0, "threadpool_add_worker() : pthread_cond_signal()");

	return err;
}

int threadpool_free(threadpool_t *pool){
	if(pool == NULL || pool->started >= 0){
		return -1;
	}

	if(pool->threads_id){
		free(pool->threads_id);
	}

	task_t *old;
	while(pool->head->next){//任务链表中的头节点是一个空节点，不保存任务
		old = pool->head->next;
		pool->head->next = pool->head->next->next;
		free(old);
	}

	return 0;
}

int threadpool_destory(threadpool_t *pool, int shutdown_mode){
	int err = 0;

	if(pool == NULL){
		log_error("threadpool_destory() : pool == NULL");
		return tp_invalid;
	}

	int rc = pthread_mutex_lock(&(pool->lock));
	if(rc != 0){
		log_error("threadpool_destory() : pthread_mutex_lock() failed");
		return tp_lock_fail;
	}

	do{
		//设置shutdown_mode，并唤醒所有线程
		if(pool->shutdown){//shutdown不等于0，其值要么是 1，要么是 2，这两个值都是表示已关闭
			err = tp_already_shutdown;
			log_info("threadpool_destory() : threadpool already shutdown");
			break;
		}

		pool->shutdown = (shutdown_mode) ? graceful_shutdown : immediate_shutdown;

		rc = pthread_cond_broadcast(&(pool->cond));
		if(rc != 0){
			err = tp_cond_broadcast;
			log_error("threadpool_destory() : pthread_cond_broadcast() failed");
			break;
		}

		rc = pthread_mutex_unlock(&(pool->lock));
		if(rc != 0){
			err = tp_lock_fail;//这里是不是应该改成 tp_unlock_fail ???
			log_error("threadpool_destory() : pthread_mutex_unlock() failed");
			break;
		}

		int i = 0;
		for(i = 0; i < thread_count; ++i){
			rc = pthread_join(pool->threads_id[i], NULL);
			if(rc != 0){
				err = tp_thread_fail;//这里是不是应该改成 tp_thread_join_fail ???
			}

			log_info("threads_id 0x%x exit", (uint32_t)pool->threads_id[i]);
		}
	}whlie(0);

	if(err == 0){
		(void)pthread_mutex_destroy(&(pool->lock));
		(void)pthread_cond_destory(&(pool->cond));

		threadpool_free(pool);
	}

	return err;
}

void *threadpool_worker(void *arg){
	if(arg == NULL){
		log_error("threadpool_worker() : arg should not be NULL");
		return NULL;
	}

	threadpool_t *pool = (threadpool_t *)arg;
	task_t *task;

	while(1){
		pthread_mutex_lock(&(pool->lock));//应该加上容错

		//等待条件变量
		//当 任务链表中的任务数量为 0，并且 线程池是未关闭的状态
		while((pool->queue_size == 0) && !(pool->shutdown)){
			pthread_cond_wait(&(pool->cond), &(pool->lock));//应该加上容错
		}

		if(pool->shutdown == immediate_shutdown){
			break;
		}else if((pool->shutdown == graceful_shutdown) && pool->queue_size == 0){
			break;
		}

		task = pool->head->next;
		if(task == NULL){
			pthread_mutex_unlock(&(pool->lock));
			continue;
		}

		pool->head->next = task->next;
		pool->queue_size--;

		pthread_mutex_unlock(&(pool->lock));

		(*(task->func))(task->arg);

		free(task);
		
		pool->started--;
	}

	pthread_exit(NULL);

	return NULL;
}
