#include "tools.h"

///////////////共用工具//////////////////////////////////////////////////////////////
void SetNopCode(BYTE* pnop, size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

void MemCopy(void* dest, void*src, size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dest, src, size);
}


wchar_t *AnsiToUnicode(const char *str)
{
	static wchar_t result[1024];
	int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, str, -1, result, len);
	result[len] = L'\0';
	return result;
}
