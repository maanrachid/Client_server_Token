all: bad client server
client:
	gcc -Wall -o client client.c checking.c
server:
	gcc -Wall -o server server.c checking.c
bad:
	gcc  -Wall -o bad badclient.c checking.c

clean:
	rm -fr client server bad *.o
