CC = gcc
# -Wall -O3
CFLAGS =  -lpthread -g
OBJECTS = main.o

default: $(OBJECTS)

clean:
	rm *.o

%.o: %.c
	$(CC) -o$@ $< $(CFLAGS)

.PHONY: clean
