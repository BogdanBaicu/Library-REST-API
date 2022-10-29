CC = g++
SOURCES = helpers.cpp buffer.cpp requests.cpp  client.cpp 

build:
	$(CC) $(SOURCES) -o client -Wall

run:
	./client

clean:
	rm -f *.o client