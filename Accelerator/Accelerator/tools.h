/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: ¹¤¾ßº¯Êý
\******************************************************************************/

#ifndef TOOLS_H
#define TOOLS_H

#include <Windows.h>


void SetNopCode(BYTE* pnop, size_t size);

void MemCopy(void* dest, void*src, size_t size);

wchar_t *AnsiToUnicode(const char *str);

#endif TOOLS_H