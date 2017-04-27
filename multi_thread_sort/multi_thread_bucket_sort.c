/*####################################################
# File Name: multi_thread_bucket_sort.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-26 00:09:54
# Last Modified: 2017-04-26 18:03:11
####################################################*/
/* 桶排序
 * 1、桶个数
 * 2、找最大值、最小值，计算每个桶的数据范围
 * 3、遍历，将每个元素分桶
 *		分桶：(int)((arr[i]-min+1)/range)
 * 4、每个桶内做排序
 *		调用排序算法
 * 5、合并每个桶中的数据
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define BUCKET_SIZE 20	//桶大小
#define BUCKET_NUM 5	//桶个数

//bubble_sort
void sort(int arr[], int arr_len){
	int i = 0, j = 0;
	int temp;

	for(i = 0; i < arr_len; i++){
		for(j = 0; j < arr_len-i-1; j++){
			if(arr[j] > arr[j+1]){
				temp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = temp;
			}
		}
	}
}

void clear_arr(int arr[], int arr_len){
	int i = 0;
	for(i = 0; i < arr_len; i++){
		arr[i] = 0;
	}
}

typedef struct param{
	int max;
	int min;
	int arr[BUCKET_SIZE];
	int arr_len;
}PARAM;

void param_init(PARAM *param, int max, int min, int arr[], int arr_len){
	param->max = max;
	param->min = min;
	param->arr_len = arr_len;

	int i = 0;
	for(i = 0; i < arr_len; i++){
		param->arr[i] = arr[i];
	}

	for(i = arr_len; i < BUCKET_SIZE; i++){
		param->arr[i] = 0;
	}

	for(i = 0; i < BUCKET_SIZE; i++){
		printf("%d-", param->arr[i]);
	}
	printf("\n");
}

void *thread_func(void *arg){
	PARAM *param = (PARAM *)arg;
	
	sort(param->arr, param->arr_len);
}

void bucket_sort(int arr[], int arr_len){
	clock_t start, finish;
	double duration;

	start = clock();

	int buckets[BUCKET_NUM][BUCKET_SIZE] = {0};//注意每个桶的大小要大于等于放入该桶中的数据个数

	int i, j, k;
	int bucket_index;
	int max = arr[0];
	int min = arr[0];

	/* 找出最大值、最小值，计算每个桶的范围 */
	for(i = 1; i < arr_len; i++){
		min = min <= arr[i] ? min : arr[i];
		max = max >= arr[i] ? max : arr[i];
	}
	double range = (max - min + 1)/BUCKET_NUM;
	printf("------max = %d\n", max);
	printf("------min = %d\n", min);
	printf("------range = %f\n", range);

	//遍历，将值分到对应的桶中
	for(i = 0; i < arr_len; i++){
		bucket_index = (int)((arr[i] - min)/range);//计算每个值应该放在第几个桶
		if(bucket_index >= BUCKET_NUM){//避免索引超过桶的个数
			bucket_index = BUCKET_NUM - 1;
		}
		printf("------bucket_index = %d,	push data : %d\n", bucket_index, arr[i]);

		for(j = 0; j < BUCKET_SIZE; j++){//如果有多个值放入同一个桶中，避免覆盖，应向后追加
			if(buckets[bucket_index][j] == 0){//不等于0，表示给位置上没有值
				buckets[bucket_index][j] = arr[i];
				break;
			}							
		}
	}

	k = 0;
	int bucket_real_len = 0;
	pthread_t tid[BUCKET_NUM];
	PARAM param[BUCKET_NUM];
	//多线程 桶内作排序，排好序的值对应放回原数组
	for(i = 0; i < BUCKET_NUM; i++){//若每个桶中的值不止一个，需要在该桶内做一个排序
		bucket_real_len = 0;
		for(j = 0; j < BUCKET_SIZE; j++){//计算该桶中有几个值
			if(buckets[i][j] == 0){
				break;
			}else{
				bucket_real_len++;
			}
		}

		param_init(&param[i], max, min, buckets[i], bucket_real_len);

		int err = pthread_create(&tid[i], NULL, thread_func, (void *)&param[i]);
	}
	//主线程等待直到子线程排序完毕
	for(i = 0; i < BUCKET_NUM; i++){
		pthread_join(tid[i], NULL);
	}

	//合并数据
	for(i = 0; i < BUCKET_NUM; i++){
		//将排好序的值 放入原数组中对应的位置
		for(j = 0; j < BUCKET_SIZE; j++){
			if(param[i].arr[j] == 0){
				break;
			}else{
				arr[k++] = param[i].arr[j];
			}
		}
		//清理桶中的数据，若是动态分配的内存，需要清理数据
		//clear_arr(buckets[i], bucket_real_len);
	}	

	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	printf( "********************** cost %f seconds\n", duration );
}

/* 打开文件，并检验是否打开成功 */
FILE *fopen_file(char *filename, char *mode){
	FILE *fp = fopen(filename, mode);
	if(fp == NULL){
		printf("fopen_file : open file failed!\n");
		exit(EXIT_FAILURE);
	}
	return fp;
}

/* 读取文件，计算共有多少个数字 */
int cal_data_length(FILE *fp){
	if(fp == NULL){
		printf("cal_data_num : fp is invalid!\n");
		exit(EXIT_FAILURE);
	}

	rewind(fp);//确保文件指针是在文件开头

	int data_length = 0;
	char buffer[80] = {0};
	while(fgets(buffer, 80, fp) != NULL){
		data_length++;
	}
	return data_length;
}

/* 读取文件中的数据，填入分配好的内存中，即数组 */
void assign_arr(FILE *fp, int arr[]){
	if(fp == NULL){
		printf("cal_data_num : fp is invalid!\n");
		exit(EXIT_FAILURE);
	}

	rewind(fp);//确保文件指针是在文件开头

	char buffer[80] = {0};
	int index = 0;
	while(fgets(buffer, 80, fp) != NULL){
		sscanf(buffer, "%d", &arr[index]);
		index++;
	}
}

int *malloc_space(int length){
	int *arr = (int *)malloc(length);
	if(arr == NULL){
		printf("malloc_func : malloc space failed!\n");
		exit(EXIT_FAILURE);
	}
	return arr;
}

/* 从文件中读取数据，存入数组中 
 * 文件中的数据是按照每行一个数字存放的
*/
int main(int argc, char *argv[]){
	setenv("MALLOC_TRACE", "memoryTraceResult.txt", 1);//trace_result是保存检测结果的文件 
	mtrace();

	if(argc < 2){
		printf("Usage : %s <filename>\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	char *filename = argv[1];
	FILE *fp;
	fp = fopen_file(filename, "r");
	
	int data_length = cal_data_length(fp);
	int *arr = malloc_space(data_length*sizeof(int));

	assign_arr(fp, arr);

	fclose(fp);

	bucket_sort(arr, data_length);

	int i;
	for(i = 0; i < data_length; i++){
		printf("%d, ", arr[i]);
	}
	printf("\n");

	free(arr);
	muntrace();
	return 0;
}
