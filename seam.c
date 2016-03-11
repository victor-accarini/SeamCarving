#include <stdio.h>
#include <glib.h>

int
main (int argc, char *argv[])
{

	if (argc != 4) {
		printf("Usage:\n\t./seam [IMAGE] [NewSizeX] [NewSizeY]\n\n");
		return 1;
	}

	return 0;
}
