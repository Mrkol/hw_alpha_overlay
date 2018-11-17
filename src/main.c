#include <stdio.h>
#include <string.h>
#include "bmp.h"


int main(int argc, char** argv)
{

	FILE* first_file = NULL;
	FILE* second_file = NULL;
	FILE* result_file = NULL;
	bitmap_t first_bmp;
	bitmap_t second_bmp;
	memset(&first_bmp, 0, sizeof(bitmap_t));
	memset(&second_bmp, 0, sizeof(bitmap_t));


	if (argc != 4)
		goto error;

	if ((first_file = fopen(argv[1], "r")) <= 0)
		goto error;
	if ((second_file = fopen(argv[2], "r")) <= 0)
		goto error;
	if ((result_file = fopen(argv[3], "w")) <= 0)
		goto error;

	read_bmp(first_file, &first_bmp);
	read_bmp(second_file, &second_bmp);

	if (overlay_bmp(&first_bmp, &second_bmp) != 0)
		goto error;

	write_bmp(result_file, &first_bmp);

	free_bmp(&first_bmp);
	free_bmp(&second_bmp);

	fclose(first_file);
	fclose(second_file);
	fclose(result_file);

	return 0;

error:
	if (first_bmp.pixel_data != NULL) free_bmp(&first_bmp);
	if (second_bmp.pixel_data != NULL) free_bmp(&second_bmp);

	if (first_file > 0) fclose(first_file);
	if (second_file > 0) fclose(second_file);
	if (result_file > 0) fclose(result_file);

	return 1;
}
