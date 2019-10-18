CC = gcc -Wall -std=c11 -O3
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)

test_sort: clean all
	#./$(FILE_O_NAME) generate dane 5000 4
	#./$(FILE_O_NAME) sort_sys dane 5000 4
	#./$(FILE_O_NAME) sort_lib daneCopy 5000 4
	./$(FILE_O_NAME) generate dane 5000 512
	./$(FILE_O_NAME) sort_sys dane 5000 512
	./$(FILE_O_NAME) sort_lib daneCopy 5000 512

sort_lib: clean all
	./$(FILE_O_NAME) generate dane 100 512
	./$(FILE_O_NAME) sort_lib dane 100 512

sort_sys: clean all
	./$(FILE_O_NAME) generate dane 100 512
	./$(FILE_O_NAME) sort_sys dane 100 512

clean:
	rm -f *.o *.a *.so *.s $(FILE_O_NAME) *.txt
