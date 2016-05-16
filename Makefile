all: getdist getdist2

clean:
	rm -f motorcontrol *.o

getdist: getdist.o
	$(CC) $(LDFLAGS) -o $@ $^

getdist2: getdist2.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


