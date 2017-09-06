CC = gcc
CFLAGS = -Wall -g -O2
LIBS = -lcrypto

DIR = $(PWD)
BIN = bin
BIN_PATH = $(DIR)/$(BIN)
ENCRYPT_TARGET = encrypt
DECRYPT_TARGET = decrypt

all:
ifeq ("bin", $(wildcard $(BIN)))  # if语句的前边不能用tab，否则会报错Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : $(BIN)"
else
	@mkdir -p $(BIN_PATH)
endif

	$(CC) -o $(BIN_PATH)/$(ENCRYPT_TARGET) aes_encrypt.c $(LIBS) $(CFLAGS)
	$(CC) -o $(BIN_PATH)/$(DECRYPT_TARGET) aes_decrypt.c $(LIBS) $(CFLAGS)
	
.PHONY:clean
clean:
	@rm -rf $(BIN_PATH)
