/*####################################################
# File Name: game.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-29 22:15:59
# Last Modified: 2016-12-13 21:36:05
####################################################*/
#include "../include/game.h"
#include <string.h>

void initValue(){
	map.m_bestScore = 0;
	map.m_bestScoreSteps = 0;
	map.m_score = 0;
	map.m_steps = 0;
	map.m_empty = PANE;
	map.m_maxNumber = 2;
	map.m_winFlag = 128;

	gameOverFlag = 0;//0 表示正在运行，1 表示结束

	memset(map.m_filename, 0, MAX);
	strncpy(map.m_filename, INFOFILE, strlen(INFOFILE));

	int row = 0, \
		col = 0;
	for(; row < ROW; ++row){
		for(; col < COL; ++col){
			map.m_map[row][col] = 0;
		}
	}
}

void init(){
	initValue();

	initscr();	/* Start curses mode */
	cbreak();	/* Line buffering disabled */
	noecho();	/* Don't echo() while we do getch */
	curs_set(0);

	int row, col;
	srand(time(0));
	col = rand() % COL;
	row = rand() % ROW;

	map.m_map[row][col] = 2;
	map.m_empty--;

	draw();
}

void draw(){
	int width, height, row, col;

	clear();

	helpInfo();

	//画四条横线
	for(height = ROW_START; height < ROW_START+HEIGHT+ROW_DISTANCE; height += 3){
		for(width = COL_START; width < COL_START+WIDTH-1; ++width){
			mvaddch(height, width, '-');
			refresh();	/* Print it on to the real screen */
		}
	}

	//画四条竖线
	for(width = COL_START; width < COL_START+WIDTH; width += COL_DISTANCE){
		for(height = ROW_START+1; height < ROW_START+HEIGHT+1; ++height){
			mvaddch(height, width, '|');
			refresh();
		}
	}

	//给每一个框中填上数字
	for(row = 0; row < ROW; ++row){
		for(col = 0; col < COL; ++col){
			drawNumber(row, col);
		}
	}
}

//向方框中填入数字
void drawNumber(int row, int col){
	int i, m, k, j;

	char temp[5] = {0x00};
	i = map.m_map[row][col];
	m = 0;

	while(i > 0){
		j = i % 10;
		temp[m++] = j + '0';
		i = i/10;
	}

	m = 0;
	k = (col+COL_START/COL_DISTANCE+1)*COL_DISTANCE+1;
	while(temp[m] != 0x00){
		mvaddch(ROW_START + ROW_DISTANCE*(row + 1) + row, k, temp[m++]);
		k--;
	}
}

void play(){
	int col = 0, row = 0, new_col, new_row, number;
	int old_empty, step;
	char ch;

	while(1){
		step = 0;
		old_empty = map.m_empty;
		ch = getch();	/* Wait for user input */
		switch(ch){
			case 97:	//左 a
			case 104:	// h
			case 68:	//左方向键
				moveLeft(row, col, &step);
				setMaxNumber();
				++map.m_steps;
					break;

			case 100:	//右移 d
			case 108:	// l
			case 67:	//右方向键
				moveRight(row, col, &step);
				setMaxNumber();
				++map.m_steps;
					break;

			case 119:	//上移 w
			case 107:	// k
			case 65:	//上方向键
				moveUp(row, col, &step);
				setMaxNumber();
				++map.m_steps;
				break;
			case 115:	//下移 s
			case 106:	// j
			case 66:	//下方向键
				moveDown(row, col, &step);
				setMaxNumber();
				++map.m_steps;
				break;
			case 'Q':
			case 'q':
				gameOver();
				cbreak();
				break;
			default:
				continue;
				break;
		}

		if(map.m_empty <= 0){
			gameOver();
		}

		if((map.m_empty != old_empty) || (step == 1)){
			do{
				new_col = rand() % 4;
				new_row = rand() % 4;
			}while(map.m_map[new_row][new_col] != 0);

			countValue(&new_row, &new_col);

			do{
				number = rand() % 4;
			}while(number == 0 || number == 2);

			map.m_map[new_row][new_col] = number + 1;
			map.m_empty--;
		}

		draw();
		if(gameOverFlag != 0){
			break;
		}

		//若达到获胜标志，则退出游戏界面
		if(map.m_maxNumber == map.m_winFlag){
			break;
		}
	}
}

