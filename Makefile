OBJS=map.o vector.o strutils.o linenoise.o
OPTS=-ggdb
LDFLAGS=-lcurl

websh: websh.h websh.c $(OBJS)
	gcc $(OPTS) -o websh websh.c $(OBJS) $(LDFLAGS)

vector.o: vector.h vector.c
	gcc $(OPTS) -c vector.c

strutils.o: strutils.h strutils.c
	gcc $(OPTS) -c strutils.c

map.o: map.h map.c
	gcc $(OPTS) -c map.c

linenoise.o: linenoise.h linenoise.c
	gcc $(OPTS) -c linenoise.c

clean:
	rm -f *test *.o *.a
