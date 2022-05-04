CC=clang
CFLAGS = -g

all: tcpc serve

tcpc: client.c
	$(CC) $(CFLAGS) -o tcpc -pthread -l json-c client.c 

serve: server.c
	$(CC) $(CFLAGS) -o serve -pthread -l json-c server.c

clean:
	rm -rf *.dSYM tcpc serve