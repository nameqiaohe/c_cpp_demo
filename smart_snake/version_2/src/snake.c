#pragma once
//如果加入 #pragma execution_character_set("utf-8")，则中文字体会异常
//#pragma execution_character_set("utf-8")

#include <stdio.h>
#include <graphics.h>
#include <time.h>

#define ROW 480
#define COL 640

//方向键值
typedef enum KEY{
	KEY_UP = 294,
	KEY_DOWN = 296,
	KEY_LEFT = 293,
	KEY_RIGHT = 295,
}Key;

//蛇的移动方向
typedef enum Direction{
	UP = 72,
	DOWN = 80,
	LEFT = 75,
	RIGHT = 77
}Direction;

//坐标
typedef struct Coor{
	int x;
	int y;
}Coor;

//蛇
typedef struct Snake{
	Direction dir;
	int len;
	Coor coor[100];
}Snake;

Snake snake;

//食物
typedef struct Food{
	Coor foodCoor;

	int isEat;
}Food;

Food food;

void DrawSnake(){
	cleardevice();

	setfillcolor(YELLOW);
	bar(snake.coor[0].x, snake.coor[0].y, snake.coor[0].x + 10, snake.coor[0].y + 10);

	int i = 1;
	for (i; i < snake.len; i++){
		setfillcolor(WHITE);
		bar(snake.coor[i].x, snake.coor[i].y, snake.coor[i].x + 10, snake.coor[i].y + 10);
	}
}

void SnakeMove(){
	int i = snake.len - 1;
	for (i; i > 0; i--){
		snake.coor[i].x = snake.coor[i - 1].x;
		snake.coor[i].y = snake.coor[i - 1].y;
	}

	switch (snake.dir){
	case UP:
		snake.coor[0].y = snake.coor[0].y - 10;
		break;
	case DOWN:
		snake.coor[0].y = snake.coor[0].y + 10;
		break;
	case LEFT:
		snake.coor[0].x = snake.coor[0].x - 10;
		break;
	case RIGHT:
		snake.coor[0].x = snake.coor[0].x + 10;
		break;
	default:
		break;
	}
}  

void ChangeDirection(){
	int move;
	move = getch();

	switch (move){
	case 'W':
	case 'w':
	case KEY_UP:
		if (snake.dir == DOWN){
			break;
		}
		if (snake.coor[0].x == snake.coor[1].x && snake.coor[0].y == snake.coor[1].y){
			break;
		}
		snake.dir = UP;
		break;
	case 'S':
	case 's':
	case KEY_DOWN:
		if (snake.dir == UP){
			break;
		}
		if (snake.coor[0].x == snake.coor[1].x && snake.coor[0].y == snake.coor[1].y){
			break;
		}
		snake.dir = DOWN;
		break;
	case 'A':
	case 'a':
	case KEY_LEFT:
		if (snake.dir == RIGHT){
			break;
		}
		if (snake.coor[0].x == snake.coor[1].x && snake.coor[0].y == snake.coor[1].y){
			break;
		}
		snake.dir = LEFT;
		break;
	case 'D':
	case 'd':
	case KEY_RIGHT:
		if (snake.dir == LEFT){
			break;
		}
		if (snake.coor[0].x == snake.coor[1].x && snake.coor[0].y == snake.coor[1].y){
			break;
		}
		snake.dir = RIGHT;
		break;
	default:
		break;
	}
}

void GenerateFood(){
	//保证不能有异常的食物坐标，如 (20, 25)，这样蛇吃的时候只能吃一半，代码实现起来稍微复杂一点
	food.foodCoor.x = rand() % 64 * 10;//640 为 界面的宽
	food.foodCoor.y = rand() % 48 * 10;//480 为 界面的高
	//food.foodCoor.x = rand() % 63 * 10;//因为画的是圆，所以算出的坐标不能超出最大值
	//food.foodCoor.y = rand() % 47 * 10;//

	//还要保证食物的坐标不能 和 蛇身冲突

	food.isEat = 1;// 没有被吃
}

void DrawFood(){
	if (food.isEat){
		setfillcolor(RED);//设置填充色
		//因为画的是圆，所以要用算出的坐标加上半径的距离
		//fillellipse(food.foodCoor.x + 5, food.foodCoor.y + 5, 5, 5);//画填充的椭圆，如果后两个参数一样，则是一个圆
		bar(food.foodCoor.x, food.foodCoor.y, food.foodCoor.x + 10, food.foodCoor.y + 10);//食物，矩形
	}
}

void EatFood(){
	if (snake.coor[0].x == food.foodCoor.x && snake.coor[0].y == food.foodCoor.y){
		snake.len++;
		food.isEat = 0;//被吃掉
	}
}

void GameOver(){
	cleardevice();

	setcolor(YELLOW);
	//setfont(18, 0, "宋体");
	setfont(18, 0, "华文行楷");
	settextjustify(LEFT_TEXT, TOP_TEXT);
	outtextxy(30, 50, "Game Over");
	outtextxy(30, 80, "游戏结束");
}

int BreakGame(){
	if (snake.coor[0].x < 0 || snake.coor[0].x > COL || snake.coor[0].y < 0 || snake.coor[0].y > ROW){
		GameOver();
		return 1;
	}

	int i = 1;
	for (i; i < snake.len; i++){
		if (snake.coor[0].x == snake.coor[i].x && snake.coor[0].y == snake.coor[i].y){//
			GameOver();
			return 1;
		}
	}

	return 0;
}

int main(void){
	srand((unsigned int)time(NULL));

	//InitSnake(snake);
	snake.len = 2;
	snake.dir = RIGHT;
	snake.coor[1].x = 0;
	snake.coor[1].y = 0;
	
	snake.coor[0].x = 10;
	snake.coor[0].y = 0;

	//初始化食物
	food.isEat = 1;//没被吃掉

	initgraph(640, 480);
	setbkcolor(LIGHTGRAY);

	DrawSnake();
	GenerateFood();
	DrawFood();

	int res;

	//kbhit() 返回值：有键盘敲击 返回0，没有键盘敲击 返回1
	while (1){
		while (!kbhit()){//若按下键盘，则跳出循环，改变方向
			Sleep(100);
			SnakeMove();

			res = BreakGame();
			if (res != 0){
				break;
			}

			DrawSnake();
			if (!food.isEat){//只有当食物被吃掉之后，再重新生成
				GenerateFood();
			}
			DrawFood();
			EatFood();
		}

		if (res != 0){
			break;
		}

		ChangeDirection();
	}
		
	getch();
	closegraph();
	return 0;
}
