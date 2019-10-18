CC = gcc -Wall -std=c11 -O3
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s
SIZE = 4 512 4096 8192
COUNT = 100 1000

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)

test: clean all
	for size in $(SIZE) ; do \
		for count in 1000 100000 ; do \
			echo $$count $$size ; \
			./$(FILE_O_NAME) generate dane $$count $$size ; \
			./$(FILE_O_NAME) copy_sys dane daneCopyOne $$count $$size ; \
			./$(FILE_O_NAME) copy_lib dane daneCopyTwo $$count $$size ;\
		done ; \
	done ; \
	for size in $(SIZE) ; do \
		for count in $(COUNT) ; do \
			echo $$count $$size ; \
			./$(FILE_O_NAME) generate dane $$count $$size ; \
			./$(FILE_O_NAME) sort_sys dane $$count $$size ; \
			./$(FILE_O_NAME) sort_lib daneCopy $$count $$size ;\
		done ; \
	done ; \

test_copy: clean all
	for size in $(SIZE) ; do \
		for count in 1000 100000 ; do \
			echo $$count $$size ; \
			./$(FILE_O_NAME) generate dane $$count $$size ; \
			./$(FILE_O_NAME) copy_sys dane daneCopyOne $$count $$size ; \
			./$(FILE_O_NAME) copy_lib dane daneCopyTwo $$count $$size ;\
		done ; \
	done ; \

test_sort: clean all
	for size in $(SIZE) ; do \
		for count in $(COUNT) ; do \
			echo $$count $$size ; \
			./$(FILE_O_NAME) generate dane $$count $$size ; \
			./$(FILE_O_NAME) sort_sys dane $$count $$size ; \
			./$(FILE_O_NAME) sort_lib daneCopy $$count $$size ;\
		done ; \
	done ; \

clean:
	rm -f *.o *.a *.so *.s $(FILE_O_NAME) sort.txt copy.txt dane.txt daneCopy.txt daneCopyOne.txt daneCopyTwo.txt
