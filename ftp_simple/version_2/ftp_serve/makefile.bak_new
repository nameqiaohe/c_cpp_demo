CC := gcc
CFLAGS := -Wall -g -Os

TARGET = ftp_serve
SHDIR := ../common

SRC := ${shell ls *.c}
SRC += ${shell ls $(SHDIR)/*.c}

DIR = $(PWD)
RESULT = result
RESULT_PATH = $(DIR)/$(RESULT)

all:$(TARGET)

$(TARGET):
ifeq ("result", $(wildcard $(RESULT)))  # if语句的前边不能用tab，否则会报错Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : result"                             
else
	@mkdir -p $(RESULT_PATH)
endif

	@$(CC) -o $(RESULT_PATH)/$(TARGET) $(SRC) $(CFLAGS)

.PHONY:
clean:
	@rm -rf $(RESULT_PATH)
	@echo Cleaning up
