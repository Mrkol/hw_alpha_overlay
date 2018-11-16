#include <stdio.h>
#include <string.h>
#include "bmp.h"


int main(int argc, char** argv)
{
	if (argc != 4)
		goto error;

	FILE* first_file;
	if ((first_file = fopen(argv[1], "r")) <= 0)
		goto error;
	FILE* second_file;
	if ((second_file = fopen(argv[2], "r")) <= 0)
		goto error;
	FILE* result_file;
	if ((result_file = fopen(argv[3], "w")) <= 0)
		goto error;

	bitmap_t first_bmp;
	memset(&first_bmp, sizeof(first_bmp), 0);
	read_bmp(first_file, &first_bmp);

	bitmap_t second_bmp;
	memset(&second_bmp, sizeof(second_bmp), 0);
	read_bmp(second_file, &second_bmp);

	if (overlay_bmp(&first_bmp, &second_bmp) != 0)
		goto error;

	write_bmp(result_file, &first_bmp);

	free_bmp(&first_bmp);
	free_bmp(&second_bmp);

	fclose(first_file);
	fclose(second_file);

	return 0;

error:
	return 1;
}
