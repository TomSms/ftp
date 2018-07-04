CC = gcc
FLAG = -W
ftpclient:protocol.o main.c
	$(CC) $(FLAG) -o $@ $^
protocol.o: protocol.h protocol.c
	$(CC) -c $^

clean:
	rm *.gch *.o
