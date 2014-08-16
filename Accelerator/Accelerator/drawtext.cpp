#include "drawtext.h"


////////////////////////////////FreeType/////////////////////////////////////////


FreeType::FreeType()
{
}

FreeType::FreeType(string fontname, int fontsize)
{
	SetFont(fontname, fontsize);
}

bool FreeType::SetFont(string fontname, int fontsize)
{
	HANDLE hfile = CreateFile(fontname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "CreateFile Error!", "Error", MB_OK);
		return false;
	}

	DWORD filesize = GetFileSize(hfile, NULL);
	DWORD sizeread = 0;
	fontdata = new BYTE[filesize];
	ReadFile(hfile, (LPVOID)fontdata, filesize, &sizeread, NULL);
	CloseHandle(hfile);

	FT_Error error;
	//��ʼ��FreeType
	error = FT_Init_FreeType(&library);
	if (error)
	{
		MessageBoxA(NULL, "Init_FreeType Error!", "Error", MB_OK);
		return false;
	}

	error = FT_New_Memory_Face(library, fontdata, filesize, 0, &face);
	if (error)
	{
		MessageBoxA(NULL, "New_Memory_Face Error!", "Error", MB_OK);
		return false;
	}

	error = FT_Set_Char_Size(face, fontsize << 6, fontsize << 6, 90, 90);
	if (error)
	{
		MessageBoxA(NULL, "Set_Char_Size Error!", "Error", MB_OK);
		return false;
	}
	
	return true;
}

charBitmap FreeType::GetCharBitmap(wchar_t wchar)
{
	charBitmap cbmp;

	FT_GlyphSlot slot = face->glyph;
	FT_Bitmap bmp;
	FT_Error error;

	//����Glyph��ת����256ɫ�����λͼ
	error = FT_Load_Char(face, wchar, FT_LOAD_RENDER);

	if (error)
	{
		MessageBox(NULL, "Load_Char Error", "Error", MB_OK);
	}

	bmp = slot->bitmap;

	cbmp.bmp_width = bmp.width;
	cbmp.bmp_height = bmp.rows;
	cbmp.bearingX = slot->bitmap_left;
	cbmp.bearingY = slot->bitmap_top;
	

	//��slot->advance���ӱ�λ�ã�slot->advance�������εĲ�����ȣ�Ҳ����ͨ����˵���߸�(escapement)����
	//����ʸ�������ص�1/64Ϊ��λ��ʾ��������ÿһ�ε�����ɾ��Ϊ��������
	cbmp.Advance = slot->advance.x >> 6;
	//�ⲿ���ڴ治��Ҫ�ֶ��ͷţ�FT���´�װ�����β۵�ʱ���Ĩ���ϴε�����
	cbmp.bmpBuffer = bmp.buffer;

	return cbmp;
}


