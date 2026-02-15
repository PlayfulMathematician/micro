CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -ggdb

TARGET = ls.out
SRC = ls.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all test clean

