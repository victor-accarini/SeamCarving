#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <glib.h>
#include <wand/MagickWand.h>

typedef struct SeamPixels {
	double red;
	double green;
	double blue;
	double index;
} SeamPixels;

typedef struct SeamImage {
	uint64_t height;
	uint64_t width;
	SeamPixels **data;
} SeamImage;

double getPixelGradient (SeamPixels left, SeamPixels right, SeamPixels top, SeamPixels bottom);

SeamImage *
newSeamImage (uint64_t height, uint64_t width)
{
	SeamImage   *image;
	uint64_t    row;

	image = (SeamImage *)malloc (sizeof (SeamImage));
	if (image == NULL) {
		printf("Error allocating image\n");
	}

	image->height = height;
	image->width = width;
	image->data = (SeamPixels **)malloc (sizeof (SeamPixels*) * height);
	if (image->data == NULL) {
		printf("Error allocating image\n");
	}
	for (row = 0; row < height; row++ ) {
		image->data[row] = (SeamPixels *) malloc (sizeof (SeamPixels) * width);
		if (image->data[row] == NULL) {
			printf("Error allocating image\n");
		}
	}

	return image;
}

void
getEnergyFromImage (SeamImage *image, SeamImage *energy_img)
{
	uint64_t    x, y;
	SeamPixels  left, top, right, bottom;

	if (image->height != energy_img->height || image->width != energy_img->width) {
		printf("ERROR: Energy image needs dimension's size equal the original image\n");
	}
	for (y = 0; y < image->height; y++) {
		for (x = 0; x < image->width; x++) {
			uint64_t center = x + y*image->width;
			
			left = (x == 0)? image->data[y][x]: image->data[y][x-1];
			right = (x == image->width-1)? image->data[y][x]: image->data[y][x+1];
			top = (y == 0)? image->data[y][x]: image->data[y-1][x];
			bottom = (y == image->height-1)? image->data[y][x]: image->data[y+1][x];

			energy_img->data[y][x].red = getPixelGradient (left, right, top, bottom);
		}
	}
	for (y = 0; y < image->height; y++) {
		for (x = 0; x < image->width; x++) {
			image->data[y][x].red = energy_img->data[y][x].red;
			image->data[y][x].green = energy_img->data[y][x].red;
			image->data[y][x].blue = energy_img->data[y][x].red;
		}
	}	
}

double
getPixelGradient (SeamPixels left, SeamPixels right, SeamPixels top, SeamPixels bottom) 
{
	double red, green, blue;
	double gradient;

	red = left.red - right.red;
	green = left.green - right.green;
	blue = left.blue - right.blue;
	
	gradient = sqrt(red*red + green*green + blue*blue);

	red = top.red - bottom.red;
	green = top.green - bottom.green;
	blue = top.blue - bottom.blue;
	
	gradient += sqrt(red*red + green*green + blue*blue);

	return gradient;
}

void
SeamRemove (SeamImage *image, size_t image_width, size_t image_height)
{
	uint64_t  height, width;
	double    brightness;
	SeamImage *energy_img;

	energy_img = newSeamImage (image->height, image->width);
	getEnergyFromImage (image, energy_img);
	return;
}

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
