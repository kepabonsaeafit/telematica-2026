CC=gcc
CFLAGS=-Wall -pthread

all: server

server: server.c
	$(CC) $(CFLAGS) server.c -o server

clean:
	rm -f server *.o mis_logs.txt

run: server
	./server 8080 mis_logs.txt

dns:
	@echo "127.0.0.1 telematica.local" | sudo tee -a /etc/hosts
	@echo "[*] DNS local configurado: telematica.local -> 127.0.0.1"
