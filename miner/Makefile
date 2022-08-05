all: miner

HDR=event_q.h mempool.h nonce.h reader.h siggen.h transactions.h

SRC=main.c event_q.c mempool.c nonce.c reader.c siggen.c transactions.c

miner: $(SRC) $(HDR)
	gcc -Wall -o miner $(SRC) -l pthread

clean:
	rm -f *.o miner

