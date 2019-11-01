CC = gcc -Wall -std=gnu11 -O3
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)
	mkdir A_Folder A_Folder/AA_Folder B_Folder C_Folder
	touch A_Folder/test_A B_Folder/test_B C_Folder/test_C
	touch A_Folder/AA_Folder/test_AA

test: clean all
	./$(FILE_O_NAME) ./ '<' "18.10.2019 18:21:01" dir

clean:
	rm -f *.o *.a *.so *.s $(FILE_O_NAME)
	rm -rf A_Folder B_Folder C_Folder
