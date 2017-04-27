/*####################################################
# File Name: multi_thread_bucket_sort_new.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-04-26 22:13:53
# Last Modified: 2017-04-27 13:56:45
####################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mcheck.h>

#define UPPER_INFINITE 999999999 //10^10 - 1
#define LOWER_INFINITE -1

#define MAX 32

typedef struct thread_param{
	int id;

	int upper;
	int lower;
	int count;
	int upper_value;
	int lower_value;

	int *unsorted_data;
	int unsorted_len;
	int *sorted;//排好序的数据地址
}PARAM;

int compare(const void *a, const void *b){
	return *(int *)a - *(int *)b;
}

void *sort(void *arg){
	PARAM param = *(PARAM *)arg;

	int *local_arr = (int *)malloc(sizeof(int) *param.count);

	int index = 0;
	int i = 0;
	for(i = 0; i < param.unsorted_len; i++){
		if(param.unsorted_data[i] <= param.upper_value && param.unsorted_data[i] > param.lower_value){
			local_arr[index] = param.unsorted_data[i];
			index++;
		}
	}

	qsort(local_arr, param.count, sizeof(int), compare);
	index = 0;

	for(i = param.lower; i < param.upper; i++){
		param.sorted[i] = local_arr[index];
		index++;
	}

	//free(local_arr);
	return (void *)0;
}

/* 从 filename中读取数据，存储到 arr数组中 */
int *read_data_from_file(char *filename, int *length){
	int temp;
	int total_num = 0;

	/* 从文件中获取数据 */
	FILE *fp = fopen(filename, "r");
	if(fp == NULL){
		printf("read_data_from_file : file can not be opened, before cal count!\n");
		exit(EXIT_FAILURE);
	}else{
		while(!feof(fp)){
			fscanf(fp, "%d", &temp);
			total_num++;
		}
		printf("file %s has %d integers!\n", filename, total_num);
	}

	length[0] = total_num;
	fclose(fp);

	/* 将读取到的数据拷贝至申请的数组中 */
	int *arr = (int *)malloc(total_num * sizeof(int));
	if(arr == NULL){
		printf("malloc space failed!\n");
		exit(EXIT_FAILURE);
	}
	int index = 0;
	if((fp = fopen(filename, "r")) == NULL){
		printf("read_data_from_file : file can not be opened, before assign arr!\n");
		exit(EXIT_FAILURE);
	}else{
		while(!feof(fp)){
			fscanf(fp, "%d", arr+index);
			index++;
		}
	}
	fclose(fp);

	return arr;
}

//void find_splitters(int *arr, int *splitter_candidate, int *final_splitter, int arr_len, int k){
void find_splitters(int arr[], int splitter_candidate[], int *final_splitter, int arr_len, int k){
	int bucket_gap = arr_len / (k*k);

	int temp_bucket_len;
	int i = 0;
	for(i = 0; i < k*k; i++){
		if(i%k == 0){
			temp_bucket_len = (i+bucket_gap*k > arr_len) ? arr_len-i : bucket_gap*k;
			printf("find_splitters : temp_bucket_len = %d\n", temp_bucket_len);

			qsort(arr+i, temp_bucket_len, sizeof(int), compare);
		}
	}

	int index = 0;//splitter candidate 1 index
	for(i = 0; i < k*k; i++){
		if(i%k != 0){
			splitter_candidate[index] = arr[i*bucket_gap];
			index++;
		}
	}

	//对 splitter_candidate 进行排序
	qsort(splitter_candidate, k*(k-1), sizeof(int), compare);

	//确定最终的 splitter
	for(i = 1; i < k; i++){
		final_splitter[i-1] = splitter_candidate[i*k-1];
	}

	printf("find_splitters : final splitter is : \n");
	for(i = 0; i < k-1; i++){
		printf("%d, ", final_splitter[i]);
	}
}

void count_elements_each_bucket(int *arr, int *final_splitter, int *elements_in_bucket, int arr_len, int k){
	int i = 0;
	for(i = 0; i < k+1; i++){//初始化每个桶中的数据
		elements_in_bucket[i] = 0;
	}

	int flag = 0;
	int j = 0;
	for(i = 0; i <arr_len; i++){
		flag = 0;
		for(j = 0; j < k-1; j++){
			if(final_splitter[j] >= arr[i]){
				flag = 1;
				elements_in_bucket[j]++;
				break;
			}
		}

		if(flag == 0){
			elements_in_bucket[k-1]++;
		}
	}
}

