#ifndef PIXELPNG_H
# define PIXELPNG_H

// https://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html

# include <stddef.h>
# include <stdio.h>

# define IHDR_CHUNK_LENGTH 13



enum color_type {
	GRAYSCALE = 0,
	RGB = 2,
	PALETTE = 3,
	GRAYSCALE_ALPHA = 4,
	RGBA = 6
};

enum chunk_type {
	IHDR,
	IDAT,
	IEND
};

// TODO perhaps worth optimizing since maximum size is 4 bytes, and we always have 6
// I suppose grayscale is identical to r==g==b so that can be removed, and the palette can be "overriden" by alpha
typedef struct {
	unsigned char grayscale;	// 0-255
	unsigned char red;		// 0-255
	unsigned char green;		// 0-255
	unsigned char blue;		// 0-255
	unsigned char alpha;		// 0-255
	unsigned char palette_index;	// 0-255
} pixel;

typedef struct {
	int width;				// pixels
	int height;				// pixels
	unsigned char bit_depth;		// Valid values are 1, 2, 4, 8, and 16
	unsigned char color_type;		// Valid values are 0, 2, 3, 4, and 6
	unsigned char compression_method;	// Only 0 is valid for now
	unsigned char filter_method;		// Only 0 is valid for now
	unsigned char interlace_method;		// 0 (no interlace) or 1 (Adam7 interlace)
} ChunkIHDR;

typedef struct {
	size_t length;
	unsigned char* data;
} ChunkIDAT;

typedef struct {
	ChunkIHDR* ihdr_chunk;
	pixel** pixels;
} pixelPNG;

// PNG
pixelPNG* initialize_png(int width, int height, unsigned char bit_depth, int color_type, unsigned char compression_method, unsigned char filter_method, unsigned char interlace_method);
void generate_png(pixelPNG* pixelPNG, char* file_name);
void free_png(pixelPNG* pixelPNG);

// Pixels
pixel get_pixel(pixelPNG* pixelPNG, int x, int y);
void set_pixel(pixelPNG* pixelPNG, int x, int y, unsigned char grayscale, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, unsigned char palette_index);
void set_pixel_grayscale(pixelPNG* pixelPNG, int x, int y, short int grayscale);
void set_pixel_grayscale_alpha(pixelPNG* pixelPNG, int x, int y, short int grayscale, short int alpha);
void set_pixel_rgb(pixelPNG* pixelPNG, int x, int y, short int red, short int green, short int blue);
void set_pixel_rgba(pixelPNG* pixelPNG, int x, int y, short int red, short int green, short int blue, short int alpha);


// Generate Chunk Data
ChunkIHDR* generate_ihdr_chunk(int width, int height, unsigned char bit_depth, int color_type, unsigned char compression_method, unsigned char filter_method, unsigned char interlace_method);
ChunkIDAT* generate_idat_chunk(pixel** pixels, ChunkIHDR* ihdr_chunk);

// Write Chunks
void write_chunk(FILE* file, size_t length, int chunk_type, unsigned char* chunk_data);
void write_ihdr_chunk(FILE* file, ChunkIHDR* ihdr_chunk);
void write_idat_chunk(FILE* file, ChunkIDAT* idat_chunk);

// CRC
// https://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html
void make_crc_table(void);
unsigned long update_crc(unsigned long crc, unsigned char *buf, int len);
unsigned long crc(unsigned char *buf, int len);

#endif
