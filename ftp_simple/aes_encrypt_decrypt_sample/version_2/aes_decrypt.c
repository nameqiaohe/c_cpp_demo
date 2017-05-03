/*####################################################
# File Name: aes_decrypt.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-02 00:03:53
# Last Modified: 2017-05-03 12:49:12
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

	long file_len = 0;

	unsigned int len;//encrypt length (in multiple of AES_BLOCK_SIZE)
	unsigned int i;

	if(argc != 3){
		fprintf(stderr, "Usage : %s <encrypt_filename> <iv_file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char *encrypt_filename = argv[1];

	char temp;

	FILE *fp_encrypt = fopen(encrypt_filename, "r");
	if(fp_encrypt == NULL){
		fprintf(stderr, "Unable to open the file %s\n", encrypt_filename);
		exit(EXIT_FAILURE);
	}
	while(fread(&temp, 1, 1, fp_encrypt) == 1){
		file_len++;
	}

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

	i = 0;
	while(fread(&input_str[i], 1, 1, fp_encrypt) == 1){
		i++;
	}

	fclose(fp_encrypt);

	char *iv_file = argv[2];

	FILE *fp_ivec = fopen(iv_file, "r");
	if(fp_ivec == NULL){
		fprintf(stderr, "Unable to open the file %s\n", iv_file);
		exit(EXIT_FAILURE);
	}
	file_len = 0;
	while(fread(&temp, 1, 1, fp_ivec) == 1){
		file_len++;
	}

	fseek(fp_ivec, 0L, SEEK_SET);
	i = 0;
	while(fread(&iv[i], 1, 1, fp_ivec) == 1){
		i++;
	}
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
	printf("decrypt string : %s\n", decrypt_str);

	return 0;
}
