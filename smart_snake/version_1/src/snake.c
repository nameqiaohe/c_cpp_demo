/*####################################################
# File Name: snake.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-30 10:32:17
# Last Modified: 2017-05-30 10:53:34
####################################################*/
#include "snake.h"
#include "dbg.h"
#include <stdio.h>

int init_snake(){
	if(snake.body != NULL){
		log_info("init_snake() : snake.body != NULL");
		return FALSE;
	}

	snake.body = (point_t *)malloc(sizeof(point_t) * MAX_X * MAX_Y);
	if(snake.body == NULL){
		log_error("init_snake() : malloc space for snake.body error");
		return FALSE;
	}
	
	int i = 0;
	for(i = 0; i < MAX_X * MAX_Y; ++i){
		snake.body[i].x = NEGATIVE_FLAG;
		snake.body[i].y = NEGATIVE_FLAG;
	}

	snake.body[0].x = 9;
	snake.body[0].y = 5;

	snake.body[0].x = 10;
	snake.body[0].y = 5;

	snake.body[0].x = 11;
	snake.body[0].y = 5;

	snake.snake_size = 3;
}

void destory_snake(){
	if(snake.body == NULL){
		log_info("destory_snake() : snake.body is already destoried");
		return;
	}

	free(snake.body);
	snake.body = NULL;
}

//只需要判断 移动到的下一个位置 是否 和蛇身的第一个节点位置重叠
int is_direction_legal(point_t next){
	if(TRUE == is_match(next, snake.body[1])){
		return FLASE;//方向不合法
	}else{
		return TRUE;
	}
}

int is_hit_itself(point_t next){
	
}
