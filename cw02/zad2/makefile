CC = gcc -Wall -std=gnu11 -O3
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s
TEST_FILE = test
TEST_FILE_SYM = test2

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)
	touch $(TEST_FILE)
	ln -s $(TEST_FILE) $(TEST_FILE_SYM)	

test_dir: clean all
	./$(FILE_O_NAME) ./ '>' "18.10.2019 00:00:00" dir
	./$(FILE_O_NAME) ./ '=' "18.10.2019 00:00:00" dir
	./$(FILE_O_NAME) ./ '<' "18.10.2019 00:00:00" dir
	./$(FILE_O_NAME) ~/Documents/ '<' "18.10.2019 12:25:00" dir

test_nftw: clean all
	./$(FILE_O_NAME) ./ '>' "18.10.2019 00:00:00" nftw
	./$(FILE_O_NAME) ./ '=' "18.10.2019 00:00:00" nftw
	./$(FILE_O_NAME) ./ '<' "18.10.2019 00:00:00" nftw
	./$(FILE_O_NAME) ~/Documents/ '<' "18.10.2019 15:35:19" nftw

clean:
	rm -f *.o *.a *.so *.s $(FILE_O_NAME) $(TEST_FILE) $(TEST_FILE_SYM)
