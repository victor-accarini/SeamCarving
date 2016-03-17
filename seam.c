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
		printf ("Usage:\n\t./seam [IMAGE] [NewWidth] [NewHeight]\n\n");
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
	uint64_t         new_width = strtol (argv[2], NULL, 10);
	uint64_t         new_height = strtol (argv[3], NULL, 10);
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
	uint64_t pos;
	for (y = 0; y < (long int) image_height*image_width; y+=image_width) {
		pixels = PixelGetNextIteratorRow (it, &width);
		if (pixels == NULL) {
			break;
		}
		for (x = 0; x < (long int) image_width; x++) {
			pos = x + y;
            PixelGetMagickColor (pixels[x], &pixel);
			image->data[pos].red = pixel.red;
			image->data[pos].green = pixel.green;
			image->data[pos].blue = pixel.blue;
			image->data[pos].index = pixel.index;
		}
	}
	
	if (y < (uint64_t) image_height) {
		return 1;//ThrowWandException (wand);
	}

	it = DestroyPixelIterator(it);
	DestroyMagickWand(wand);

	/* Here is were the magic works */
	SeamResize (image, new_width, new_height);

	/* Print result */
	PixelWand *kkk = NewPixelWand();
	PixelSetColor(kkk, "blue");
	status = MagickNewImage (out_wand, new_width, new_height, kkk);
	if (status == MagickFalse) {
        return 1;//ThrowWandException (out_wand);
	}

	out_it = NewPixelIterator (out_wand);
	if (out_it == NULL) {
        return 1;//ThrowWandException (out_wand);
	}

	for (y = 0; y < (long int) new_height*new_width; y+= new_width) {
		pixels = PixelGetNextIteratorRow (out_it, &width);
		if (pixels == NULL) {
			break;
		}
		for (x = 0; x < (long int) new_width; x++) {
			pos = x + y;
            PixelGetMagickColor (pixels[x], &pixel);
			pixel.red = image->data[pos].red;
			pixel.green = image->data[pos].green;
			pixel.blue = image->data[pos].blue;
			pixel.index = image->data[pos].index;
			PixelSetMagickColor (pixels[x], &pixel);
		}
		PixelSyncIterator(out_it);
	}

	if (y < (uint64_t) new_height) {
		return 1;//ThrowWandException (out_wand);
	}

	MagickWriteImage (out_wand, "out.jpg");

	out_it = DestroyPixelIterator(out_it);
	DestroyMagickWand(out_wand);

	MagickWandTerminus();

	return 0;
}
