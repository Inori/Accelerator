// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include <Windows.h>

#include "detours.h"
#include "Accelerator.h"

///////////////////��������///////////////////////////////////////////////////////
#define ACR_GBKFONT
//#define ACR_DRAWTEXT


///////////////��������///////////////////////////////////////////////////////////

#ifdef ACR_DRAWTEXT

int charbyte(char* c)
{
	if ((int)c > 0x00 && (int)c < 0x7F)
		return 1;
	else if ((int)c == 0x00)
		return 0;
	else
		return 2;
}

char *s = "ACR���ܲ��ԣ����β��ԣ�ACR���ܲ��ԣ����β��ԣ�";

PVOID g_pOldTextOutA = (void*)0x41AB3C;
typedef bool (WINAPI *PfuncTextOutA)(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString);
bool WINAPI NewTextOutA(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString)
{
	//������ջ
	__asm sub ebp, 4

	lpString = s;
	cbString = charbyte(s);
	s += cbString;
	return ((PfuncTextOutA)g_pOldTextOutA)(hdc, nXStart, nYStart, lpString, cbString);
}

#endif
////////////�����ַ���////////////////////////////////////////////////////////
#ifdef ACR_GBKFONT

PVOID g_pOldCreateFontIndirectA = NULL;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = ANSI_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;
	strcpy(lplf->lfFaceName, "����");
	
	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

#endif


//��װHook 
void SetHook()
{
#ifdef ACR_DRAWTEXT
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&g_pOldTextOutA, NewTextOutA);
	DetourTransactionCommit();
#endif


#ifdef ACR_GBKFONT
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldCreateFontIndirectA = DetourFindFunction("GDI32.dll", "CreateFontIndirectA");
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourTransactionCommit();
#endif
}

__declspec(dllexport)void WINAPI Dummy()
{
}

BOOL APIENTRY DllMain( HMODULE hModule,
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
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

