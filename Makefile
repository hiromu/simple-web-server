CC = gcc

CFLAGS = -Wall
LDFLAGS = 

TARGET = server
OBJS = main.o server.o util.o

all: $(TARGET)

server: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(TARGET) $(OBJS)
