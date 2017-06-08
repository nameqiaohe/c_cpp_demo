/*####################################################
# File Name: getString.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2016-12-26 21:51:48
# Last Modified: 2017-02-23 21:12:14
####################################################*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<mcheck.h>
#include <string.h>
#include <libgen.h>

#define LEN 100

char token;
int markFlag;

char *next(char *str);
char *nextWithoutSpaceAndEnter(char *str);
void setResultFilePath(char *dst, char *dirPath, const char *filename);

void setResultFilePath(char *dst, char *dirPath, const char *filename){
	strncpy(dst, dirPath, strlen(dirPath));
	strncat(dst, filename, strlen(filename));
}

//读取下一个字符
char *next(char *str){
	if(markFlag == 2){
		markFlag = 0;
		return nextWithoutSpaceAndEnter(str);
	}

	token = *str;
	return ++str;
}

//读取过程中 跳过 空格(Space)、回车/换行(CR/LF)，即在输出结果中去掉了 空格、回车/换行
char *nextWithoutSpaceAndEnter(char *str){
	while(str && *str && ((unsigned char)*str <= 32)){
		str++;
	}
	token = *str;
	return ++str;
}

void process(char *src, char *result){
	char *ptr = src;
	ptr = next(ptr);

	printf("=== 1 === token is : %c\n", token);
	*result = token;
	result++;

	while(token > 0){
		if(token == '"'){
			markFlag++;
			ptr = next(ptr);
		}else if(markFlag > 0){
			ptr = next(ptr);
		}else{
			ptr = nextWithoutSpaceAndEnter(ptr);
		}
		printf("=== 2 === token is : %c\n", token);
		*result = token;
		result++;
	}
}

int main(int argc, char *argv[]){
	char *dirPath = dirname(argv[0]);

	char memoryTraceResultFile[LEN] = {0};
	setResultFilePath(memoryTraceResultFile, dirPath, "/memoryTraceResult.txt");
	//检测是否有内存泄漏
	//setenv("MALLOC_TRACE", "memoryTraceResult", 1);//trace_result是保存检测结果的文件
	setenv("MALLOC_TRACE", memoryTraceResultFile, 1);//trace_result是保存检测结果的文件
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
	char *str = (char *)malloc(fileSize+1);
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
	bzero(resultStr, 0);
	char *ptrRes = resultStr;

	process(ptrStr, resultStr);//process the string, delete the external space
	printf("resultStr is : %s\n", ptrRes);

	char readResultFile[LEN] = {0};
	setResultFilePath(readResultFile, dirPath, "/readResult.txt");
	//将读取到的内容输出到文件中，以便查看结果
	//FILE *fpWrite = fopen("readResult.txt", "w");
	FILE *fpWrite = fopen(readResultFile, "w");
	if(fpWrite == NULL){
		perror("open file failed!");
		exit(-1);
	}
	fputs(ptrRes, fpWrite);
	fclose(fpWrite);

	//释放内存
	free(ptrStr);
	str = NULL;
	free(ptrRes);
	ptrRes = NULL;

	mtrace();
	return 0;
}
