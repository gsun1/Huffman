CFLAGS= -g -Wall

compresser: compresser.o decode.o tree.o table.o
	cc $(CFLAGS) -o compresser compresser.o decode.o tree.o table.o

compresser.o: compresser.c decode.h tree.h table.h
	cc $(CFLAGS) -c compresser.c


decode.o: decode.c decode.h tree.h table.h
	cc $(CFLAGS) -c decode.c

tree.o: tree.c tree.h table.h
	cc $(CFLAGS) -c tree.c

table.o: table.c table.h
	cc $(CFLAGS) -c table.c

