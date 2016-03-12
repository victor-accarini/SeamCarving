#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <glib.h>
#include "seam.h"
#include <wand/MagickWand.h>

int
main (int argc, char *argv[])
{

	if (argc != 4) {
		printf ("Usage:\n\t./seam [IMAGE] [NewSizeX] [NewSizeY]\n\n");
		return 1;
	}

	MagickWand        *wand;
	PixelIterator     *it;

	MagickWand        *out_wand;
	PixelIterator     *out_it;
	
	MagickPixelPacket pixel;
	PixelWand         **pixels;
	MagickBooleanType status;
	unsigned long     width;

	size_t           image_width;
	size_t           image_height;
	uint64_t         new_lines = strtol (argv[2], NULL, 10);
	uint64_t         new_columns = strtol (argv[3], NULL, 10);
	SeamImage        *image;

	MagickWandGenesis ();
	wand = NewMagickWand ();
	out_wand = NewMagickWand ();

	status = MagickReadImage (wand, argv[1]);
	if (status == MagickFalse) {
        return 1;//ThrowWandException (wand);
	}

	it = NewPixelIterator (wand);
	if (it == NULL) {
        return 1;//ThrowWandException (wand);
	}

	image_height = MagickGetImageHeight (wand);
	image_width = MagickGetImageWidth (wand);

	image = newSeamImage (image_height, image_width);

	long int y, x;
	for (y = 0; y < (long int) image_height; y++) {
		pixels = PixelGetNextIteratorRow (it, &width);
		if (pixels == NULL) {
			break;
		}
		for (x = 0; x < (long int) image_width; x++) {
            PixelGetMagickColor (pixels[x], &pixel);
			image->data[y][x].red = pixel.red;
			image->data[y][x].green = pixel.green;
			image->data[y][x].blue = pixel.blue;
			image->data[y][x].index = pixel.index;
		}
	}
	
	if (y < (uint64_t) image_height) {
		return 1;//ThrowWandException (wand);
	}

	it = DestroyPixelIterator(it);
	DestroyMagickWand(wand);

	/* Here is were the magic works */
	SeamRemove (image, new_lines, new_columns);

	/* Print result */
	PixelWand *kkk = NewPixelWand();
	PixelSetColor(kkk, "blue");
	status = MagickNewImage (out_wand, image_width, image_height, kkk);
	if (status == MagickFalse) {
        return 1;//ThrowWandException (out_wand);
	}

	out_it = NewPixelIterator (out_wand);
	if (out_it == NULL) {
        return 1;//ThrowWandException (out_wand);
	}

	for (y = 0; y < (long int) image_height; y++) {
		pixels = PixelGetNextIteratorRow (out_it, &width);
		if (pixels == NULL) {
			break;
		}
		for (x = 0; x < (long int) image_width; x++) {
            PixelGetMagickColor (pixels[x], &pixel);
			//printf ("%lf %lf %lf\n", pixel.red, pixel.green, pixel.blue);
			//printf ("%.0lf %.0lf %.0lf - ", image[y][x].red, image[y][x].green, image[y][x].blue);
			pixel.red = image->data[y][x].red;
			pixel.green = image->data[y][x].green;
			pixel.blue = image->data[y][x].blue;
			pixel.index = image->data[y][x].index;
			PixelSetMagickColor (pixels[x], &pixel);
		}
		PixelSyncIterator(out_it);
	}

	if (y < (uint64_t) image_height) {
		return 1;//ThrowWandException (out_wand);
	}

	MagickWriteImage (out_wand, "out.jpg");

	out_it = DestroyPixelIterator(out_it);
	DestroyMagickWand(out_wand);

	MagickWandTerminus();

	return 0;
}
