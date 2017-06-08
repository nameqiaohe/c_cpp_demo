/*####################################################
# File Name: header.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-29 22:11:20
# Last Modified: 2016-12-19 21:35:43
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
#define TIP "Tip : When the max_number increased to 128, you win!"

#define INFOFILE "bestInfo.txt"
#define MAX 25

typedef struct Map{
	int m_bestScore;
	int m_bestScoreSteps;
	int m_score;
	int m_steps;

	int m_winFlag;
	
	char m_filename[MAX];

	int m_map[ROW][COL];
	int m_maxNumber;
	int m_empty;
	int m_old_row, m_old_col;
}Map;

Map map;

int gameOverFlag;

WINDOW *HELP_WIN, *GAME_WIN;

void draw();
void play();
void init();
void initValue();
void getBestScoreInfo();

void drawNumber(int row, int col);
void countValue(int *new_row, int *new_col);
int countOne(int row, int col);
void gameOver();

void moveLeft(int row, int col, int *step);
void moveRight(int row, int col, int *step);
void moveUp(int row, int col, int *step);
void moveDown(int row, int col, int *step);

void writeBestScoreInfoToFile();

void helpInfo();
void addScore(int number);
void refreshScore();

void setMaxNumber();

#endif//GAME_H_
