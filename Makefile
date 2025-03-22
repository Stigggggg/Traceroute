TARGET = traceroute

CC = gcc

CFLAGS = -std=gnu99 -g -Wall -Wextra

SOURCES = main.c icmp_receive.c icmp_send.c utils.c 

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(TARGET)

.PHONY: all clean distclean