CC = g++

server: server.cpp hash_table.o
	$(CC) -o server server.cpp hash_table.o

hash_table.o: hash_table.cpp hash_table.h
	$(CC) -c hash_table.cpp

clean:
	rm -f server hash_table.o
