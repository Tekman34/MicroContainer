CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu99
LDFLAGS = 

TARGET = microjail
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