FreeType::~FreeType()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	delete[]fontdata;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//�����ڴ����ⲿ���룬�ⲿ�ͷ�
void Cvt8BitTo32Bit(BYTE* dst, BYTE*src, DWORD nWidth, DWORD nHeight)
{

	//��ͨ��
	static const int pix_size = 4;

	BYTE *srcline;
	BYTE *dstline;
	BYTE pix;

	for (int y = 0; y < nHeight; y++)
	{
		srcline = &src[y * nWidth];
		//srcline = &src[(nHeight - y - 1)*nWidth]; //��ֱ��תͼ��
		dstline = &dst[y * nWidth * pix_size];
		for (int x = 0; x < nWidth; x++)
		{
			pix = srcline[x];
			dstline[x*pix_size] = pix;
			dstline[x*pix_size + 1] = pix;
			dstline[x*pix_size + 2] = pix;
			dstline[x*pix_size + 3] = pix;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
GdipDrawer::GdipDrawer()
{
}

GdipDrawer::GdipDrawer(string fontname, int font_size)
{
	InitDrawer( fontname, font_size);
	
}

bool GdipDrawer::InitDrawer(string fontname, int font_size)
{

	// ��ʼ�� GDI+
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// ��ʼ��FreeType
	if (!ft.SetFont(fontname, font_size))
		return false;

	fontsize = font_size;

	has_color = false;
	has_glow = false;
	has_shadow = false;
	return true;
}


void GdipDrawer::SetTextColor(TextColor color)
{
	text_colorMatrix =
	{ 
		(float)color.b / 255.0,	0.0f,					0.0f,					0.0f,					0.0f,
		0.0f,					(float)color.g / 255.0,	0.0f,					0.0f,					0.0f,
		0.0f,					0.0f,					(float)color.r / 255.0, 0.0f,					0.0f,
		0.0f,					0.0f,					0.0f,					(float)color.a / 255.0, 0.0f,
		0.0f,					0.0f,					0.0f,					0.0f,					1.0f 
	};

	has_color = true;
}


void GdipDrawer::ApplyEffect(TextEffect effect, TextColor color, float _d_pixwidth, float _radius)
{
	//���Ч����ɫ����
	effect_colorMatrix =
	{
		(float)color.b / 255.0, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, (float)color.g / 255.0, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, (float)color.r / 255.0, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, (float)color.a / 255.0, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	d_pixwidth = _d_pixwidth;
	radius = _radius;

	if (effect == Shadow)
	{
		has_shadow = true;

	}
	else if (effect == Glow)
	{
		has_glow = true;
	}
	else
	{
		MessageBox(NULL, "Effect not support!", "Error", MB_OK);
		return;
	}
	
}



//lineHeight���������ָ߶�
//void  GdipDrawer::DrawString(HDC hdc, wstring str, int xdest, int ydest, int lineHeight)
void  GdipDrawer::DrawString(HDC hdc, wstring str, int xdest, int ydest)
{
	//�������
	int pen_x = xdest;
	int pen_y = ydest;

	//��δ����Ч��ֵ�û��ɡ���Orz
	for each (wchar_t wc in str)
	{
		//����
		if (wc == L'\n')
		{
			pen_y += fontsize;
			pen_x = xdest;
			continue;
		}

		cbmp = ft.GetCharBitmap(wc);

		DWORD nWidth = cbmp.bmp_width;
		DWORD nHeight = cbmp.bmp_height;

		//���������Ƶ���������ͷ��ڴ棬�д��Ż�
		BYTE *src = cbmp.bmpBuffer;
		BYTE *dst = new BYTE[nWidth*nHeight*4];
		Cvt8BitTo32Bit(dst, src, nWidth, nHeight);

		//��ȡbmp�� PixelFormat32bppPARGB->Alpha���
		Gdiplus::Bitmap bitmap(nWidth, nHeight, 4*nWidth, PixelFormat32bppPARGB, dst);

		Graphics graphics(hdc);

		//��ʾ����Ч��ͼ��
		float dpixwidth = 0.0;
		if (has_shadow)
		{
			ImageAttributes iAtt;
			iAtt.SetColorMatrix(&effect_colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
			graphics.DrawImage(&bitmap,
				Rect(pen_x + d_pixwidth, fontsize - cbmp.bearingY + pen_y + d_pixwidth, nWidth + d_pixwidth, nHeight + d_pixwidth),
				0, 0, nWidth, nHeight,
				UnitPixel, &iAtt);
		}
		else if (has_glow)
		{
			ImageAttributes iAtt;
			iAtt.SetColorMatrix(&effect_colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

			Matrix myMatrix(1.3f, 0.0f, 0.0f, 1.3f, pen_x, fontsize - cbmp.bearingY + pen_y);
			RectF srcRect(0.0f, 0.0f, nWidth, nHeight);

			BlurParams myBlurParams;
			myBlurParams.expandEdge = TRUE;
			myBlurParams.radius = radius;

			Blur myBlur;
			myBlur.SetParameters(&myBlurParams);
			
			graphics.DrawImage(&bitmap, &srcRect, &myMatrix, &myBlur, &iAtt, UnitPixel);

			dpixwidth = d_pixwidth;
		}

		//��ʾǰ������ͼ��
		//����������ɫ
		if (has_color)
		{
			ImageAttributes imageAtt;
			imageAtt.SetColorMatrix(&text_colorMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
			graphics.DrawImage(&bitmap, 
				Rect(pen_x + dpixwidth, fontsize - cbmp.bearingY + pen_y + dpixwidth, nWidth, nHeight),
				0, 0, nWidth, nHeight, 
				UnitPixel, &imageAtt);
		}
		else
		{
			CachedBitmap  cBitmap(&bitmap, &graphics);
			//����CachedBitmap����
			graphics.DrawCachedBitmap(&cBitmap, pen_x + dpixwidth, fontsize - cbmp.bearingY + pen_y + dpixwidth);
		}
		


		//�߸�
		pen_x += cbmp.Advance;

		//�ض������
		//pen_x += cbmp.bearingX;
		//pen_y += cbmp.bearingY;

		
		delete[]dst;
		memset(&cbmp, 0, sizeof(charBitmap));
	}

	
}

GdipDrawer::~GdipDrawer()
{
	GdiplusShutdown(gdiplusToken);
}