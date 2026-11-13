CC = cc
CPPFLAGS += -Iinclude
CPPFLAGS += $(shell pkg-config --cflags notcurses-core)

CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -g
CFLAGS += -pthread

LDLIBS += $(shell pkg-config --libs notcurses-core)

SOURCES = src/main.c src/bank.c src/journal.c src/dashboard.c
HEADERS = include/bank.h include/journal.h include/dashboard.h

all: piggybank

piggybank: $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS) -o $@

clean:
	rm -f piggybank piggybank.journal

.PHONY: all clean
