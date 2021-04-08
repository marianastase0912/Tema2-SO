CC = gcc
CFLAGS = -Wall -g -fPIC
 
build: libso_stdio.so

libso_stdio.so: so_stdio.o
	$(CC) -shared -o $@ $^

so_stdio.o: so_stdio.c so_stdio.h
	$(CC) $(CFLAGS) -c so_stdio.c so_stdio.h

.PHONY: clean
 
clean:
	rm -f *.o *~ libso_stdio.so so_stdio.h.gch 
