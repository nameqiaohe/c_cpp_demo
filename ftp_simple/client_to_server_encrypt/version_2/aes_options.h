/*####################################################
# File Name: aes_options.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-02 23:26:55
# Last Modified: 2017-05-02 23:27:11
####################################################*/
#ifndef _ASE_H_
#define _ASE_H_

int encrypt(char *input_string, char **encrypt_string);
void decrypt(char *encrypt_string, char **decrypt_string, int len);

#endif
