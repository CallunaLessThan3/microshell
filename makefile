CC     = gcc
CFLAGS =
DEPS   = argparse.h builtin.h
OBJ    = myshell.o argparse.o builtin.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

myshell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o main
