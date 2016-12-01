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

#define PANE 16

#define ROW_START 0
#define COL_START 0

#define WIDTH 22
#define HEIGHT 10

#define ROW 4
#define COL 4

int map[ROW][COL] = {0};
int empty;
int old_row, old_col;

void draw();
void play();
void init();
void draw_one(int row, int col);
void cnt_value(int *new_row, int *new_col);
int game_over();
int cnt_one(int row, int col);

#endif
