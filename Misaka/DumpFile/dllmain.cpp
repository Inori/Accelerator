// dllmain.cpp : ���ڴ���dump�ű���ͼƬ�ļ���
// code by ����, 2014/2/25

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <map>
#include <string>
#include <shlwapi.h>
#include "detours.h"
#include "zlib.h"
using namespace std;

///////////////////ȫ�ֱ���//////////////////////////////////////////////////////////


char* Name = NULL;
DWORD DataOffset = 0;
BYTE* out_buff = NULL;
DWORD out_len = 0;

bool ready = false;
///////////////���ù���//////////////////////////////////////////////////////////////
void SetNopCode(BYTE* pnop, size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	for (size_t i = 0; i<size; i++)
	{
		pnop[i] = 0x90;
	}
}

void memcopy(void* dest, void*src, size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(dest, src, size);
}





///////////////�ռ�filename, offset, size��Ϣ//////////////////////////////////////////////////////

PVOID pGetName = (PVOID)0x47387E;
__declspec(naked)void GetName()
{
	__asm
	{
		pushad
			mov Name, edi
			popad
			jmp pGetName
	}
}

PVOID pDataOffset = (PVOID)0x4734AD;
__declspec(naked)void GetDataOffset()
{
	__asm
	{
		pushad
			mov DataOffset, esi
			popad
			jmp pDataOffset
	}
}

//zlibѹ����Ϣ��׵����{��Rioshiina������
PVOID pFileInfo = (PVOID)0x4754F6;
__declspec(naked)void GetFileInfo()
{
	__asm
	{
		pushad
			mov out_buff, eax
			mov out_len, ecx
			mov ready, 1
			popad
			jmp pFileInfo
	}
}

///////////////ѡ�����λ�ô��ڴ���dump file//////////////////////////////////////////////////////
void WINAPI DumpFile()
{
	if (ready)
	{
		if (Name != NULL)
		{
			if (!strcmp(strchr(Name, '.'), ".txt") || !strcmp(strchr(Name, '.'), ".TXT"))
			{
				//MessageBoxA(NULL, Name, "Test", MB_OK);
				HANDLE hfile = CreateFileA(Name, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_ARCHIVE, NULL);
				if (hfile == INVALID_HANDLE_VALUE)
				{
					MessageBoxA(NULL, "Create File Erroe!", "Test", MB_OK);
					CloseHandle(hfile);
					ready = false;
					return;
				}
				DWORD outlen = 0;
				WriteFile(hfile, (LPCVOID)out_buff, out_len, &outlen, NULL);

				CloseHandle(hfile);
			}
		}
		ready = false;
	}
}

PVOID pDumpFile = (PVOID)0x47379D;
__declspec(naked)void _DumpFile()
{
	__asm
	{
		pushad
			call DumpFile
			popad
			jmp pDumpFile
	}
}



//��װHook 
void SetHook()
{

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pGetName, GetName);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pFileInfo, GetFileInfo);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pDumpFile, _DumpFile);
	DetourTransactionCommit();
}

//ж��Hook
void DropHook()
{

}

__declspec(dllexport)void WINAPI Dummy()
{

}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SetHook();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DropHook();
		break;
	}
	return TRUE;
}

