CC := gcc
CFLAGS := -Wall -g -Os
LIBS = -lcrypto

TARGET = ftp_serve
SHDIR := ../common

SRC := ${shell ls *.c}
SRC += ${shell ls $(SHDIR)/*.c}

DIR = $(PWD)
BIN = bin
BIN_PATH = $(DIR)/$(BIN)

all:$(TARGET)

$(TARGET):
ifeq ("bin", $(wildcard $(BIN)))  # if语句的前边不能用tab，否则会报错Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : bin"                             
else
	@mkdir -p $(BIN_PATH)
endif

	@$(CC) -o $(BIN_PATH)/$(TARGET) $(SRC) $(CFLAGS) $(LIBS)

.PHONY:
clean:
	@rm -rf $(BIN_PATH)
	@echo Cleaning up
