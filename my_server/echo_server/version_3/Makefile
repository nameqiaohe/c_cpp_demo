CC = gcc
CFLAGS += -g -Wall	-lpthread

SERVER_SRC = ${shell ls *.c | grep -v "echo_client.c"}
CLIENT_SRC = ${shell ls *.c | grep -v "echo_server.c"}

# Makefile 内置变量
DIR = $(PWD)
#DIR = ${shell pwd}	# 或者用shell命令获取当前路径
BIN=bin
BIN_PATH = $(DIR)/$(BIN)

include env.mk

all:

ifeq ("bin", $(wildcard $(BIN)))	# if语句的前边不能用tab，否则会报错 Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : bin"
else
	mkdir -p $(BIN)
endif

	@echo "compiling ..."
#for i in $(TARGET); do $(CC) $$i.c $(GDBFLAG) -o $(BIN_PATH)/$$i $(CFLAGS); done
	$(CC) $(SERVER_SRC) -o $(BIN_PATH)/echo_server
	$(CC) $(CLIENT_SRC) -o $(BIN_PATH)/echo_client

.PHONY:all clean
clean:
	@echo "clean up"
#for i in $(TARGET); do rm -rf $(BIN_PATH)/$$i; done
	rm -rf $(BIN_PATH)/echo_server
	rm -rf $(BIN_PATH)/echo_client
	rm -rf $(BIN_PATH)

