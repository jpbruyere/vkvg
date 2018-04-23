inclpath = .

CC=gcc

CFLAGS= -UDEBUG -DSTANDALONE -UCLOCK\
	-I$(inclpath) -L/lib/pa1.1 -g 


# DEBUG: turn on debugging output
#
# STANDALONE: run as a separate program. read data from file.
#	      If this flag is False, then use the interface procedure 
#	      triangulate_polygon() instead.


LDFLAGS= -lm

objects= construct.o misc.o monotone.o tri.o
executable = triangulate

$(executable): $(objects)
	rm -f $(executable)
	$(CC) $(CFLAGS) $(objects) $(LDFLAGS) -o $(executable)

$(objects): $(inclpath)/triangulate.h

clean:
	rm -f $(objects)

