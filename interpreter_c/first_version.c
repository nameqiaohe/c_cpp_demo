/*####################################################
# File Name: first_version.c
# Author: YanWei
# Email: nameqiaohe@126.com
# Create Time: 2016-07-20 17:38:57
# Last Modified: 2016-07-20 17:38:57
####################################################*/
/*功能：逐个读取文件，然后输出文件*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;//current token
char *src, *old_str;//pointer to source code string
int poolsize;//default size of text/data/stack
int line;//line number

void next(){
	token = *src++;
	return;
}

void expression(){
	//do nothing
}

void program(){
	next();
	while(token > 0){
		printf("token is : %c\n", token);
		next();
	}
}

int eval(){
	//do nothing yet
	return 0;
}

int main(int argc, char *argv[]){
	int i, fd;

	argc--;
	argv++;

	poolsize = 256 * 1024;
	line = 1;

	if((fd = open(*argv, 0)) < 0){
		perror("open file failed!");
		return -1;
	}

	src = old_str = malloc(poolsize);
	if(!src){
		perror("malloc memory failed!");
		return -1;
	}

	//read source file
	i = read(fd, src, poolsize-1);
	if(i <= 0){
		printf("read file failed, returned %d\n", i);
		return -1;
	}

	src[i] = 0;//add EOF charactor
	close(fd);

	program();

	return eval();
}
