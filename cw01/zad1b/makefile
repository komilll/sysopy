CC = gcc -Wall -ansi
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)

asm_O0:
	$(CC) -O0 -S -o $(FILE_S_NAME) $(FILE_C_NAME)

asm_O1:
	$(CC) -O1 -S -o $(FILE_S_NAME) $(FILE_C_NAME)

asm_O2:
	$(CC) -O2 -S -o $(FILE_S_NAME) $(FILE_C_NAME)

asm_O3:
	$(CC) -O3 -S -o $(FILE_S_NAME) $(FILE_C_NAME)

O0:
	$(CC) -O0 $(FILE_C_NAME) -o $(FILE_O_NAME)

O1:
	$(CC) -O1 $(FILE_C_NAME) -o $(FILE_O_NAME)

O2:
	$(CC) -O2 $(FILE_C_NAME) -o $(FILE_O_NAME)

O3:
	$(CC) -O3 $(FILE_C_NAME) -o $(FILE_O_NAME)

clean:
	rm -f *.o *.a *.so *.s $(FILE_O_NAME) lorem_ipsum.txt
