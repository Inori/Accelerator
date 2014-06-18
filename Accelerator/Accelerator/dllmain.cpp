// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <Windows.h>

#include "detours.h"
#include "Accelerator.h"

///////////////////功能设置///////////////////////////////////////////////////////
#define ACR_GBKFONT
//#define ACR_DRAWTEXT

//#define ACR_SYSTEXT
///////////////绘制文字///////////////////////////////////////////////////////////


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

char *s = "ACR功能测试，初次测试！ACR功能测试，初次测试！";

PVOID g_pOldTextOutA = (void*)0x41AB3C;
typedef bool (WINAPI *PfuncTextOutA)(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString);
bool WINAPI NewTextOutA(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString)
{
	//调整堆栈
	__asm sub ebp, 4

	lpString = s;
	cbString = charbyte(s);
	s += cbString;
	return ((PfuncTextOutA)g_pOldTextOutA)(hdc, nXStart, nYStart, lpString, cbString);
}

#endif
////////////中文字符集////////////////////////////////////////////////////////
#ifdef ACR_GBKFONT

PVOID g_pOldCreateFontIndirectA = NULL;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	//lplf->lfCharSet = ANSI_CHARSET;
	lplf->lfCharSet = GB2312_CHARSET;
	strcpy(lplf->lfFaceName, "My黑体");
	
	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

#endif

///////////////修改少量系统文字///////////////////////////////////////////////////
#ifdef ACR_SYSTEXT
PVOID g_pOldSetWindowTextA = NULL;
typedef int (WINAPI *PfuncSetWindowTextA)(HWND hwnd, LPCTSTR lpString);
int WINAPI NewSetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if (!memcmp(lpString, "\x90\xAF\x8B\xF3", 4))
	{
		strcpy((char*)(LPCTSTR)lpString, "架向星空之桥AA");
	}
	return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
}
#endif

//安装Hook 
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

#ifdef ACR_SYSTEXT
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldSetWindowTextA = DetourFindFunction("USER32.dll", "SetWindowTextA");
	DetourAttach(&g_pOldSetWindowTextA, NewSetWindowTextA);
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

