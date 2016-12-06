/*####################################################
# File Name: game.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-29 22:15:59
# Last Modified: 2016-12-06 23:01:55
####################################################*/
#include "game.h"

int main(void){
	init();
	play();
	endwin();

	return 0;
}

void init(){
	initscr();
	cbreak();
	noecho();
	curs_set(0);

	int row, col;
	empty = PANE;
	srand(time(0));
	col = rand() % COL;
	row = rand() % ROW;

	map[row][col] = 2;
	empty--;

	draw();
}

void draw(){
	int width, height, row, col;

	clear();

	//画四条横线
	for(height = ROW_START; height < ROW_START+HEIGHT+ROW_DISTANCE; height += 3){
		for(width = COL_START; width < COL_START+WIDTH-1; ++width){
			move(height, width);
			addch('-');
			refresh();
		}
	}

	//画四条竖线
	for(width = COL_START; width < COL_START+WIDTH; width += COL_DISTANCE){
		for(height = ROW_START+1; height < ROW_START+HEIGHT+1; ++height){
			move(height, width);
			addch('|');
			refresh();
		}
	}

	//给每一个框中填上数字
	for(row = 0; row < ROW; ++row){
		for(col = 0; col < COL; ++col){
			draw_number(row, col);
		}
	}
}

//向方框中填入数字
void draw_number(int row, int col){
	int i, m, k, j;

	char temp[5] = {0x00};
	i = map[row][col];
	m = 0;

	while(i > 0){
		j = i % 10;
		temp[m++] = j + '0';
		i = i/10;
	}

	m = 0;
	k = (col+COL_START/COL_DISTANCE+1)*COL_DISTANCE+1;
	while(temp[m] != 0x00){
		move(ROW_START + ROW_DISTANCE*(row + 1) + row, k);
		addch(temp[m++]);
		k--;
	}
}

void play(){
	int col = 0, row = 0, i = 0, new_col, new_row, number;
	int old_empty, step;
	char ch;

	while(1){
		step = 0;
		old_empty = empty;
		ch = getch();
		switch(ch){
			case 97:	//左 a
			case 104:	// h
			case 68:	//左方向键
				moveLeft(row, col, i, &step);
					break;

			case 100:	//右移 d
			case 108:	// l
			case 67:	//右方向键
				moveRight(row, col, i, &step);
					break;

			case 119:	//上移 w
			case 107:	// k
			case 65:	//上方向键
				moveUp(row, col, i, &step);
				break;
			case 115:	//下移 s
			case 106:	// j
			case 66:	//下方向键
				moveDown(row, col, i, &step);
				break;
			case 'Q':
			case 'q':
				game_over();
				break;
			default:
				continue;
				break;
		}

		if(empty <= 0){
			game_over();
		}

		if((empty != old_empty) || (step == 1)){
			do{
				new_col = rand() % 4;
				new_row = rand() % 4;
			}while(map[new_row][new_col] != 0);

			cnt_value(&new_row, &new_col);

			do{
				number = rand() % 4;
			}while(number == 0 || number == 2);

			map[new_row][new_col] = number + 1;
			empty--;
		}

		draw();
	}
}

//下边两个函数用于统计每一个方格周围的空的格子个数
int cnt_one(int row, int col){
	int value = 0;

	if(row-1 > 0){
		map[row-1][col] ? 0 : value++;
	}
	if(row+1 < 4){
		map[row+1][col] ? 0 : value++;
	}

	if(col-1 >= 0){
		map[row][col-1] ? 0 : value++;
	}
	if(col+1 < 4){
		map[row][col+1] ? 0 : value++;
	}

	if(row-1 >= 0 && col-1 >= 0){
		map[row-1][col-1] ? 0 : value++;
	}
	if(row-1 >= 0 && col+1 < 4){
		map[row-1][col+1] ? 0 : value++;
	}

	if(row+1 < 4 && col-1 >= 0){
		map[row+1][col-1] ? 0 : value++;
	}
	if(row+1 < 4 && col+1 < 4){
		map[row+1][col+1] ? 0 : value++;
	}

	return value;
}

void cnt_value(int *new_row, int *new_col){
	int col, row, value;
	int max = 0;

	max = cnt_one(*new_row, *new_col);
	for(row = 0; row < ROW; ++row){
		for(col = 0; col < COL; ++col){
			if(!map[row][col]){	//if(map[row][col] == 0)
				value = cnt_one(row, col);
				if(value > max && old_row != row && old_col != col){
					//避免同一位置反复出现数字
					*new_row = row;
					*new_col = col;
					old_row = row;
					old_row = col;
					break;
				}
			}
		}
	}
}

int game_over(){
	sleep(1);
	endwin();
	exit(1);
}

void moveLeft(int row, int col, int i, int *step){
	for(row = 0; row < 4; ++row){
		for(col = 0; col < 4;){
			if(map[row][col] == 0){
				col++;
				continue;
			}else{
				for(i = col+1; i < 4; ++i){
					if(map[row][col] == 0){
						continue;
					}else{
						if(map[row][col] == map[row][i]){
							map[row][col] += map[row][i];
							map[row][i] = 0;
							empty++;
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
			if(map[row][col] == 0){
				continue;
			}else{
				for(i = col; (i > 0) && (map[row][i-1] == 0); --i){
					map[row][i-1] = map[row][i];
					map[row][i] = 0;
					*step = 1;
				}
			}
		}
	}
}

void moveRight(int row, int col, int i, int *step){
	for(row = 0; row < 4; ++row){
		for(col = 3; col >= 0;){
			if(map[row][col] == 0){
				col--;
				continue;
			}else{
				for(i = col-1; i >= 0; i--){
					if(map[row][i] == 0){
						continue;
					}else if(map[row][col] == map[row][i]){
						map[row][col] += map[row][i];
						map[row][i] = 0;
						empty++;
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
			if(map[row][col] == 0){
				continue;
			}else{
				for(i = col; (i < 3) && (map[row][i+1] == 0); ++i){
					map[row][i+1] = map[row][i];
					map[row][i] = 0;
					*step = 1;
				}
			}
		}
	}
}

void moveUp(int row, int col, int i, int *step){
	for(col = 0; col < 4; ++col){
		for(row = 0; row < 4;){
			if(map[row][col] == 0){
				row++;
				continue;
			}else{
				for(i = row+1; i < 4; ++i){
					if(map[i][col] == 0){
						continue;
					}else if(map[row][col] == map[i][col]){
						map[row][col] += map[i][col];
						map[i][col] = 0;
						empty++;
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
			if(map[row][col] == 0){
				continue;
			}else{
				for(i = row; (i > 0) && (map[i-1][col] == 0); --i){
					map[i-1][col] = map[i][col];
					map[i][col] = 0;
					*step = 1;
				}
			}
		}
	}
}

void moveDown(int row, int col, int i, int *step){
	for(col = 0; col < 4; ++col){
		for(row = 3; row >= 0;){
			if(map[row][col] == 0){
				--row;
				continue;
			}else{
				for(i = row-1; i >= 0; --i){
					if(map[i][col] == 0){
						continue;
					}else if(map[row][col] == map[i][col]){
						map[row][col] += map[i][col];
						map[i][col] = 0;
						empty++;
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
			if(map[row][col] == 0){
				continue;
			}else{
				for(i = row; (i < 3) && (map[i+1][col] == 0); ++i){
					map[i+1][col] = map[i][col];
					map[i][col] = 0;
					*step = 1;
				}
			}
		}
	}
}