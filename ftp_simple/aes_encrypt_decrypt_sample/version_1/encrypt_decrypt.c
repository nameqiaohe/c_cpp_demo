/*####################################################
# File Name: encrypt_decrypt.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-02 00:03:53
# Last Modified: 2017-05-02 00:29:56
####################################################*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/aes.h>

int main(int argc, char *argv[]){
	AES_KEY aes;
	unsigned char key[AES_BLOCK_SIZE];//AES_BLOCK_SIZE = 16
	unsigned char iv[AES_BLOCK_SIZE];//init vector

	unsigned char *input_str;
	unsigned char *encrypt_str;
	unsigned char *decrypt_str;

	unsigned int len;//encrypt length (in multiple of AES_BLOCK_SIZE)
	unsigned int i;

	if(argc != 2){
		fprintf(stderr, "Usage : %s <plain text>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	//set the encryption length
	len = 0;
	int ret_val = (strlen(argv[1]) + 1) % AES_BLOCK_SIZE;
	if(ret_val == 0){
		len = strlen(argv[1]) + 1;
	}else{
		ret_val = (strlen(argv[1]) + 1) / AES_BLOCK_SIZE + 1;
		len = ret_val * AES_BLOCK_SIZE;
	}

	//set the input string
	input_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	if(input_str == NULL){
		fprintf(stderr, "Unable to allocate memory for input_str\n");
		exit(EXIT_FAILURE);
	}
	strncpy((char *)input_str, argv[1], strlen(argv[1]));

	//generate AES 128-bit key
	for(i = 0; i < AES_BLOCK_SIZE; ++i){
		key[i] = 32 + i;
	}

	//set encrypt key
	for(i = 0; i < AES_BLOCK_SIZE; ++i){
		iv[i] = 0;
	}
	
	if(AES_set_encrypt_key(key, 128, &aes) < 0){
		fprintf(stderr, "Unable to set encryption key in AES\n");
		exit(EXIT_FAILURE);
	}

	//alloc encrypt string
	encrypt_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	if(encrypt_str == NULL){
		fprintf(stderr, "Unable to allocate memory for encrypt_str\n");
		exit(EXIT_FAILURE);		
	}

	//encrypt ( iv will change )
	AES_cbc_encrypt(input_str, encrypt_str, len, &aes, iv, AES_ENCRYPT);

	//alloc decrypt string
	decrypt_str = (unsigned char *)calloc(len, sizeof(unsigned char));
	if(decrypt_str == NULL){
		fprintf(stderr, "Unable to allocate memory for decrypt_str\n");
		exit(EXIT_FAILURE);		
	}

	//set decrypt key
	for(i = 0; i < AES_BLOCK_SIZE; ++i){
		iv[i] = 0;
	}

	ret_val = AES_set_decrypt_key(key, 128, &aes);
	if(ret_val < 0){
		fprintf(stderr, "Unable to set decryption key in AES\n");
		exit(EXIT_FAILURE);
	}

	//decrypt
	AES_cbc_encrypt(encrypt_str, decrypt_str, len, &aes, iv, AES_DECRYPT);

	//print
	printf("input string : %s\n", input_str);
	printf("encrypt string : ");
	for(i = 0; i < len; ++i){
		printf("%x%x", (encrypt_str[i] >> 4) & 0xf, encrypt_str[i] & 0xf);
	}
	printf("\n");

	printf("decrypt string : %s\n", decrypt_str);

	return 0;
}
