CC = gcc -Wall -std=gnu11 -O3
FILE_C_NAME = main.c
FILE_O_NAME = main
FILE_S_NAME = main.s

all:
	$(CC) $(FILE_C_NAME) -o $(FILE_O_NAME)

test: clean all
	@echo "~~~"
	@echo "Test dla typu 1"
	@echo "~~~"
	./$(FILE_O_NAME) 2000 1
	@echo "~~~"
	@echo "Test dla typu 2"
	@echo "~~~"
	./$(FILE_O_NAME) 2000 2
	@echo "~~~"
	@echo "Test dla typu 3"
	@echo "~~~"
	./$(FILE_O_NAME) 2000 3

clean:
	rm -rf *.o *.a *.so *.s $(FILE_O_NAME)
