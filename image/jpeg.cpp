#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <jpeglib.h>
#include "../jpeg.h"

void read_jpeg_header(const char *filename, int& width, int& height)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *infile;

	/* create jpeg decompress object */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	/* set input file name */
	if ((infile = fopen(filename, "rb")) == NULL) 
    {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}

	jpeg_stdio_src(&cinfo, infile);

	/* read header */
	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);
	width = cinfo.output_width;
	height = cinfo.output_height;
	
	fclose(infile);
}

void read_jpeg(const char *filename, unsigned char *image, int width, int height)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	FILE *infile;
	JSAMPROW row_pointer; /* pointer to a row */
	int j, k;

	/* create jpeg decompress object */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	/* set input file name */
	if ((infile = fopen(filename, "rb")) == NULL) 
    {
		fprintf(stderr, "can't open %s\n", filename);
		exit(1);
	}

	jpeg_stdio_src(&cinfo, infile);

	/* read header */
	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);
	if (width != cinfo.output_width || height != cinfo.output_height)
    {
        throw std::runtime_error("Error: Actual JPEG resolution does not match the provided one.");
    }

	row_pointer = (JSAMPROW) malloc(sizeof(JSAMPLE)*(width)*3);

	while (cinfo.output_scanline < cinfo.output_height) 
    {
		jpeg_read_scanlines(&cinfo, &row_pointer, 1);

		for(j=0; j < width; j++) 
        {
			for(k=0; k < 3; k++)
            {
                image[(cinfo.output_scanline - 1) * width * 3 + j * 3 + k] = (unsigned char) row_pointer[3*j + k];
            }
        }
	}

	jpeg_finish_decompress(&cinfo);
	fclose(infile);
	free(row_pointer);
	jpeg_destroy_decompress(&cinfo);
}

void write_jpeg(char *filename, unsigned char *image, int width, int height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE *outfile;
    JSAMPROW row_pointer = (JSAMPROW) malloc(sizeof(JSAMPLE)*width*3); /* pointer to a row */
    int j, k;

    /* create jpeg compress object */
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    /* set output file name */
    if ((outfile = fopen(filename, "wb")) == NULL) 
	{
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    /* set parameters */
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);

    jpeg_start_compress(&cinfo, TRUE);
    while (cinfo.next_scanline < cinfo.image_height) 
	{
        /* this loop converts each element to JSAMPLE */
        for(j=0; j < width; j++) 
        {
            for(k=0; k < 3; k++) 
            {
                row_pointer[3*j + k] = (JSAMPLE) image[cinfo.next_scanline * width * 3 + j * 3 + k];
            }
        }
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    free(row_pointer);
    jpeg_destroy_compress(&cinfo);
}
