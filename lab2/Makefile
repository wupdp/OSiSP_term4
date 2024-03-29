CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
.PHONY:clean

all: parent child

parent: parent.c Makefile
	$(CC) $(CFLAGS) parent.c -o daddy
	
child: child.c Makefile
	$(CC) $(CFLAGS) child.c -o son

clean:
	rm -f daddy son
