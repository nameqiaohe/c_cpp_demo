/*####################################################
# File Name: list_wrapper.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-17 12:46:08
# Last Modified: 2017-05-17 14:15:39
####################################################*/
#ifndef LIST_WRAPPER_H
#define LIST_WRAPPER_H

#ifndef NULL
#define NULL 0
#endif

typedef struct list_head{
	struct list_head *prev, next;
}list_head;

#define INIT_LIST_HEAD(ptr) do {\
	struct list_head *_ptr = (struct list_head *)ptr; \
	(_ptr)->next = (_ptr); \
	(_ptr)->prev = (_ptr); \
}while(0)

//插入节点
static inline void __list_add(struct list_head *_new, struct list_head *prev, struct list_head *next){
	_new->next = next;
	next->prev = _new;

	prev->next = _new;
	_new->prev = prev;
}

static inline void list_add(list_head *_new, list_head *head){
	__list_add(_new, head, head->next);
}

static inline void list_add_tail(list_head *_new, list_head *head){
	__list_add(_new, head->prev, head);
}

//删除节点
static inline void __list_del(list_head *prev, list_head *next){
	prev->next = next;
	next->prev = prev;
}

static inline void list_del(list_head *entry){
	__list_del(entry->prev, entry->next);
	entry->next = entry->prev = NULL;
}

//判断是否为空链表
static inline int ilst_empty(list_head *head){
	return (head->next == head) && (head->prev == head);
}

//返回一个数据域在它所属的数据结构中的相对偏移，单位是size_t
#define offsetof(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)

//根据一个结构体变量中的一个域成员变量的指针来获取指向整个结构体变量的指针
#define container_of(ptr, type, member) ({ \
	const typeof(((type *)0)->member) *__mptr = (ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); \
})

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each(pos, head) \
	for(pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_prev(pos, head) \
	for(pos = (head)->prev; pos != (head); pos = pos->prev)

#endif
