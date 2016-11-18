target = ${shell ls *.c | sed 's/\.c//g'}

all:
	for i in $(target); do gcc -o $$i $$i.c; done

clean:
	for i in $(target); do rm -rf $$i; done
