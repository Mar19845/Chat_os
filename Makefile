CC=clang
CFLAGS = -g

all: tcpc serve

tcpc: client.c
	$(CC) $(CFLAGS) -o tcpc client.c 

serve: server.c
	$(CC) $(CFLAGS) -o serve server.c

clean:
	rm -rf *.dSYM tcpc serve