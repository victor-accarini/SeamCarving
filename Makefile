CFLAGS=-g -Wall
PKGFLAGS=`pkg-config --cflags --libs MagickWand` `pkg-config --cflags --libs glib-2.0`
LFLAGS=-lm

all: seam

seam: seam.c seam.h
	gcc $(CFLAGS) seam.c $(PKGFLAGS) -o seam $(LFLAGS) 

test: test.c seam.h
	gcc $(CFLAGS) test.c $(PKGFLAGS) -o seam $(LFLAGS) 
