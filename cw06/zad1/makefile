CC = gcc -Wall -std=gnu11 -O3
BARBER_C_NAME = barber.c
BARBER_O_NAME = barber

CLIENT_C_NAME = clientHandler.c
CLIENT_O_NAME = clientHandler

all:
	$(CC) $(BARBER_C_NAME) -o $(BARBER_O_NAME)
	$(CC) $(CLIENT_C_NAME) -o $(CLIENT_O_NAME)

test_barber: clean all
	./$(BARBER_O_NAME)

test_client:
	./$(CLIENT_O_NAME)

clean:
	rm -f *.o *.a *.so *.s $(BARBER_O_NAME) $(CLIENT_O_NAME)
