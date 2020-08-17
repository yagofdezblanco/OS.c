OBJS=listas.o listasProcesos.o shell.o

all: shell

shell: $(OBJS)
	gcc -g -o shell $(OBJS)

%.o: %.c
	gcc -g -c $<

clean:
	rm -f $(OBJS) shell *.o

tar:
	tar czvf p3.tar.gz *.c *.h Makefile
