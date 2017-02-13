/*####################################################
# File Name: first_version.c
# Author: YanWei
# Email: nameqiaohe@126.com
# Create Time: 2016-07-22 22:38:57
# Last Modified: 2016-07-22 22:38:57
####################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;//current token
char *src, *old_str;//pointer to source code string
int poolsize;//default size of text/data/stack
int line;//line number

/* start of second version addation */
int *text,	//text segment
	*old_text,	//for dump text segment  ??????
	*stack;	//stack
char *data;//data segment

int *pc, *bp, *sp, ax, cycle;	//virtual machine registers
/* end of second version addation */

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

	/* start of second version addation */
	//allocate memory for virtual mechine
	text = old_text = malloc(poolsize);
	if(!text){
		perror("allocate memory for text area failed!");
		return -1;
	}

	data = malloc(poolsize);
	if(!data){
		perror("allocate memory for data area failed!");
		return -1;
	}

	stack = malloc(poolsize);
	if(!stack){
		perror("allocate memory for stack failed!");
		return -1;
	}

	memset(text, 0, poolsize);
	memset(data, 0, poolsize);
	memset(stack, 0, poolsize);

	bp = sp = (int *)((int)stack + poolsize);
	ax = 0;
	/* end of second version addation */

	program();

	return eval();
}
