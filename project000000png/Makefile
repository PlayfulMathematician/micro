CC = g++
CFLAGS = -Wall -Wextra -O2

TARGET = png
SRC = png.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

draw: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all draw clean

