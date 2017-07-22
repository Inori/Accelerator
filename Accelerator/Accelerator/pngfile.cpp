#include "pngfile.h"

bool PngFile::ReadPngFile(const char* pszFileName, PNG_DATA* pPngData)
{
	if (!pszFileName || !pPngData)
	{
		return false;
	}

	FILE* fPng = fopen(pszFileName, "rb+");

	bool bSuc = ReadPngFile(fPng, pPngData);

	fclose(fPng);

	return bSuc;
}

bool PngFile::ReadPngFile(FILE* pFile, PNG_DATA* pPngData)
{
	if (!pFile || !pPngData)
	{
		return false;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

	png_init_io(png_ptr, pFile);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, NULL);

	int nChannels = png_get_channels(png_ptr, info_ptr); 
	int nColorType = png_get_color_type(png_ptr, info_ptr);

	int nWidth = png_get_image_width(png_ptr, info_ptr);
	int nHeight = png_get_image_height(png_ptr, info_ptr);
	png_bytepp pRowPointers = png_get_rows(png_ptr, info_ptr);

	bool bReadSuccess = false;
	if (nChannels == 4 || nColorType == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		bReadSuccess = ReadRGBA(pRowPointers, nWidth, nHeight, pPngData);
	}
	else if (nChannels == 3 || nColorType == PNG_COLOR_TYPE_RGB)
	{
		bReadSuccess = ReadRGB(pRowPointers, nWidth, nHeight, pPngData);
	}
	else
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return false;
	};

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return bReadSuccess;
}

bool PngFile::WritePngFile(const char* pszFileName, PNG_DATA* pPngData)
{
	if (!pszFileName || !pPngData)
	{
		return false;
	}

	FILE* fPng = fopen(pszFileName, "wb");
	bool bSuc = WritePngFile(fPng, pPngData);
	fclose(fPng);

	return bSuc;
}

bool PngFile::WritePngFile(FILE* pFile, PNG_DATA* pPngData)
{
	if (!pFile || !pPngData)
	{
		return false;
	}

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		return false;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	png_init_io(png_ptr, pFile);


	int nWidth = pPngData->nWidth;
	int nHeight = pPngData->nHeight;

	int nColorType = 0;
	int nChannels = 0;
	if (pPngData->eFlag == HAVE_ALPHA)
	{
		nColorType = PNG_COLOR_TYPE_RGB_ALPHA;
		nChannels = 4;
	}
	else
	{
		nColorType = PNG_COLOR_TYPE_RGB;
		nChannels = 3;
	}

	png_set_IHDR(png_ptr, info_ptr, nWidth, nHeight,
		8, nColorType, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return false;
	}

	int nStride = nChannels * nWidth;
	int nImageSize = nWidth * nHeight * nChannels;
    png_bytepp row_pointers = (png_bytep*)malloc(nImageSize);

	int x = 0, y = 0;
	png_bytep pRgba = pPngData->pRgba;
	for (y = 0; y < nHeight; ++y)
	{
		row_pointers[y] = (png_bytep)malloc(nStride);
		if (pPngData->eFlag == HAVE_ALPHA)
		{
			for (x = 0; x < nStride; x += 4)
			{
				row_pointers[y][x] = pRgba[y*nStride + x + 2];
				row_pointers[y][x + 1] = pRgba[y*nStride + x + 1];
				row_pointers[y][x + 2] = pRgba[y*nStride + x];
				row_pointers[y][x + 3] = pRgba[y*nStride + x + 3]; //alpha
			}
		}
		else
		{
			for (x = 0; x < nStride; x += 3)
			{
				row_pointers[y][x] = pRgba[y*nStride + x];
				row_pointers[y][x + 1] = pRgba[y*nStride + x + 1];
				row_pointers[y][x + 2] = pRgba[y*nStride + x + 2];
			}
		}

	}

	png_write_image(png_ptr, row_pointers);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		return false;
	}

	png_write_end(png_ptr, NULL);

	for (y = 0; y < nHeight; ++y)
		free(row_pointers[y]);
	free(row_pointers);
	return true;
}

bool PngFile::ReadRGBA(png_bytepp pRow, int nWidth, int nHeight, PNG_DATA* pPngData)
{
	if (!pRow || !pPngData)
	{
		return false;
	}

	int nImageSize = nWidth * nHeight * 4;
	png_bytep pRgba = (png_bytep)malloc(nImageSize);
	if (!pRgba)
	{
		return false;
	}

	pPngData->nWidth = nWidth;
	pPngData->nHeight = nHeight;
	pPngData->eFlag = HAVE_ALPHA;
	pPngData->pRgba = pRgba;

	int nStride = 4 * nWidth;
	int x = 0, y = 0;
	for (y = 0; y < nHeight; ++y)
	{
		for (x = 0; x < nStride; x += 4)
		{
			pRgba[y*nStride + x] = pRow[y][x + 2];
			pRgba[y*nStride + x + 1] = pRow[y][x + 1];
			pRgba[y*nStride + x + 2] = pRow[y][x];
			pRgba[y*nStride + x + 3] = pRow[y][x + 3];
		}
	}
	return true;
}

bool PngFile::ReadRGB(png_bytepp pRow, int nWidth, int nHeight, PNG_DATA* pPngData)
{
	if (!pRow || !pPngData)
	{
		return false;
	}

	int nImageSize = nWidth * nHeight * 3;
	png_bytep pRgba = (png_bytep)malloc(nImageSize);
	if (!pRgba)
	{
		return false;
	}

	pPngData->nWidth = nWidth;
	pPngData->nHeight = nHeight;
	pPngData->eFlag = NO_ALPHA;
	pPngData->pRgba = pRgba;

	int nStride = 3 * nWidth;
	int x = 0, y = 0;
	for (y = 0; y < nHeight; ++y)
	{
		for (x = 0; x < nStride; x += 4)
		{
			pRgba[y*nStride + x] = pRow[y][x];
			pRgba[y*nStride + x + 1] = pRow[y][x + 1];
			pRgba[y*nStride + x + 2] = pRow[y][x + 2];
		}
	}
	return true;
}