//统计每一个方格周围一圈空的格子个数
int countOne(int row, int col){
	int value = 0;

	if(row-1 > 0){
		map.m_map[row-1][col] ? 0 : value++;
	}
	if(row+1 < 4){
		map.m_map[row+1][col] ? 0 : value++;
	}

	if(col-1 >= 0){
		map.m_map[row][col-1] ? 0 : value++;
	}
	if(col+1 < 4){
		map.m_map[row][col+1] ? 0 : value++;
	}

	if(row-1 >= 0 && col-1 >= 0){
		map.m_map[row-1][col-1] ? 0 : value++;
	}
	if(row-1 >= 0 && col+1 < 4){
		map.m_map[row-1][col+1] ? 0 : value++;
	}

	if(row+1 < 4 && col-1 >= 0){
		map.m_map[row+1][col-1] ? 0 : value++;
	}
	if(row+1 < 4 && col+1 < 4){
		map.m_map[row+1][col+1] ? 0 : value++;
	}

	return value;
}

void countValue(int *new_row, int *new_col){
	int col, row, value;
	int max = 0;

	max = countOne(*new_row, *new_col);
	for(row = 0; row < ROW; ++row){
		for(col = 0; col < COL; ++col){
			if(!map.m_map[row][col]){	//if(map[row][col] == 0)
				value = countOne(row, col);//优选周围空格子最多的方格填入数字
				if(value > max && map.m_old_row != row && map.m_old_col != col){
					//避免同一位置反复出现数字
					*new_row = row;
					*new_col = col;
					map.m_old_row = row;
					map.m_old_row = col;
					break;
				}
			}
		}
	}
}

void gameOver(){
	if(map.m_bestScore < map.m_score){
		map.m_bestScore = map.m_score;
		map.m_bestScoreSteps = map.m_steps;
		
		writeBestScoreInfoToFile();
	}
	gameOverFlag = 1;//gameOverFlag用于标识何时应该跳出while循环
	sleep(1);
	endwin();	/* End curses mode */
	//exit(1);	//之前的操作是：判断游戏结束时，直接退出，这样不太好
}

void moveLeft(int row, int col, int *step){
	int i = 0;
	for(row = 0; row < 4; ++row){
		for(col = 0; col < 4;){
			if(map.m_map[row][col] == 0){
				col++;
				continue;
			}else{
				for(i = col+1; i < 4; ++i){
					if(map.m_map[row][col] == 0){
						continue;
					}else{
						if(map.m_map[row][col] == map.m_map[row][i]){
							addScore(map.m_map[row][col]);

							map.m_map[row][col] += map.m_map[row][i];
							map.m_map[row][i] = 0;

							map.m_empty++;
							break;
						}else{
							break;
						}
					}
				}
				col = i;
			}
		}
	}

	for(row = 0; row < 4; ++row){
		for(col = 0; col < 4; col++){
			if(map.m_map[row][col] == 0){
				continue;
			}else{
				for(i = col; (i > 0) && (map.m_map[row][i-1] == 0); --i){
					map.m_map[row][i-1] = map.m_map[row][i];
					map.m_map[row][i] = 0;
					*step = 1;
				}
			}
		}
	}
}

void moveRight(int row, int col, int *step){
	int i = 0;
	for(row = 0; row < 4; ++row){
		for(col = 3; col >= 0;){
			if(map.m_map[row][col] == 0){
				col--;
				continue;
			}else{
				for(i = col-1; i >= 0; i--){
					if(map.m_map[row][i] == 0){
						continue;
					}else if(map.m_map[row][col] == map.m_map[row][i]){
						addScore(map.m_map[row][col]);

						map.m_map[row][col] += map.m_map[row][i];
						map.m_map[row][i] = 0;

						map.m_empty++;
						break;
					}else{
						break;
					}
				}
				col = i;
			}
		}
	}
	for(row = 0; row < 4; ++row){
		for(col = 3; col >= 0; --col){
			if(map.m_map[row][col] == 0){
				continue;
			}else{
				for(i = col; (i < 3) && (map.m_map[row][i+1] == 0); ++i){
					map.m_map[row][i+1] = map.m_map[row][i];
					map.m_map[row][i] = 0;
					*step = 1;
				}
			}
		}
	}
}

