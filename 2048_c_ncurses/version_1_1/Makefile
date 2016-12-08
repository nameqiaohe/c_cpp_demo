CC = gcc
CFLAGS += -Wall
LIBS += -lcurses

SRC = ${shell ls *.c}
RESULT = 2048

include env.mk

all:
	@echo "make game : 2048"
	$(CC) $(SRC) $(GDBFLAG) -o $(RESULT) $(CFLAGS) $(LIBS)

.PHONY:all clean

clean:
	rm $(RESULT)
