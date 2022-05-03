CC=clang
CFLAGS = -g

all: tcpc serve

tcpc: client.c
	$(CC) $(CFLAGS) -o tcpc -pthread client.c 

serve: server.c
	$(CC) $(CFLAGS) -o serve -pthread server.c

clean:
	rm -rf *.dSYM tcpc serve