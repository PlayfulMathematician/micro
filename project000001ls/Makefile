CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

TARGET = ls
SRC = ls.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all test clean

