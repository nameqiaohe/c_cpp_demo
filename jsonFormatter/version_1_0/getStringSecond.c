/*####################################################
# File Name: getString.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-26 21:51:48
# Last Modified: 2017-01-16 23:09:58
####################################################*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<mcheck.h>
#include <strings.h>

char token;
int markFlag;

char *next(char *str);
char *nextWithoutSpaceAndEnter(char *str);

//读取下一个字符
char *next(char *str){
	if(markFlag == 2){
		markFlag = 0;
		return nextWithoutSpaceAndEnter(str);
	}

	token = *str++;
	return str;
}

//读取过程中 跳过 空格(Space)、回车/换行(CR/LF)，即在输出结果中去掉了 空格、回车/换行
char *nextWithoutSpaceAndEnter(char *str){
	while(str && *str && ((unsigned char)*str <= 32)){
		str++;
	}
	token = *str;
	return str;
}

void process(char *src, char **result){
	src = next(src);
	printf("token is : %c\n", token);
	**result++ = token;

	while(token > 0){
		if(token == '"'){
			markFlag++;
			src = next(src);
		}else if(markFlag > 0){
			src = next(src);
		}else{
			src = nextWithoutSpaceAndEnter(src);
		}
		printf("token is : %c\n", token);
		**result++ = token;
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

	FILE *fp = fopen(argv[1], "r");//open file, named argv[1]
	if(fp == NULL){
		perror("open file failed");
		exit(-1);
	}

	int fileSize = 0;
	fseek(fp,0L,SEEK_END);  
	fileSize = ftell(fp);
	char *str = (char *)malloc(fileSize);
	bzero(str, 0);
	char *ptrStr = str;

	//(void)fseek(fp, 0L, SEEK_SET);
	rewind(fp);

	char ch;
	while((ch = fgetc(fp)) != EOF){
		*str++ = ch;
	}
	*str = 0;//add EOF charactor to the end
	fclose(fp);//close the file

	markFlag = 0;

	char *resultStr = (char *)malloc(fileSize);
	process(ptrStr, &resultStr);//process the string, delete the external space
	printf("resultStr is : %s\n", resultStr);

	//将读取到的内容输出到文件中，以便查看结果
	FILE *fpWrite = fopen("readResult.txt", "w");
	if(fpWrite == NULL){
		perror("open file failed!");
		exit(-1);
	}
	fputs(resultStr, fpWrite);
	fclose(fpWrite);

	free(str);
	str = NULL;
	free(ptrStr);
	ptrStr = NULL;

	mtrace();
	return 0;
}
