CFLAGS =-std=c11 -Wall -Wextra -Werror -Wstrict-prototypes \
			-Wredundant-decls -Wshadow -pedantic -pedantic-errors \
			-fno-strict-aliasing -D_XOPEN_SOURCE=700 -O2 
BIN = mmap_userspace

all: $(BIN)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN): $(BIN).o
	$(CC) -o $(BIN) -O $(BIN).o

clean:
	rm -f $(BIN) *.o

