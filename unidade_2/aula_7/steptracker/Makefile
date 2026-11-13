CC = cc
CPPFLAGS += -Iinclude
CPPFLAGS += $(shell pkg-config --cflags notcurses-core)

CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -g

LDLIBS += -lm
LDLIBS += $(shell pkg-config --libs notcurses-core)

SOURCES = src/main.c src/tracker.c src/terminal.c
HEADERS = include/tracker.h include/terminal.h

all: steptracker

steptracker: $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS) -o $@ $(LDLIBS)

clean:
	rm -f steptracker

.PHONY: all clean
