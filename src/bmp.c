#include "bmp.h"
#include <stdlib.h>



int read_bmp(FILE* fd, bitmap_t* bmp)
{
	size_t read = fread(&bmp->header, sizeof(header_t), 1, fd);
	if (read < 0)
		goto error;

	read = fread(&bmp->dib_header, sizeof(uint32_t), 1, fd);
	if (read < 0)
		goto error;

	read = fread(((char*) &bmp->dib_header) + sizeof(uint32_t),
				bmp->dib_header.size - sizeof(uint32_t), 1, fd);
	if (read < 0)
		goto error;

	uint32_t actualAlloc = bmp->dib_header.image_size;
	actualAlloc = (actualAlloc / 16 + !!(actualAlloc % 16)) * 16;

	//padding
	bmp->pixel_data = aligned_alloc(16, actualAlloc);

	read = fread(bmp->pixel_data, bmp->dib_header.image_size, 1, fd);

	return 0;

error:
	return 1;
}

int write_bmp(FILE* fd, bitmap_t* bmp)
{
	size_t retcode = 0;
	retcode = fseek(fd, 0, SEEK_SET);
	if (retcode < 0)
		goto error;

	retcode = fwrite(&bmp->header, sizeof(header_t), 1, fd);
	if (retcode < 0)
		goto error;
	
	retcode = fwrite(&bmp->dib_header, sizeof(dib_header_t), 1, fd);
	if (retcode < 0)
		goto error;

	retcode = fwrite(bmp->pixel_data, bmp->dib_header.image_size, 1, fd);
	if (retcode < 0)
			goto error;

	return 0;

error:
	return 1;
}

void free_bmp(bitmap_t* bmp)
{
	free(bmp->pixel_data);
}

int overlay_bmp(bitmap_t* first, bitmap_t* second)
{
	if (first->dib_header.image_width != second->dib_header.image_width
		|| first->dib_header.image_height != second->dib_header.image_height)
	{
		return 1;
	}

	uint32_t count = first->dib_header.image_width
		* first->dib_header.image_height;

	for (uint32_t i = 0; i < count; i += 4)
	{
		__m128i first_pixel = _mm_load_si128((__m128i*)(first->pixel_data + i));
		__m128i second_pixel = _mm_load_si128((__m128i*)(second->pixel_data + i));

		__m128i result = overlay_blend(second_pixel, first_pixel);

		_mm_store_si128((__m128i*)(first->pixel_data + i), result);
	}

	return 0;
}
