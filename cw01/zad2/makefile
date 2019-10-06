CC = gcc -Wall -ansi
SOURCE_NAME = main
SOURCE_NAME_DYNAMIC = mainDynamic
LIB_NAME = saving

use_static: static
	$(CC) -o $(SOURCE_NAME) $(SOURCE_NAME).o $(LIB_NAME).a -I .
	./$(SOURCE_NAME)

use_shared: shared
	$(CC) -g main.c -L./ -lsaving -o main
	LD_LIBRARY_PATH=./ ./$(SOURCE_NAME)

use_dynamic: dynamic
	$(CC) -rdynamic -o $(SOURCE_NAME_DYNAMIC) $(SOURCE_NAME_DYNAMIC).c -ldl
	./$(SOURCE_NAME_DYNAMIC)

static:
	$(CC) -c $(LIB_NAME).c
	ar rcs $(LIB_NAME).a $(LIB_NAME).o
	$(CC) -c $(SOURCE_NAME).c
	$(CC) $(SOURCE_NAME).o $(LIB_NAME).a -o $(SOURCE_NAME)

shared:
	$(CC) -c -fPIC $(LIB_NAME).c
	$(CC) -shared -fPIC $(LIB_NAME).c -o lib$(LIB_NAME).so

dynamic:
	make shared

clean:
	rm -f *.o *.a *.so *.s $(SOURCE_NAME) $(SOURCE_NAME_DYNAMIC) lorem_ipsum.txt
