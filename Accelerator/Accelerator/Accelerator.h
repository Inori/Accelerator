#ifndef ACCELERATOR_H
#define ACCELERATOR_H

#include <Windows.h>


void SetNopCode(BYTE* pnop, size_t size);
void memcopy(void* dest, void*src, size_t size);

#endif