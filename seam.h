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

/* Constructor */
SeamImage*
newSeamImage (uint64_t height, uint64_t width);

/* Pixel Functions */
double
getPixelGradient (SeamPixels left,
	              SeamPixels right,
				  SeamPixels top,
				  SeamPixels bottom);

/* Image Functions */
SeamImage*
getEnergyFromImage (SeamImage *image);

/* Seam Functions */
void
SeamRemove (SeamImage *image,
	   		size_t image_width,
		   	size_t image_height);


/* --- Implementations --- */
SeamImage*
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

SeamImage*
getEnergyFromImage (SeamImage *image)
{
	uint64_t    x, y;
	SeamPixels  left, top, right, bottom;
	SeamImage *energy_img;

	energy_img = newSeamImage (image->height, image->width);

	for (y = 0; y < image->height; y++) {
		for (x = 0; x < image->width; x++) {
			/* Boundary check */
			left = (x == 0)? image->data[y][x]: image->data[y][x-1];
			right = (x == image->width-1)? image->data[y][x]: image->data[y][x+1];
			top = (y == 0)? image->data[y][x]: image->data[y-1][x];
			bottom = (y == image->height-1)? image->data[y][x]: image->data[y+1][x];

			energy_img->data[y][x].red = getPixelGradient (left, right, top, bottom);
		}
	}
	return energy_img;
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
	uint64_t  y, x;
	SeamImage *energy_img;

	energy_img = getEnergyFromImage (image);
	for (y = 0; y < image->height; y++) {
		for (x = 0; x < image->width; x++) {
			image->data[y][x].red = energy_img->data[y][x].red;
			image->data[y][x].green = energy_img->data[y][x].red;
			image->data[y][x].blue = energy_img->data[y][x].red;
		}
	}	
	return;
}
