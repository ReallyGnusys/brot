
all: test

clean:
	rm *.o
	rm test

test: test.o brot.o
	gcc -Xassembler -mavxscalar=256 test.o brot.o  `pkg-config --cflags --libs gtk+-2.0` -o test
	objdump -d test > test.lst

test.o: test.c brot.h
	gcc -c `pkg-config --cflags --libs gtk+-2.0` test.c

brot.o: brot.c brot.h
	gcc -mavx -c  brot.c

