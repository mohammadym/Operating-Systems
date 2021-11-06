CC = gcc
# -Wall -O3
CFLAGS =  -Wall -O3 -g
OBJECTS = producer.o

default: $(OBJECTS)

clean:
	rm *.o

%.o: %.c
	$(CC) -o$@ $< $(CFLAGS)

.PHONY: clean
