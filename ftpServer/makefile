CC=gcc
FLAG = -W

app: ftp_handler.h ftp_handler.c protocol.h protocol.c main.c
	$(CC) $(FLAG) -o $@ -pthread $^ 

clean:
	rm *.gch
