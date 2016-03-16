typedef struct SeamPixels {
	double red;
	double green;
	double blue;
	double index;
} SeamPixels;

typedef struct SeamImage {
	uint64_t height;
	uint64_t width;
	SeamPixels *data;
} SeamImage;

typedef struct SeamPath {
	uint64_t pos;
} SeamPath;

/* Constructor */
SeamImage*
newSeamImage (uint64_t height, uint64_t width);

void
destroySeamImage (SeamImage *image);

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
SeamPath*
findImageVerticalSeamPath (SeamImage *image);

void
SeamRemove (SeamImage *image,
	   		size_t image_width,
		   	size_t image_height);


/* --- Implementations --- */
SeamImage*
newSeamImage (uint64_t height, uint64_t width)
{
	SeamImage   *image;

	image = (SeamImage *)malloc (sizeof (SeamImage));
	if (image == NULL) {
		printf("Error allocating image\n");
		return NULL;
	}
	image->data = (SeamPixels *)malloc (sizeof (SeamPixels) * width * height);
	if (image->data == NULL) {
		printf("Error allocating image\n");
		return NULL;
	}
	image->height = height;
	image->width = width;

	return image;
}

void
destroySeamImage (SeamImage *image) {
	if (image != NULL) {
		if (image->data != NULL) {
			free (image->data);
		}
		free (image);
	}
}

/*
 * Return the energy value of the pixels in the red attribute
 * of the energy image
 */
SeamImage*
getEnergyFromImage (SeamImage *image)
{
	uint64_t    x, y;
	SeamPixels  left, top, right, bottom;
	SeamImage *energy_img;

	energy_img = newSeamImage (image->height, image->width);

	for (y = 0; y < image->height*image->width; y += image->width) {
		for (x = 0; x < image->width; x++) {
			uint64_t pos = x + y;
			/* Boundary check */
			left = (x == 0)? image->data[pos]: image->data[pos-1];
			right = (x == image->width-1)? image->data[pos]: image->data[pos+1];
			top = (y == 0)? image->data[pos]: image->data[pos-image->width];
			bottom = (y == (image->height-1)*image->width)? image->data[pos]: image->data[pos+image->width];

			energy_img->data[pos].red = getPixelGradient (left, right, top, bottom);
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
	uint64_t  max;
	SeamImage *energy_img;
	SeamPath *path;

	while (image->width > image_width) {
		energy_img = getEnergyFromImage (image);
		path = findImageVerticalSeamPath (energy_img);
		destroySeamImage (energy_img);

		uint64_t pathpos = 0;
		x = 0;
		for (y = 0; y < image->height*image->width; y++) {
			if (path[pathpos].pos != y) {
				image->data[x] = image->data[y];
				x++;
			} else {
				pathpos++;
			}
		}
		image->width -= 1;
		free (path);
	}
	/*max = 0;
	for (y = 0; y < image->height*image->width; y++) {
		if (max < energy_img->data[y].green) {
			max = energy_img->data[y].green;
		}
	}
	printf("%ld\n",max);
	for (y = 0; y < image->height*image->width; y++) {
		energy_img->data[y].green = (energy_img->data[y].green*90000)/max;
	}*/

	/*for (y = 0; y < image->height*image->width; y+=image->width) {
		for (x = 0; x < image->width; x++) {
			uint64_t pos = x+y;
			//image->data[pos].red = energy_img->data[pos].red;
			//image->data[pos].green = energy_img->data[pos].red;
			//image->data[pos].blue = energy_img->data[pos].red;
			image->data[pos].red = energy_img->data[pos].green;
			image->data[pos].green = energy_img->data[pos].green;
			image->data[pos].blue = energy_img->data[pos].green;
		}
	}*/
	return;
}

SeamPath*
findImageVerticalSeamPath (SeamImage *image)
{
	/* RED: Image Energy */
	/* GREEN: Minimum path matrix -> Dynamic Programing */
	uint64_t   y, x;
	uint64_t   pos, posabove;
	SeamPath   *path;
	SeamPixels *data;
	
	data = image->data;
	path = (SeamPath*) malloc (sizeof(SeamPath)*(image->height));
	if (path == NULL) {
		printf ("Error: unable to allocate memory for minimum path");
		return NULL;
	}

	/* Calculate all minimum paths */	
	for (x = 0; x < image->width; x++) {
		data[x].green = data[x].red;
	}

	for (y = image->width; y < image->height*image->width; y+=image->width) {
		for (x = 0; x < image->width; x++) {
			pos = x + y;
			posabove = pos - image->width;
			if (x == 0) {
				data[pos].green = data[pos].red + fmin(data[posabove].green, data[posabove+1].green);
			} else if (x == (image->width - 1)) {
				data[pos].green = data[pos].red + fmin(data[posabove-1].green, data[posabove].green);
			} else {
				data[pos].green = data[pos].red + fmin(data[posabove-1].green, fmin(data[posabove].green, data[posabove+1].green));
			}
		}
	}

	/* Find minimum path */
	uint64_t minimum = (image->height-1)*image->width;
	for (pos = minimum; pos < image->height*image->width; pos++) {
		if (data[pos].green < data[minimum].green) {
			minimum = pos;
		}
	}

	int64_t path_pos = image->height-1; /* Must be signed integer! */
	path[path_pos].pos = minimum;
	pos = minimum;
	for (path_pos = image->height-2; path_pos >= 0; path_pos--) {
		posabove = pos - image->width;
		if (pos % image->width == 0) {
			path[path_pos].pos = (data[posabove].green < data[posabove+1].green)? posabove : posabove+1;
		} else if ((pos+1) % image->width == 0) {
			path[path_pos].pos = (data[posabove-1].green < data[posabove].green)? posabove-1 : posabove;
		} else {
			if (data[posabove-1].green < data[posabove+1].green) {
				path[path_pos].pos = (data[posabove].green < data[posabove-1].green)? posabove : posabove-1;
			} else {
				path[path_pos].pos = (data[posabove].green < data[posabove+1].green)? posabove : posabove+1;
			}
		}
		pos = path[path_pos].pos; //Update current position
	}

	return path;
}
