CC = gcc -Wall -std=gnu11 -O3
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)

test: clean all
	./$(FILE_O_NAME)

clean:
	rm -f *.o *.a *.so *.s $(FILE_O_NAME)$(TEST_FILE) $(TEST_FILE_SYM)
