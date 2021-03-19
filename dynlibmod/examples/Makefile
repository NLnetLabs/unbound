CFLAGS=-I../.. -c -Wall -Werror -fpic -g -Wno-unused-variable

all: resgen.so helloworld.so

resgen.so: resgen.o
	$(CC) -shared -Wall -Werror -g -o resgen.so resgen.o

helloworld.so: helloworld.o
	$(CC) -shared -Wall -Werror -g -o helloworld.so helloworld.o

clean:
	-rm -rf *.o *.so

.PHONY : all clean


