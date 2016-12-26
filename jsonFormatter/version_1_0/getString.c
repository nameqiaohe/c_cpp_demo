/*####################################################
# File Name: getString.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-26 21:51:48
# Last Modified: 2016-12-26 22:37:57
####################################################*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *srcStr, *posSrc, *resultStr, *posRes;
char token;
int poolsize;
int markFlag;

void initValue();
void next();
void nextWithoutSpaceAndEnter();

void initValue(){
	poolsize = 512 * 1024;
	srcStr = (char *)malloc(poolsize);
	if(!srcStr){//if(srcStr == NULL)
		perror("malloc memory failed!");
		exit(-1);
	}
	posSrc = srcStr;

	resultStr = (char *)malloc(poolsize);
	if(!resultStr){//if(srcStr == NULL)
		perror("malloc memory failed!");
		exit(-1);
	}
	posRes = resultStr;

	markFlag = 0;
}

void next(){
	if(markFlag == 2){
		markFlag = 0;
		nextWithoutSpaceAndEnter();
		return;
	}

	token = *srcStr++;
	return;
}

void nextWithoutSpaceAndEnter(){
	while(*srcStr == ' ' || *srcStr == '\n'){
		srcStr++;
	}
	token = *srcStr++;
	return;
}

void process(){
	next();
	printf("token is : %c\n", token);
	*resultStr++ = token;

	while(token > 0){
		if(token == '"'){
			markFlag++;
			next();
		}else if(markFlag > 0){
			next();
		}else{
			nextWithoutSpaceAndEnter();
		}
		printf("token is : %c\n", token);
		*resultStr++ = token;
	}
}

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("Usage : ./getString filename\n");
		return 0;
	}

	initValue();

	int fd = open(argv[1], 0);
	if(fd < 0){
		perror("open file failed!");
		exit(-1);
	}

	int i = 0;
	i = read(fd, srcStr, poolsize-1);
	if(i <= 0){
		perror("read file failed!");
		exit(-1);
	}

	srcStr[i] = 0;//add EOF charactor to the end
	close(fd);//close the file

	process();//process the string, delete the external space
	printf("resultStr is : %s\n", posRes);

	free(posSrc);
	posSrc = NULL;
	free(posRes);
	posRes = NULL;

	return 0;
}
