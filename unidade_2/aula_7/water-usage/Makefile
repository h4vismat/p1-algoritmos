CC = cc
CPPFLAGS += -Iinclude

CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -g

LDLIBS += -lm

SOURCES = src/main.c src/consumption.c src/log.c src/monitor.c src/resident.c src/replay.c src/sensor.c src/simulator.c src/stats.c
HEADERS = include/consumption.h include/log.h include/monitor.h include/resident.h include/replay.h include/sensor.h include/simulator.h include/stats.h

all: water_usage

water_usage: $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(SOURCES) $(LDLIBS) -o $@ $(LDLIBS)

clean:
	rm -f water_usage

.PHONY: all clean
