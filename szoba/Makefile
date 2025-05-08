CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -I C:/Users/flask/Desktop/szgarf/grafika/c_sdk/MinGW/include
LDFLAGS = -L C:/Users/flask/Desktop/szgarf/grafika/c_sdk/MinGW/lib -lmingw32 -lSDL2main -lSDL2_image -lSDL2 -lopengl32 -lglu32 -lfreeglut
TARGET = szoba

SRCS = szoba.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET).exe $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q $(TARGET).exe *.o

