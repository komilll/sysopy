CC = gcc -Wall -std=gnu11 -O3

all: clean
	$(CC) main.c -o main

test: all
	@echo -e "\nStart test\n"
	./main .

clean:
	rm -f *.o *.a *.so *.s main
