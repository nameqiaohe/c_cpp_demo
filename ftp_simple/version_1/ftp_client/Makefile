CC := gcc
CFLAGS := -Wall -g -Os

SHDIR := ../common
OBJS := ftp_client.o $(SHDIR)/common.o

DIR = $(PWD)
RESULT = result
RESULT_PATH = $(DIR)/$(RESULT)

all:ftp_client

ftp_client:$(OBJS)
ifeq ("result", $(wildcard $(RESULT)))  # if语句的前边不能用tab，否则会报错Syntax error: word unexpected (expecting ")")
	@echo "already has the directory : result"                             
else
	@mkdir -p $(RESULT_PATH)
endif

	@$(CC) -o $(RESULT_PATH)/ftp_client $(OBJS) $(CFLAGS)

$(OBJS): %.o:%.c
	@$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	@rm -f ftp_client.o
	@rm -f $(SHDIR)/common.o
	@rm -rf $(RESULT_PATH)
	@echo Cleaning up
