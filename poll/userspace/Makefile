CFLAGS =-std=c11 -Wall -Wextra -Werror -Wstrict-prototypes \
			-Wredundant-decls -Wshadow -pedantic -pedantic-errors \
			-fno-strict-aliasing -D_XOPEN_SOURCE=700 -O2 
BIN_POLL = kpoll_poll_userspace
BIN_SELECT = kpoll_select_userspace

all: $(BIN_POLL) $(BIN_SELECT)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN_POLL): $(BIN_POLL).o
	$(CC) -o $(BIN_POLL) -O $(BIN_POLL).o

$(BIN_SELECT): $(BIN_SELECT).o
	$(CC) -o $(BIN_SELECT) -O $(BIN_SELECT).o

clean:
	rm -f $(BIN_POLL) $(BIN_SELECT) *.o

