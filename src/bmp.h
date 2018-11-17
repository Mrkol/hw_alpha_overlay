#ifndef BMP_H
#define BMP_H

#include <pmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <inttypes.h>
#include <stdio.h>

struct _header
{
	uint8_t signature[2];
	uint32_t file_size;
	uint8_t reserved[4];
	uint32_t pixel_data_offset;
} __attribute__((packed));

typedef struct _header header_t; 

struct _dib_header
{
	uint32_t size;
	uint32_t image_width;
	uint32_t image_height;
	uint16_t planes;
	uint16_t bits_per_pixel;
	uint32_t compression;
	uint32_t image_size;
	uint32_t x_ppm;
	uint32_t y_ppm;
	uint32_t colors;
	uint32_t important_colors;
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t alpha_mask;
	uint32_t color_space_type;
	uint8_t color_space_endpoints[36];
	uint32_t red_gamma;
	uint32_t green_gamma;
	uint32_t blue_gamma;
	uint32_t intent;
	uint32_t icc_profile_data;
	uint32_t icc_profile_size;
	uint32_t reserved;
};

typedef struct _dib_header dib_header_t;

struct _bitmap
{
	header_t header;
	dib_header_t dib_header;
	uint32_t* pixel_data;
};

typedef struct _bitmap bitmap_t;

int read_bmp(FILE* fd, bitmap_t* bmp);

int write_bmp(FILE* fd, bitmap_t* bmp);

void free_bmp(bitmap_t* bmp);

int overlay_bmp(bitmap_t* first, bitmap_t* second);

// a hack from "Dirty Pixels" by Jim Blinn, perfect scaling by 0..255
// #define mask 0xFF00FF
// uint32_t i = a8 * (b32 & mask) + 0x800080;
// return (i + ((i >> 8) & mask)) >> 8 & mask;
// doing it twice

#define magic _mm_set_epi32(0x00800080, 0x00800080, 0x00800080, 0x00800080)
#define mask_even_8 _mm_set_epi32(0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff)

static inline __m128i scale_color(__m128i base, __m128i scale)
{
	__m128i i =
		_mm_add_epi32(_mm_mullo_epi32(scale,
			_mm_and_si128(base, mask_even_8)), magic);

	__m128i result1 =
		_mm_and_si128(
			_mm_srli_epi32(
				_mm_add_epi32(
					_mm_and_si128(_mm_srli_epi32(i, 8), mask_even_8),
					i),
				8),
			mask_even_8);
	
	__m128i j =
		_mm_add_epi32(_mm_mullo_epi32(scale,
			_mm_and_si128(_mm_srli_epi32(base, 8), mask_even_8)), magic);

	__m128i result2 =
		_mm_and_si128(
			_mm_srli_epi32(
				_mm_add_epi32(
					_mm_and_si128(_mm_srli_epi32(j, 8), mask_even_8),
					j),
				8),
			mask_even_8);

	return _mm_or_si128(result1, _mm_slli_epi32(result2, 8));
}

#define mask_alpha _mm_set_epi32(0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff)

static inline __m128i overlay_blend(__m128i front, __m128i back)
{
	__m128i alpha = _mm_and_si128(front, mask_alpha);
	__m128i one_minus_alpha = _mm_sub_epi8(mask_alpha, alpha);

	__m128i fst = scale_color(front, alpha);
	__m128i snd = scale_color(back, one_minus_alpha);

	return _mm_or_si128(_mm_add_epi8(fst, snd), mask_alpha);
}

static inline uint32_t get_pos(uint32_t mask)
{
	if (mask & 0xff000000) return 3;
	if (mask & 0x00ff0000) return 2;
	if (mask & 0x0000ff00) return 1;
	if (mask & 0x000000ff) return 0;
	return -1;
}

static inline uint32_t get_shuffle(dib_header_t* dib)
{
	return (get_pos(dib->red_mask) << 24)
		 | (get_pos(dib->green_mask) << 16)
		 | (get_pos(dib->blue_mask) << 8)
		 | (get_pos(dib->alpha_mask) << 0);
}




#endif // BMP_H
