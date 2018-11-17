#include "bmp.h"
#include <stdlib.h>



int read_bmp(FILE* fd, bitmap_t* bmp)
{
	size_t read = 0;


	read = fread(&bmp->header, sizeof(header_t), 1, fd);
	if (read < 0)
		goto error;

	read = fread(&bmp->dib_header, sizeof(uint32_t), 1, fd);
	if (read < 0)
		goto error;

	uint32_t real_header_size = bmp->dib_header.size > sizeof(dib_header_t)
		? sizeof(dib_header_t) : bmp->dib_header.size;

	read = fread(((char*) &bmp->dib_header) + 4, real_header_size - 4, 1, fd);

	if (read < 0)
		goto error;

	uint32_t actualAlloc = bmp->dib_header.image_size;
	actualAlloc = (1 + ((actualAlloc - 1) >> 4)) << 4;

	bmp->pixel_data = aligned_alloc(16, actualAlloc);

	if (fseek(fd, bmp->header.pixel_data_offset, SEEK_SET) < 0)
		goto error;

	read = fread(bmp->pixel_data, bmp->dib_header.image_size, 1, fd);
	if (read < 0)
		goto error;

	return 0;

error:
	return 1;
}

int write_bmp(FILE* fd, bitmap_t* bmp)
{
	bmp->header.pixel_data_offset = sizeof(header_t) + bmp->dib_header.size;

	size_t retcode = 0;

	retcode = fwrite(&bmp->header, sizeof(header_t), 1, fd);
	if (retcode < 0)
		goto error;
	
	retcode = fwrite(&bmp->dib_header, bmp->dib_header.size, 1, fd);
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
		|| first->dib_header.image_height != second->dib_header.image_height
		|| first->dib_header.image_size != second->dib_header.image_size)
	{
		return 1;
	}

	uint8_t* fst = (uint8_t*) first->pixel_data;
	uint8_t* snd = (uint8_t*) second->pixel_data;

	uint32_t fst_fmt = get_shuffle(&first->dib_header);
	uint32_t snd_fmt = get_shuffle(&second->dib_header);

	__m128i fst_shuffler = 
		_mm_set_epi32(fst_fmt + 0x0c0c0c0c, fst_fmt + 0x08080808, fst_fmt + 0x04040404, fst_fmt);

	__m128i snd_shuffler = 
		_mm_set_epi32(snd_fmt + 0x0c0c0c0c, snd_fmt + 0x08080808, snd_fmt + 0x04040404, snd_fmt);

	for (uint32_t i = 0; i < first->dib_header.image_size; i += 16, fst += 16, snd += 16)
	{
		__m128i first_pixel = _mm_shuffle_epi8(_mm_load_si128((__m128i*) fst), fst_shuffler);
		__m128i second_pixel = _mm_shuffle_epi8(_mm_load_si128((__m128i*) snd), snd_shuffler);

		__m128i result = overlay_blend(second_pixel, first_pixel);

		_mm_store_si128((__m128i*) fst, result);
	}

	first->dib_header.red_mask =   0xff000000;
	first->dib_header.green_mask = 0x00ff0000;
	first->dib_header.blue_mask =  0x0000ff00;
	first->dib_header.alpha_mask = 0x000000ff;

	return 0;
}
