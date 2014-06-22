#include "drawtext.h"


////////////////////////////////FreeType/////////////////////////////////////////


FreeType::FreeType()
{
}

FreeType::FreeType(string fontname, int fontsize)
{
	SetFont(fontname, fontsize);
}

void FreeType::SetFont(string fontname, int fontsize)
{
	HANDLE hfile = CreateFile(fontname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD filesize = GetFileSize(hfile, NULL);
	DWORD sizeread = 0;
	fontdata = new BYTE[filesize];
	ReadFile(hfile, (LPVOID)fontdata, filesize, &sizeread, NULL);
	CloseHandle(hfile);

	//≥ı ºªØFreeType
	FT_Init_FreeType(&library);
	FT_New_Memory_Face(library, fontdata, filesize, 0, &face);

	FT_Set_Char_Size(face, fontsize << 6, fontsize << 6, 90, 90);
}


FreeType::~FreeType()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	delete[]fontdata;
}
