#ifndef PNGFILE_H
#define PNGFILE_H
#include <string>
#include <stdio.h>

#include "zlib.h"
#include "png.h"

using namespace std;

#define PNG_BYTES_TO_CHECK 4
#define HAVE_ALPHA 1
#define NO_ALPHA 0


typedef struct _pic_data
{
	unsigned int width, height; /* �ߴ� */
	int bit_depth;  /* λ�� */
	int flag;   /* һ����־����ʾ�Ƿ���alphaͨ�� */

	unsigned char *rgba; /* ͼƬ���� */
} pic_data;

int read_png_file(string filepath, pic_data *out);

int write_png_file(string file_name, pic_data *graph);


#endif