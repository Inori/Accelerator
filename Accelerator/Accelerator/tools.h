/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: ���ߺ���
\******************************************************************************/

#ifndef TOOLS_H
#define TOOLS_H

#include <Windows.h>


void SetNopCode(BYTE* pnop, size_t size);

void memcopy(void* dest, void*src, size_t size);

DWORD wstrlen(wchar_t *ws);

wchar_t *AnsiToUnicode(const char *str);

#endif TOOLS_H