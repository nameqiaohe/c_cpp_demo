#include "func.h"

//设定内核的定时周期
int set_ticker(int n_msec){
	struct itimerval timerset;
	long n_sec, n_usec;

	n_sec = n_msec / 1000;
	n_usec = (n_msec % 1000) * 1000L;

	timerset.it_interval.tv_sec = n_sec;
	timerset.it_interval.tv_usec = n_usec;

	timerset.it_value.tv_sec = n_sec;
	timerset.it_value.tv_usec = n_usec;

	//setitimer函数，参数１为ITIMER_REAL，表示已实际时间计算，发送SIGINT信号
	return setitimer(ITIMER_REAL, &timerset, NULL);
}

//信号接受函数
//让背景向前移动一列，让bird向下掉落一行，而且要检测bird是否撞到柱子，如果撞到，则game over
void drop(int sig){
	int j;
	Lnode tmp, p;
	
//draw bird
	move(bird_y, bird_x);//将光标移动到某个位置
	addch(BLANK);//在这个位置上显示某个字符
	refresh();//将写入窗体的数据 更新到 窗体上
	bird_y++;//bird下降一行
	
	//bird下降一行之后重新绘制bird
	move(bird_y, bird_x);
	addch(BIRD);
	refresh();

	//若检测到 bird 与 柱子相撞，则游戏退出
	if((char)inch() == STONE){
		set_ticker(0);
		sleep(1);
		endwin();//释放窗体所占资源
		exit(0);
	}

//first wall out of screen
	//当有柱子跑出屏幕范围，则释放那个节点，并且重新开辟空间来显示STONE
	p = head->next;
	if(p->x < 0){
		head->next = p->next;
		free(p);
		tmp = (Lnode)malloc(sizeof(Node));
		tmp->x = 99;
		do{
			tmp->y = rand() % 16;
		}while(tmp->y < 5);
		tmp->gap = rand() % 2 + 3;//计算柱子的上下部分间隔，并保存在节点中
		tail->next = tmp;
		tmp->next = NULL;
		tail = tmp;

		ticker -= 10;	//speed up 越往后越快
		set_ticker(ticker);
	}

//draw new walls 每向左移动一列，需要重新绘制整个画面中的柱子
	for(p = head->next; p->next != NULL; p->x--, p = p->next){
		for(j = 0; j < p->y; j++){
			move(j, p->x);
			addch(BLANK);
			refresh();
		}
		//for(j = p->y+5; j <= 23; j++){
		for(j = p->y+p->gap; j <= 23; j++){//绘制柱子的下半部分
			move(j, p->x);
			addch(BLANK);
			refresh();
		}

		if(p->x-10 >= 0 && p->x < 80){
			for(j = 0; j < p->y; j++){
				move(j, p->x-10);
				addch(STONE);
				refresh();
			}
			//for(j = p->y+5; j <= 23; j++){
			for(j = p->y+p->gap; j <= 23; j++){//绘制柱子的下半部分
				move(j, p->x-10);
				addch(STONE);
				refresh();
			}
		}
	}
	tail->x--;
}

//初始化屏幕，调用了ncuses提供的函数
//安装了信号函数drop，并且设定了定时时间
void init(){
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	srand(time(0));
	signal(SIGALRM, drop);

	init_bird();//做好初始化之后，先绘制bird
	init_head();//初始化链表，head为头节点，不存储数据
	init_wall();//生成五个位置，用于接下来在init_draw函数中绘制
	init_draw();//绘制柱子
	//sleep(1);
	ticker = 500;
	set_ticker(ticker);
}

void init_bird(){
	//bird最初的位置在 (5, 15),屏幕的左上角是原点，向下是y正方向
	bird_x = 2;
	bird_y = 15;
	move(bird_y, bird_x);
	addch(BIRD);
	refresh();
	//sleep(1);//绘制好bird之后，先休眠1秒，等待绘制柱子
}

void init_head(){
	Lnode tmp;
	tmp = (Lnode)malloc(sizeof(Node));//分配空间
	tmp->next = NULL;
	head= tmp;//让 head、tail在最开始时指向同一个地方，head、tail是全局变量
	tail = head;
}
//这里还没有开始绘制，在init_draw函数中才开始绘制
//初始化完之后，head之后有五个节点，head作为头节点，不存储数据
void init_wall(){
	int i;
	Lnode tmp, p;

	p = head;
	//这里固定了柱子的绘制范围：宽度，在第0列到100列之间
	//每个节点指向柱子的最右端
	for(i = 19; i <= 99; i += 20){//每隔20列画一根柱子，每根柱子宽10列
		tmp = malloc(sizeof(Node));
		tmp->x = i;
		do{
			tmp->y = rand() % 16;//第一个柱子上半段的最低位置 不能低于 bird的初始位置
		}while(tmp->y < 5);//当生成的随机数 小于5，需要重新生成随机数，主要是为了防止bird来不及飞上去
		tmp->gap = rand() % 2 + 3;//这里也要计算一次gap的值
		p->next = tmp;
		tmp->next = NULL;
		p = tmp;
	}
	tail = p;//不要忘了将tail指向尾节点
}

void init_draw(){
	Lnode p;
	int i, j;
	for(p = head->next; p->next != NULL; p = p->next){
		for(i = p->x; i > p->x-10; i--){//绘制时 从每根柱子的后边向前边绘制
			//绘制上半段
			for(j = 0; j < p->y; j++){
				move(j, i);
				addch(STONE);
				refresh();
			}
			//绘制下半段，中间间隔5行
			//for(j = p->y+5; j <= 23; j++){//这里固定了柱子的绘制范围：高度，最高23行
			for(j = p->y+p->gap; j <= 23; j++){//每根柱子上下部分间隔为gap
				move(j, i);
				addch(STONE);
				refresh();
			}
		}
		//sleep(1);//每绘制一根柱子之后，休眠1秒
	}
}

#if 1
int main(void){
	char ch;
	init();

	while(1){
		ch = getch();
		//当按键为 空格/w/W时，bird向上移动一行
		if(ch == ' ' || ch == 'w' || ch == 'W'){
			move(bird_y, bird_x);
			addch(BLANK);
			refresh();
			bird_y--;

			move(bird_y, bird_x);
			addch(BIRD);
			refresh();
			
			//判断：若bird与柱子的位置重合（相撞），则游戏结束
			//通过判断 光标的当前位置 是否为STONE
			if((char)inch() == STONE){
				set_ticker(0);//时钟设为0
				sleep(1);//休眠1秒之后，释放窗体资源，结束游戏
				endwin();
				exit(0);
			}
		}else if(ch == 'p' || ch == 'P'){//若 按键为p/P，则暂停
			set_ticker(0);//时钟设为0
			do{
				ch = getch();
			}while(ch != 'p' && ch != 'P');//阻塞，等待下一次按键，若为p/P，则继续游戏
			set_ticker(ticker);//通过设置时钟来恢复游戏
		}else if(ch == 'q' || ch == 'Q'){
			sleep(1);
			endwin();
			exit(0);
		}
	}

	return 0;
}
#endif
