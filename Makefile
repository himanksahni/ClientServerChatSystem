PORT=56730
DEPENDENCIES = hash.h ftree.h
FLAGS = -DPORT=$(PORT) -Wall -std=gnu99

all: rcopy_server rcopy_client

rcopy_server: rcopy_server.o ftree.o hash_functions.o
	gcc ${FLAGS} -o $@ $^

rcopy_client: rcopy_client.o ftree.o hash_functions.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c ${DEPENDENCIES}
	gcc ${FLAGS} -c $<

clean:
	rm *.o rcopy_server rcopy_client

