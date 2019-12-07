CC = gcc -Wall -std=gnu11 -O3
CLIENT_C = client.c
CLIENT_O = client

SERVER_C = server.c
SERVER_O = server

LIB_NAME = properties

all: clean
	$(CC) $(SERVER_C) -o $(SERVER_O)
	$(CC) $(CLIENT_C) -o $(CLIENT_O)

#test: clean all
	#./$(SERVER_O) & 
#	./$(CLIENT_O) &

clean:
	rm -f *.o *.a *.so *.s $(SERVER_O) $(CLIENT_O)
