#include "pngfile.h"

int read_png_file(string filepath, pic_data *out)
/* ���ڽ���pngͼƬ */
{
	FILE *pic_fp;
	pic_fp = fopen(filepath.c_str(), "rb");
	if (pic_fp == NULL) /* �ļ���ʧ�� */
		return -1;

	/* ��ʼ�����ֽṹ */
	png_structp png_ptr;
	png_infop   info_ptr;
	char        buf[PNG_BYTES_TO_CHECK];
	int         temp;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);

	setjmp(png_jmpbuf(png_ptr)); // ������Ҫ

	temp = fread(buf, 1, PNG_BYTES_TO_CHECK, pic_fp);
	temp = png_sig_cmp((png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK);

	/*����Ƿ�Ϊpng�ļ�*/
	if (temp != 0) return 1;

	rewind(pic_fp);
	/*��ʼ���ļ�*/
	png_init_io(png_ptr, pic_fp);
	// ���ļ���
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	int color_type, channels;

	/*��ȡ��ȣ��߶ȣ�λ���ɫ����*/
	channels = png_get_channels(png_ptr, info_ptr); /*��ȡͨ����*/

	out->bit_depth = png_get_bit_depth(png_ptr, info_ptr); /* ��ȡλ�� */
	color_type = png_get_color_type(png_ptr, info_ptr); /*��ɫ����*/

	int i, j;
	int size;
	/* row_pointers��߾���rgba���� */
	png_bytep* row_pointers;
	row_pointers = png_get_rows(png_ptr, info_ptr);
	out->width = png_get_image_width(png_ptr, info_ptr);
	out->height = png_get_image_height(png_ptr, info_ptr);

	size = out->width * out->height; /* ����ͼƬ�������ص����� */

	if (channels == 4 || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{/*�����RGB+alphaͨ��������RGB+�����ֽ�*/
		size *= (4 * sizeof(unsigned char)); /* ÿ�����ص�ռ4���ֽ��ڴ� */
		out->flag = HAVE_ALPHA;    /* ��� */
		out->rgba = (unsigned char*)malloc(size);
		if (out->rgba == NULL)
		{/* ��������ڴ�ʧ�� */
			fclose(pic_fp);
			puts("����(png):�޷������㹻���ڴ湩�洢����!");
			return 1;
		}

		temp = (4 * out->width);/* ÿ����4 * out->width���ֽ� */
		for (i = 0; i < out->height; i++)
		{
			for (j = 0; j < temp; j += 4)
			{/* һ���ֽ�һ���ֽڵĸ�ֵ */
				out->rgba[i*temp + j] = row_pointers[i][j+2];       // red
				out->rgba[i*temp + j + 1] = row_pointers[i][j + 1];   // green
				out->rgba[i*temp + j + 2] = row_pointers[i][j ];   // blue
				out->rgba[i*temp + j + 3] = row_pointers[i][j + 3];   // alpha
			}
		}
	}
	else if (channels == 3 || color_type == PNG_COLOR_TYPE_RGB)
	{/* �����RGBͨ�� */
		size *= (3 * sizeof(unsigned char)); /* ÿ�����ص�ռ3���ֽ��ڴ� */
		out->flag = NO_ALPHA;    /* ��� */
		out->rgba = (unsigned char*)malloc(size);
		memset(out->rgba, 0, size);
		if (out->rgba == NULL)
		{/* ��������ڴ�ʧ�� */
			fclose(pic_fp);
			puts("����(png):�޷������㹻���ڴ湩�洢����!");
			return 1;
		}

		temp = (3 * out->width);/* ÿ����3 * out->width���ֽ� */
		for (i = 0; i < out->height; i++)
		{
			for (j = 0; j < temp; j += 3)
			{/* һ���ֽ�һ���ֽڵĸ�ֵ */
				out->rgba[i*temp + j] = row_pointers[i][j];       // red
				out->rgba[i*temp + j + 1] = row_pointers[i][j + 1];   // green
				out->rgba[i*temp + j + 2] = row_pointers[i][j + 2];   // blue
			}
		}
	}
	else return 1;

	/* ��������ռ�õ��ڴ� */
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return 0;
}

int write_png_file(string file_name, pic_data *graph)
/* ���ܣ���LCUI_Graph�ṹ�е�����д����png�ļ� */
{
	int j, i, temp;
	png_byte color_type;

	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;
	/* create file */
	FILE *fp = fopen(file_name.c_str(), "wb");
	if (!fp)
	{
		printf("[write_png_file] File %s could not be opened for writing", file_name.c_str());
		return -1;
	}


	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
	{
		printf("[write_png_file] png_create_write_struct failed");
		return -1;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		printf("[write_png_file] png_create_info_struct failed");
		return -1;
	}
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("[write_png_file] Error during init_io");
		return -1;
	}
	png_init_io(png_ptr, fp);


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("[write_png_file] Error during writing header");
		return -1;
	}
	/* �ж�Ҫд�����ļ���ͼƬ�����Ƿ���͸���ȣ���ѡ��ɫ������ */
	if (graph->flag == HAVE_ALPHA) color_type = PNG_COLOR_TYPE_RGB_ALPHA;
	else color_type = PNG_COLOR_TYPE_RGB;

	png_set_IHDR(png_ptr, info_ptr, graph->width, graph->height,
		graph->bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("[write_png_file] Error during writing bytes");
		return -1;
	}
	if (graph->flag == HAVE_ALPHA) temp = (4 * graph->width);
	else temp = (3 * graph->width);

	row_pointers = (png_bytep*)malloc(graph->height*sizeof(png_bytep));

	for (i = 0; i < graph->height; i++)
	{
		row_pointers[i] = (png_bytep)malloc(sizeof(unsigned char)*temp);
		if (graph->flag == HAVE_ALPHA)
		{
			for (j = 0; j < temp; j += 4)
			{
				row_pointers[i][j] = graph->rgba[i*temp + j + 2];
				row_pointers[i][j + 1] = graph->rgba[i*temp + j +1];
				row_pointers[i][j + 2] = graph->rgba[i*temp + j ];
				row_pointers[i][j + 3] = graph->rgba[i*temp + j + 3]; //alpha
			}
		}
		else
		{
			for (j = 0; j < temp; j += 3)
			{
				row_pointers[i][j] = graph->rgba[i*temp + j];
				row_pointers[i][j + 1] = graph->rgba[i*temp + j + 1];
				row_pointers[i][j + 2] = graph->rgba[i*temp + j + 2];
			}
		}

	}

	png_write_image(png_ptr, row_pointers);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		printf("[write_png_file] Error during end of write");
		return -1;
	}
	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	for (j = 0; j<graph->height; j++)
		free(row_pointers[j]);
	free(row_pointers);
	//free(graph->rgba);
	fclose(fp);
	return 0;
}
