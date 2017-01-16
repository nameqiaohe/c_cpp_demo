/*####################################################
# File Name: getString.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-26 21:51:48
# Last Modified: 2017-01-16 20:46:53
####################################################*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<mcheck.h>

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

//读取下一个字符
void next(){
	if(markFlag == 2){
		markFlag = 0;
		nextWithoutSpaceAndEnter();
		return;
	}

	token = *srcStr++;
	return;
}

//读取过程中 跳过 空格(Space)、回车/换行(CR/LF)，即在输出结果中去掉了 空格、回车/换行
void nextWithoutSpaceAndEnter(){
	while(srcStr && *srcStr && ((unsigned char)*srcStr <= 32)){
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
	//检测是否有内存泄漏
	setenv("MALLOC_TRACE", "memoryTraceResult", 1);//trace_result是保存检测结果的文件
	mtrace();

	if(argc < 2){
		printf("Usage : ./getString filename\n");
		return 0;
	}

	initValue();

	int fd = open(argv[1], 0);//open the file, named argv[1]
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

	//将读取到的内容输出到文件中，以便查看结果
	FILE *fp = fopen("readResult.txt", "w");
	if(fp == NULL){
		perror("open file failed!");
		exit(-1);
	}
	fputs(posRes, fp);
	fclose(fp);

	free(posSrc);
	posSrc = NULL;
	free(posRes);
	posRes = NULL;

	mtrace();
	return 0;
}
