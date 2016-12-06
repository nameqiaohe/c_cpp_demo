/*####################################################
# File Name: header.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-29 22:11:20
# Last Modified: 2016-12-06 23:07:02
####################################################*/
#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define ROW 4
#define COL 4

#define PANE ROW*COL

#define ROW_START 10
#define COL_START 35

#define WIDTH 42
#define HEIGHT 11

#define COL_DISTANCE 10
#define ROW_DISTANCE 2

int map[ROW][COL] = {0};
int empty;
int old_row, old_col;

void draw();
void play();
void init();
void draw_number(int row, int col);
void count_value(int *new_row, int *new_col);
int count_one(int row, int col);
int game_over();
void moveLeft(int row, int col, int i, int *step);
void moveRight(int row, int col, int i, int *step);
void moveUp(int row, int col, int i, int *step);
void moveDown(int row, int col, int i, int *step);

#endif
