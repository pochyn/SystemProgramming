# This illustrates how you can set the value of a defined constant 
# on the compile line.
PORT=54748
CFLAGS = -DPORT=$(PORT) -g -Wall -std=gnu99

all: rcopy_server rcopy_client

# The variable $@ has the value of the target. 
rcopy_server: rcopy_server.o hash_functions.o
	gcc ${CFLAGS} -o $@ rcopy_server.o hash_functions.o

rcopy_client: rcopy_client.o hash_functions.o
	gcc ${CFLAGS} -o $@ rcopy_client.o hash_functions.o

.c.o:
	gcc  ${CFLAGS}  -c $<

clean:
	rm *.o 

realclean:
	rm rcopy_server rcopy_client 

