CC=clang
CFLAGS = -g

all: tcpc

tcpc: client.c
	$(CC) $(CFLAGS) -o tcpc client.c

clean:
	rm -rf *.dSYM tcpc