CFLAGS=-g -Wall
CFAST=-O3 -Wall
PKGFLAGS=`pkg-config --cflags --libs MagickWand` `pkg-config --cflags --libs glib-2.0`
LFLAGS=-lm

all: seam_dev

seam_dev: seam.c seam.h
	gcc $(CFLAGS) seam.c $(PKGFLAGS) -o seam_dev $(LFLAGS) 

test: test.c seam.h
	gcc $(CFLAGS) test.c $(PKGFLAGS) -o seam $(LFLAGS) 

seam: seam.c seam.h
	gcc $(CFAST) seam.c $(PKGFLAGS) -o seam $(LFLAGS) 
