/*####################################################
# File Name: header.h
# Author: xxx
# Last Modified: 2016-12-07 21:31:50
# Create Time: 2016-11-29 22:11:20
# Last Modified: 2016-12-07 23:00:55
####################################################*/
#ifndef GAME_H_
#define GAME_H_

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

#define UP_INFO "Up : Up / w / k"
#define DOWN_INFO "Down : Down / s / j"
#define LEFT_INFO "Left : Left / a / h"
#define RIGHT_INFO "Right : Right / d / l"

#define BEST_SCORE_INFO "BestScore : "
#define BEST_SCORE_STEPS "BestScoreSteps : "
#define CURRENT_SCORE "CurrentScoer : "

#define MAX 25

char *FILENAME = "bestInfo.txt";

int map[ROW][COL] = {0};
int empty;
int old_row, old_col;

int bestScore = 0;
int bestScoreSteps = 0;
int score = 0;
int steps = 0;

void draw();
void play();
void init();
void getBestScoreInfo();

void drawNumber(int row, int col);
void countValue(int *new_row, int *new_col);
int countOne(int row, int col);
int gameOver();

void moveLeft(int row, int col, int i, int *step);
void moveRight(int row, int col, int i, int *step);
void moveUp(int row, int col, int i, int *step);
void moveDown(int row, int col, int i, int *step);

void writeBestScoreInfoToFile();

void helpInfo();
void addScore(int number);
void refreshScore();

#endif//GAME_H_
