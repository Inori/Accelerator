/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: ��������ı���Ҫ����
\******************************************************************************/

#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include <Windows.h>
#include <string>
using namespace std;

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_STROKER_H

//����߰汾GDI+, ��ʹ��һЩ��Ⱦ����
#define GDIPVER 0x0110

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

//��8λ�Ҷ�ͼת����32λRGBAͼ
void Cvt8BitTo32Bit(BYTE* dst, BYTE*src, DWORD nWidth, DWORD nHeight);


//FreeTypeȡ���Ķ���8λ�Ҷ�ͼ��
typedef struct _charBitmap
{
	int bmp_width;  //λͼ��
	int bmp_height; //λͼ��
	int bearingX;   //����ˮƽ�ı����У����Ǵӵ�ǰ���λ�õ�����ͼ������ߵı߽��ˮƽ����
	int bearingY;   //����ˮƽ�ı����У����Ǵӵ�ǰ���λ�ã�λ�ڻ��ߣ�������ͼ�����ϱߵı߽��ˮƽ���롣 
	int Advance;    //����ˮƽ�ı����У���������Ϊ�ַ�����һ���ֱ�����ʱ�����������ӱ�λ�õ�ˮƽ����
	BYTE* bmpBuffer; //��������
} charBitmap;

class FreeType
{
public:
	FreeType();
	FreeType(string fontname, int fontsize);

	//���ò���ʼ������
	bool SetFont(string fontname, int fontsize);
	//��ȡ��ģͼ�����Ϣ
	charBitmap GetCharBitmap(wchar_t wchar);

	~FreeType();

private:
	FT_Library library;
	FT_Face face;

	//���建�棬�����˳�(����)ǰ�����ͷţ�
	BYTE* fontdata;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
//�����ʼ��
typedef struct _TextColor
{
	_TextColor(){}
	_TextColor(BYTE bi, BYTE gi, BYTE ri, BYTE ai = 255)
	{
		b = bi;
		g = gi;
		r = ri;
		a = ai;
	}

	struct
	{
		BYTE b, g, r, a;
	};
}TextColor;

enum TextEffect
{
	Glow = 0,
	Shadow
};


class Drawer
{
public:

	virtual bool InitDrawer(string fontname, int fontsize) = 0;
	//virtual void SetTextColor(TextColor color) = 0;

	//effect: Ч������
	//color: ��ߡ���Ӱ���ⷢ�����ɫ
	//d_pixwidth: ��ߡ���Ӱ���ⷢ��ʱ���ֱ�Ե�����ֱ�Ե���ؿ��
	//radius: �ⷢ��뾶
	//virtual void ApplyEffect(TextEffect effect, TextColor color, int d_pixwidth, float radius = 0.0) = 0;
	virtual void DrawString(HDC hdc, wstring str, int xdest, int ydest, int lineHeight) = 0;
};


class GdipDrawer
{
public:
	GdipDrawer();
	GdipDrawer(string fontname, int fontsize);

	bool InitDrawer(string fontname, int fontsize);

	void SetTextColor(TextColor color);

	//effect: Ч������
	//color: ��ߡ���Ӱ���ⷢ�����ɫ
	//d_pixwidth: ������ߡ���Ӱ���ⷢ��ʱ���ֱ�Ե�����ֱ�Ե���ؿ��
	//radius: �ⷢ��뾶
	void ApplyEffect(TextEffect effect, TextColor color, float _d_pixwidth = 2, float _radius = 0.0);

	void DrawString(HDC hdc, wstring str, int xdest, int ydest, int lineHeight);

	~GdipDrawer();

private:
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	FreeType ft;
	charBitmap cbmp;

	bool has_color;
	bool has_glow;
	bool has_shadow;

	int d_pixwidth;
	int radius;
	ColorMatrix text_colorMatrix;
	ColorMatrix effect_colorMatrix;
};




#endif