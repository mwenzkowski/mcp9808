CC = gcc
CFLAGS = -Wall -Wextra -pedantic -O2

all: mcp9808_info gaerbox

gaerbox: gaerbox.o mcp9808.o
	$(CC) $(CFLAGS) -o gärbox gaerbox.o mcp9808.o

mcp9808_info: mcp9808_info.o mcp9808.o
	$(CC) $(CFLAGS) -o $@ mcp9808_info.o mcp9808.o

# Eine Regel file.o: <Abhängigkeiten> hängt automatisch von file.c ab
gaerbox.o: mcp9808.h
mcp9808_info.o: mcp9808.h
mcp9808.o: mcp9808.h

.PHONY:
clean:
	rm *.o mcp9808_info gärbox
