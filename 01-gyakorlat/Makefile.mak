CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = cuboid

all: $(TARGET)

$(TARGET): cuboid.o
	$(CC) $(CFLAGS) -o $(TARGET) cuboid.o

cuboid.o: cuboid.c
	$(CC) $(CFLAGS) -c cuboid.c

clean:
	rm -f $(TARGET) *.o