void moveUp(int row, int col, int *step){
	int i = 0;
	for(col = 0; col < 4; ++col){
		for(row = 0; row < 4;){
			if(map.m_map[row][col] == 0){
				row++;
				continue;
			}else{
				for(i = row+1; i < 4; ++i){
					if(map.m_map[i][col] == 0){
						continue;
					}else if(map.m_map[row][col] == map.m_map[i][col]){
						addScore(map.m_map[row][col]);

						map.m_map[row][col] += map.m_map[i][col];
						map.m_map[i][col] = 0;

						map.m_empty++;
						break;
					}else{
						break;
					}
				}
				row = i;
			}
		}
	}
	for(col = 0; col < 4; ++col){
		for(row = 0; row < 4; ++row){
			if(map.m_map[row][col] == 0){
				continue;
			}else{
				for(i = row; (i > 0) && (map.m_map[i-1][col] == 0); --i){
					map.m_map[i-1][col] = map.m_map[i][col];
					map.m_map[i][col] = 0;
					*step = 1;
				}
			}
		}
	}
}

void moveDown(int row, int col, int *step){
	int i = 0;
	for(col = 0; col < 4; ++col){
		for(row = 3; row >= 0;){
			if(map.m_map[row][col] == 0){
				--row;
				continue;
			}else{
				for(i = row-1; i >= 0; --i){
					if(map.m_map[i][col] == 0){
						continue;
					}else if(map.m_map[row][col] == map.m_map[i][col]){
						addScore(map.m_map[row][col]);

						map.m_map[row][col] += map.m_map[i][col];
						map.m_map[i][col] = 0;

						map.m_empty++;
						break;
					}else{
						break;
					}
				}
				row = i;
			}
		}
	}
	for(col = 0; col < 4; ++col){
		for(row = 3; row >= 0; --row){
			if(map.m_map[row][col] == 0){
				continue;
			}else{
				for(i = row; (i < 3) && (map.m_map[i+1][col] == 0); ++i){
					map.m_map[i+1][col] = map.m_map[i][col];
					map.m_map[i][col] = 0;
					*step = 1;
				}
			}
		}
	}
}

void helpInfo(){
	mvaddstr(1, COL_START, UP_INFO);
	refresh();

	mvaddstr(2, COL_START, DOWN_INFO);
	refresh();

	mvaddstr(3, COL_START, LEFT_INFO);
	refresh();

	mvaddstr(4, COL_START, RIGHT_INFO);
	refresh();

	char str[MAX] = {0};
	getBestScoreInfo();
	mvaddstr(6, COL_START, BEST_SCORE_INFO);
	refresh();
	sprintf(str, "%d", map.m_bestScore);
	mvaddstr(6, COL_START + strlen(BEST_SCORE_INFO), str);
	refresh();

	mvaddstr(7, COL_START, BEST_SCORE_STEPS);
	refresh();
	sprintf(str, "%d", map.m_bestScoreSteps);
	mvaddstr(7, COL_START + strlen(BEST_SCORE_STEPS), str);
	refresh();

	mvaddstr(8, COL_START, CURRENT_SCORE);
	refresh();

	mvaddstr(9, COL_START, TIP);
	refresh();
}

void addScore(int number){
	map.m_score += number;
}

void refreshScore(){
	
}

void getBestScoreInfo(){
	char str[MAX] = {0};
	FILE *fp = fopen(map.m_filename, "r");
	if(fp != NULL){
		fgets(str, MAX, fp);
		map.m_bestScore = atoi(str);

		memset(str, '0', MAX);
		fgets(str, MAX, fp);
		map.m_bestScoreSteps = atoi(str);

		fclose(fp);
	}
}

void writeBestScoreInfoToFile(){
	FILE *fp = fopen(map.m_filename, "w");
	if(fp == NULL){
		return;
	}

	char str[MAX] = {0};
	sprintf(str, "%d\n", map.m_bestScore);
	fputs(str, fp);

	memset(str, '0', MAX);
	sprintf(str, "%d", map.m_bestScoreSteps);
	fputs(str, fp);

	fclose(fp);
}

void setMaxNumber(){
	int row = 0, col = 0;
	for(row = 0; row < ROW; ++row){
		for(col = 0; col < COL; ++col){
			if(map.m_maxNumber < map.m_map[row][col]){
				map.m_maxNumber = map.m_map[row][col];
			}
		}
	}
}
