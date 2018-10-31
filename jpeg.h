#ifndef __jpeg_h__
#define __jpeg_h__

void read_jpeg_header(const char *filename, int& width, int& height);
void read_jpeg(const char *filename, unsigned char *image, int width, int height);
void write_jpeg(char *filename, unsigned char *image, int width, int height);

#endif //__jpeg_h__
