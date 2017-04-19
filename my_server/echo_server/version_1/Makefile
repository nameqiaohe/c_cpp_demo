CC = gcc
CFLAGS += -g -Wall	-lpthread

SERVER_SRC = ${shell ls *.c | grep -v "echo_client.c"}
CLIENT_SRC = ${shell ls *.c | grep -v "echo_server.c"}

# Makefile 内置变量
DIR = $(PWD)
#DIR = ${shell pwd}	# 或者用shell命令获取当前路径
RESULT=result
RESULT_PATH = $(DIR)/$(RESULT)

include env.mk

all:

ifeq ("result", $(wildcard $(RESULT)))	# if语句的前边不能用tab，否则会报错 Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : result"
else
	mkdir -p result
endif

	@echo "compiling ..."
#for i in $(TARGET); do $(CC) $$i.c $(GDBFLAG) -o $(RESULT_PATH)/$$i $(CFLAGS); done
	$(CC) $(SERVER_SRC) -o $(RESULT_PATH)/echo_server
	$(CC) $(CLIENT_SRC) -o $(RESULT_PATH)/echo_client

.PHONY:all clean
clean:
	@echo "clean up"
#for i in $(TARGET); do rm -rf $(RESULT_PATH)/$$i; done
	rm -rf $(RESULT_PATH)/echo_server
	rm -rf $(RESULT_PATH)/echo_client