void find_arr_bound_in_sorted(int *elements_in_bucket, int *arr_bound_of_buckets, int k){
	arr_bound_of_buckets[0] = 0;
	int i = 0;
	for(i = 1; i < k+1; i++){
		arr_bound_of_buckets[i] = arr_bound_of_buckets[i-1] + elements_in_bucket[i-1];
	}
}


int main(int argc, char *argv[]){
	setenv("MALLOC_TRACE", "memoryTraceResult.txt", 1);//trace_result是保存检测结果的文件 
	mtrace();

	int k;//the number of the threads to create
	char *filename;//the filename of data file to be stored

	/* 获取用户输入：创建的线程数、存储数据的文件名 */
	if(argc != 3){
		printf("please check your parameters input!\nUsage : %s <filename> <thread_counts>\n", argv[0]);
		exit(EXIT_FAILURE);
	}else{
		filename = argv[1];
		k = atoi(argv[2]);
		//filename = *(argv+1);
		//k = atoi(*(argv+2));
	}

	/* 读取数据到数组中 */
	int *arr_len = (int *)malloc(sizeof(int));
	int *arr = read_data_from_file(filename, arr_len);

	/* 选取splitter */
	int *splitter_candidate;
	int *final_splitter;
	splitter_candidate = (int *)malloc(sizeof(int)*k*(k-1));
	final_splitter = (int *)malloc(sizeof(int)*(k-1));

	find_splitters(arr, splitter_candidate, final_splitter, arr_len[0], k);

	/* 算出每个桶中的数据个数 */
	int *elements_in_bucket = (int *)malloc(sizeof(int)*k);
	count_elements_each_bucket(arr, final_splitter, elements_in_bucket, arr_len[0], k);

	/* 每个桶的数据范围 */
	int *sorted_arr = (int *)malloc(sizeof(int)*arr_len[0]);
	int *sorted_arr_bound = (int *)malloc(sizeof(int)*(k-1));

	find_arr_bound_in_sorted(elements_in_bucket, sorted_arr_bound, k);

	PARAM *param_list = (PARAM *)malloc(sizeof(PARAM)*k);
	pthread_t pid[MAX];

	int i = 0;
	for(i = 0; i < k; i++){
		PARAM param;
		param.id = i;

		param.upper = sorted_arr_bound[i+1];
		param.lower = sorted_arr_bound[i];
		param.count = elements_in_bucket[i];

		param.unsorted_data = arr;
		param.unsorted_len = arr_len[0];
		param.sorted = sorted_arr;

		if(i == 0){
			param.upper_value = final_splitter[i];
			param.lower_value = LOWER_INFINITE;
		}else if(i == k-1){
			param.upper_value = UPPER_INFINITE;
			param.lower_value = final_splitter[i-1];
		}else{
			param.upper_value = final_splitter[i];
			param.lower_value = final_splitter[i-1];
		}

		if(i == 0 && i == k-1){
			param.upper_value = UPPER_INFINITE;
			param.lower_value = LOWER_INFINITE;
		}
		param_list[i] = param;
	}

	for(i = 0; i <k; i++){
		pthread_create(&pid[i], NULL, sort, (void *)(param_list + i));
	}

	for(i = 0; i < k; i++){
		pthread_join(pid[i], NULL);
	}

	/* 将排好序的数据写到文件中 */
	FILE *fp_write = fopen("./result/sort_data_file", "w");
	for(i = 0; i < arr_len[0]; i++){
		fprintf(fp_write, "%d\n", sorted_arr[i]);
	}
	fclose(fp_write);
	printf("write the sorted data to file successfully!\n");

	free(arr_len);
	free(arr);
	free(splitter_candidate);
	free(final_splitter);
	free(elements_in_bucket);
	free(sorted_arr);
	free(sorted_arr_bound);
	free(param_list);

	muntrace();
	return 0;
}
