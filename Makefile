all: seam

seam: seam.c
	gcc -g seam.c `pkg-config --cflags --libs glib-2.0` -o seam 
