CC = gcc
CFLAGS += -Wall	-lpthread

SRC = ${shell ls *.c}
TARGET = ${shell ls *.c | sed 's/\.c//g'}
		
RESULT_PATH =`pwd`
RESULT=$(RESULT_PATH)/result

include env.mk

all:
	@echo "conpiling ..."
	$(CC) $(SRC) $(GDBFLAG) -o $(RESULT)/$(TARGET) $(CFLAGS)

.PHONY:all clean
clean:
	@echo "clean up"
	rm -rf $(RESULT)/$(TARGET)

