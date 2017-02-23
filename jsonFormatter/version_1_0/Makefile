CC = gcc
CFLAGS += -Wall	-lpthread

SRC = ${shell ls *.c}
TARGET = ${shell ls *.c | sed 's/\.c//g'}
		
RESULT_PATH =`pwd`
RESULT=$(RESULT_PATH)/result

include env.mk

all:
	@echo "conpiling ..."
	for i in $(TARGET); do $(CC) $$i.c $(GDBFLAG) -o $(RESULT)/$$i $(CFLAGS); done

.PHONY:all clean
clean:
	@echo "clean up"
	for i in $(TARGET); do rm -rf $(RESULT)/$$i; done
	rm -rf $(RESULT)/readResult*
	rm -rf $(RESULT)/memoryTrace*

