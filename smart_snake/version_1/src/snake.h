/*####################################################
# File Name: snake.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-30 10:19:04
# Last Modified: 2017-05-30 10:48:32
####################################################*/
#ifndef SNAKE_H
#define SNAKE_H

#define TRUE 1
#define FALSE 0

#define MAX_X 20
#define MAX_Y 10

#define NEGATIVE_FLAG -1

typedef struct point{
	int x;
	int y;
}point_t;

typedef struct sname{
	int snake_size;
	point_t *body;
}snake_t;

//全局变量
snake_t snake;//蛇
point_t food;//食物
point_t wall[2*(MAX_X + MAX_Y)];

//初始化蛇
int init_snake();

//销毁蛇
void destory_snake();

//初始化墙
void init_wall();

//移动方向是否合法
int is_direction_legal(point_t next);

//是否撞到自己
int is_hit_itself(point_t next);

//移动蛇身
void body_move(point_t next);

//向上移动
int move_up();

int move_down();

int move_left();

int move_right();

//下一个位置是否为 食物所在位置
int is_food(point_t next);

//吃食物
void eat_food(point_t next);

//两个点是否在同一位置
int is_match(point_t p1, point_t p2);

//生成新的食物的位置
int generate_food();

//获取食物所在位置
point_t get_food_coordinate();


#endif
