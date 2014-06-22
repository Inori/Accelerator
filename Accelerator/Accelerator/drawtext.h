/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: 定义绘制文本需要的类
\******************************************************************************/

#ifndef DRAWTEXT_H
#define DRAWTEXT_H

#include <Windows.h>
#include <string>
using namespace std;

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_STROKER_H

#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")


class FreeType
{
public:
	FreeType();
	FreeType(string fontname, int fontsize);

	void SetFont(string fontname, int fontsize);

	~FreeType();

private:
	FT_Library library;
	FT_Face face;

	//字体缓存，程序退出(析构)前不可释放！
	BYTE* fontdata;

};








#endif