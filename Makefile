CC      := gcc
CFLAGS  := -Wall -Wextra -O2
LDFLAGS := -lrt

.PHONY: all clean

all: clean test

clean:
	rm -f test *.o

%.o: %.c
	$(CC) $(CFLAGS) -c $^

test: test.o barcode.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o test
