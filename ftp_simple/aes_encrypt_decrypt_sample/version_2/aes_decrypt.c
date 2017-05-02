/*####################################################
# File Name: aes_decrypt.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-02 00:03:53
# Last Modified: 2017-05-02 13:33:16
####################################################*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <openssl/aes.h>

int main(int argc, char *argv[]){
	AES_KEY aes;
	unsigned char key[AES_BLOCK_SIZE];//AES_BLOCK_SIZE = 16
	unsigned char iv[AES_BLOCK_SIZE];//init vector

	unsigned char *input_str;
	unsigned char *decrypt_str;

	long file_len;

	unsigned int len;//encrypt length (in multiple of AES_BLOCK_SIZE)
	unsigned int i;

	if(argc != 2){
		fprintf(stderr, "Usage : %s <filename>\n", argv[0]);
		exit(EXIT_FAILURE);
	}


	FILE *fp_encrypt = fopen("./bin/encrypt_str.txt", "r");
	if(fp_encrypt == NULL){
		fprintf(stderr, "Unable to open the file ./bin/encrypt_str.txt\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp_encrypt, 0L, SEEK_END);
	file_len = ftell(fp_encrypt);

	//set the encryption length
	len = 0;
	int ret_val = (file_len + 1) % AES_BLOCK_SIZE;
	if(ret_val == 0){
		len = file_len + 1;
	}else{
		len = ((file_len + 1) / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
	}

	//set the input string
	input_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	if(input_str == NULL){
		fprintf(stderr, "Unable to allocate memory for input_str\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp_encrypt, 0L, SEEK_SET);

	char temp;
	for(i = 0; i < len; ++i){
		fscanf(fp_encrypt, "%c", &temp);
		printf("input string : %c\n", temp);

	}
	fclose(fp_encrypt);

#if 0
	char cur_real_path[200];
	realpath(argv[0], cur_real_path);

	char *cur_path = dirname(cur_real_path);
	printf("cur_path : %s\n", cur_path);
	char *ivec_path = dirname(cur_path);
	printf("ivec_path : %s\n", ivec_path);
	strcat(ivec_path, "/ivec");
#endif

	FILE *fp_ivec = fopen("/home/wei/2016/c_cpp_demo/ftp_simple/aes_encrypt_decrypt_sample/version_2/ivec", "r");
	if(fp_ivec == NULL){
		fprintf(stderr, "Unable to open the file ../ivec\n");
		exit(EXIT_FAILURE);
	}
	fseek(fp_ivec, 0L, SEEK_END);
	file_len = ftell(fp_ivec);
	fseek(fp_ivec, 0L, SEEK_SET);
	fread(iv, 1, file_len-1, fp_ivec);
	printf("iv : %s\n", iv);

	fclose(fp_ivec);


	//generate AES 128-bit key
	for(i = 0; i < AES_BLOCK_SIZE; ++i){
		key[i] = 32 + i;
	}

	//alloc decrypt string
	decrypt_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	if(decrypt_str == NULL){
		fprintf(stderr, "Unable to allocate memory for decrypt_str\n");
		exit(EXIT_FAILURE);		
	}

	ret_val = AES_set_decrypt_key(key, 128, &aes);
	if(ret_val < 0){
		fprintf(stderr, "Unable to set decryption key in AES\n");
		exit(EXIT_FAILURE);
	}

	//decrypt
	AES_cbc_encrypt(input_str, decrypt_str, len, &aes, iv, AES_DECRYPT);

	//print
	printf("input string : %s\n", input_str);

	printf("decrypt string : %s\n", decrypt_str);

	return 0;
}
