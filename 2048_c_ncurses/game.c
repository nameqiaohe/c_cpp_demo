/*####################################################
# File Name: game.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-11-29 22:15:59
# Last Modified: 2016-11-29 22:15:59
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

	int x, y;
	empty = 15;
	srand(time(0));
	x = rand() % 4;
	y = rand() % 4;

	map[y][x] = 2;
	draw();
}

void draw(){
	int row, col, x, y;

	clear();

	for(row = 0; row < 9; row += 2){
		for(col = 0; col < 21; ++col){
			move(row, col);
			addch('-');
			refresh();
		}
	}

	for(col = 0; col < 22; col += 5){
		for(row = 1; row < 8; ++row){
			move(row, col);
			addch('|');
			refresh();
		}
	}

	for(y = 0; y < 4; ++y){
		for(x = 0; x < 4; ++x){
			draw_one(y, x);
		}
	}
}

void draw_one(int y, int x){
	int i, m, k, j;

	char c[5] = {0x00};
	i = map[y][x];
	m = 0;

	while(i > 0){
		j = i % 10;
		c[m++] = j + '0';
		i = i/10;
	}

	m = 0;
	k = (x + 1)*5 - 1;
	while(c[m] != 0x00){
		move(2*y+1, k);
		addch(c[m++]);
		k--;
	}
}

void play(){
	int x, y, i, new_x, new_y, temp;
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
				for(y = 0; y < 4; ++y){
					for(x = 0; x < 4;){
						if(map[y][x] == 0){
							x++;
							continue;
						}else{
							for(i = x+1; i < 4; ++i){
								if(map[y][x] == 0){
									continue;
								}else{
									if(map[y][x] == map[y][i]){
										map[y][x] += map[y][i];
										map[y][i] = 0;
										empty++;
										break;
									}else{
										break;
									}
								}
							}
							x = i;
						}
					}
				}

				for(y = 0; y < 4; ++y){
					for(x = 0; x < 4; x++){
						if(map[y][x] == 0){
							continue;
						}else{
							for(i = x; (i > 0) && (map[y][i-1] == 0); --i){
								map[y][i-1] = map[y][i];
								map[y][i] = 0;
								step = 1;
							}
						}
					}
				}
				break;

			case 100:	//右移 d
			case 108:	// l
			case 67:	//右方向键
				for(y = 0; y < 4; ++y){
					for(x = 3; x >= 0;){
						if(map[y][x] == 0){
							x--;
							continue;
						}else{
							for(i = x-1; i >= 0; i--){
								if(map[y][i] == 0){
									continue;
								}else if(map[y][x] == map[y][i]){
									map[y][x] += map[y][i];
									map[y][i] = 0;
									empty++;
									break;
								}else{
									break;
								}
							}
							x = i;
						}
					}
				}
				for(y = 0; y < 4; ++y){
					for(x = 3; x >= 0; --x){
						if(map[y][x] == 0){
							continue;
						}else{
							for(i = x; (i < 3) && (map[y][i+1] == 0); ++i){
								map[y][i+1] = map[y][i];
								map[y][i] = 0;
								step = 1;
							}
						}
					}
				}
				break;

			case 119:	//上移 w
			case 107:	// k
			case 65:	//上方向键
				for(x = 0; x < 4; ++x){
					for(y = 0; y < 4;){
						if(map[y][x] == 0){
							y++;
							continue;
						}else{
							for(i = y+1; i < 4; ++i){
								if(map[i][x] == 0){
									continue;
								}else if(map[y][x] == map[i][x]){
									map[y][x] += map[i][x];
									map[i][x] = 0;
									empty++;
									break;
								}else{
									break;
								}
							}
							y = i;
						}
					}
				}
				for(x = 0; x < 4; ++x){
					for(y = 0; y < 4; ++y){
						if(map[y][x] == 0){
							continue;
						}else{
							for(i = y; (i > 0) && (map[i-1][x] == 0); --i){
								map[i-1][x] = map[i][x];
								map[i][x] = 0;
								step = 1;
							}
						}
					}
				}
				break;
			case 115:	//下移 s
			case 106:	// j
			case 66:	//下方向键
				for(x = 0; x < 4; ++x){
					for(y = 3; y >= 0;){
						if(map[y][x] == 0){
							--y;
							continue;
						}else{
							for(i = y-1; i >= 0; --i){
								if(map[i][x] == 0){
									continue;
								}else if(map[y][x] == map[i][x]){
									map[y][x] += map[i][x];
									map[i][x] = 0;
									empty++;
									break;
								}else{
									break;
								}
							}
							y = i;
						}
					}
				}
				for(x = 0; x < 4; ++x){
					for(y = 3; y >= 0; --y){
						if(map[y][x] == 0){
							continue;
						}else{
							for(i = y; (i < 3) && (map[i+1][x] == 0); ++i){
								map[i+1][x] = map[i][x];
								map[i][x] = 0;
								step = 1;
							}
						}
					}
				}
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
				new_x = rand() % 4;
				new_y = rand() % 4;
			}while(map[new_y][new_x] != 0);

			cnt_value(&new_y, &new_x);

			do{
				temp = rand() % 4;
			}while(temp == 0 || temp == 2);

			map[new_y][new_x] = temp + 1;
			empty--;
		}

		draw();
	}
}

int cnt_one(int y, int x){
	int value = 0;

	if(y-1 > 0){
		map[y-1][x] ? 0 : value++;
	}
	if(y+1 < 4){
		map[y+1][x] ? 0 : value++;
	}

	if(x-1 >= 0){
		map[y][x-1] ? 0 : value++;
	}
	if(x+1 < 4){
		map[y][x+1] ? 0 : value++;
	}

	if(y-1 >= 0 && x-1 >= 0){
		map[y-1][x-1] ? 0 : value++;
	}
	if(y-1 >= 0 && x+1 < 4){
		map[y-1][x+1] ? 0 : value++;
	}

	if(y+1 < 4 && x-1 >= 0){
		map[y+1][x-1] ? 0 : value++;
	}
	if(y+1 < 4 && x+1 < 4){
		map[y+1][x+1] ? 0 : value++;
	}

	return value;
}

void cnt_value(int *new_y, int *new_x){
	int max_x, max_y, x, y, value;
	int max = 0;

	max = cnt_one(*new_y, *new_x);
	for(y = 0; y < 4; ++y){
		for(x = 0; x < 4; ++x){
			if(!map[y][x]){	//if(map[y][x] == 0)
				value = cnt_one(y, x);
				if(value > max && old_y != y && old_x != x){
					//避免同一位置反复出现数字
					*new_y = y;
					*new_x = x;
					old_y = y;
					old_x = x;
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
