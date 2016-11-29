SRC = ${shell ls *.c}
FLAG = -lcurses
RESULT = 2048

all:
	gcc $(SRC) -o $(RESULT) $(FLAG)

.PHONY:all clean

clean:
	rm $(RESULT)
