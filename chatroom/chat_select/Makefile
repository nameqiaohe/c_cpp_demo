CC = gcc
CFLAGS += -Wall
LIBS += -lcurses

#SRC = ${shell ls ./src/*.c}
SERVER_SRC = ${shell ls ./src/*.c | grep server}
CLIENT_SRC = ${shell ls ./src/*.c | grep client}
		
RESULT_PATH =`pwd`
#RESULT=$(RESULT_PATH)/result/2048
SERVER_RESULT = $(RESULT_PATH)/result/server
CLIENT_RESULT = $(RESULT_PATH)/result/client

include env.mk

all:
	@echo "make game : 2048"
	$(CC) $(SERVER_SRC) $(GDBFLAG) -o $(SERVER_RESULT) $(CFLAGS) $(LIBS)
	$(CC) $(CLIENT_SRC) $(GDBFLAG) -o $(CLIENT_RESULT) $(CFLAGS) $(LIBS)

.PHONY:all clean
clean:
	@echo "clean up"
	rm -rf $(SERVER_RESULT)
	rm -rf $(CLIENT_RESULT)

