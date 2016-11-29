/*####################################################
# File Name: header.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-29 22:11:20
# Last Modified: 2016-11-29 22:11:20
####################################################*/
#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int map[4][4] = {0};
int empty;
int old_y, old_x;

void draw();
void play();
void init();
void draw_one(int y, int x);
void cnt_value(int *new_y, int *new_x);
int game_over();
int cnt_one(int y, int x);

#endif
