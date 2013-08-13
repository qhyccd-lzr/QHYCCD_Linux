CC=gcc
OBJ= canon350d.o

all: shutter

shutter: $(OBJ)
	$(CC) -o $@ $^

%.o:%.c
	$(CC) -c -O3 $^ -o $@

clean: 
	rm -f $(OBJ)

distclean: clean
	rm -f shutter

dep:
	$(CC) -M -I. *.c >dep

-include "dep"
