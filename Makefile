CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = saku-shell

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -r $(TARGET)
