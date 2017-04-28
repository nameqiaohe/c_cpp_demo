CC := gcc
CFLAGS := -Wall -g -Os

TARGET = ftp_serve
SHDIR := ../common
OBJS := ftp_serve.o $(SHDIR)/common.o

DIR = $(PWD)
RESULT = result
RESULT_PATH = $(DIR)/$(RESULT)

all:$(TARGET)

$(TARGET):$(OBJS)
ifeq ("result", $(wildcard $(RESULT)))  # if语句的前边不能用tab，否则会报错Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : result"                             
else
	@mkdir -p $(RESULT_PATH)
endif

	@$(CC) -o $(RESULT_PATH)/$(TARGET) $(OBJS) $(CFLAGS)

$(OBJS): %.o:%.c
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f $(SHDIR)/common.o
	@rm -f ftp_serve.o
	@rm -rf $(RESULT_PATH)
	@echo Cleaning up
