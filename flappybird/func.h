#pragma once

#include <curses.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define BIRD 'O'
#define STONE '*'
#define BLANK ' '

typedef struct node{
	int x, y;
	int gap;//每根柱子的上下间隔
	struct node *next;
}Node, *Lnode;

Lnode head, tail;
int bird_x, bird_y;
int ticker;

void init();
void init_bird();
void init_draw();
void init_head();
void init_wall();
void drop(int sig);

int set_ticker(int n);
