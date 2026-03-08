CC = g++
CFLAGS = -Wall -Wextra -std=c++20 -O0 -ggdb

TARGET = json.out
SRC = json.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

test: $(TARGET)
	./$(TARGET) test.json
clean:
	rm -f $(TARGET)

.PHONY: all test clean
