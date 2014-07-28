#include "Accelerator.h"


///////////////////功能设置：用来定义本次编译需要完成的功能///////////////////////

//中文字符集
//#define ACR_GBKFONT

//绘制字体
//#define ACR_DRAWTEXT

//动态汉化
#define ACR_TRANSLATE


//包括以下在内的其他功能：
//1、修改窗口标题
//#define ACR_EXTRA

////////////全局变量////////////////////////////////////////////////////////

#ifdef ACR_DRAWTEXT

GdipDrawer gdrawer;
#endif

#ifdef ACR_TRANSLATE

StringInjector injector;

#endif





////////////中文字符集////////////////////////////////////////////////////////
#ifdef ACR_GBKFONT

PVOID g_pOldCreateFontIndirectA = NULL;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = ANSI_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;

	//修改后的字体，包括音符等特殊符号
	strcpy(lplf->lfFaceName, "黑体");

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

#endif

///////////////修改少量系统文字///////////////////////////////////////////////////
#ifdef ACR_EXTRA
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


///////////////绘制文字///////////////////////////////////////////////////////////


#ifdef ACR_DRAWTEXT



void WINAPI FT_TextOut(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int strlen)
{
	wstring ws(AnsiToUnicode(lpString), strlen);
	gdrawer.DrawString(hdc, ws, nXStart, nYStart);
}

void* g_p_textout_black = (void*)0x4069BE;
void* g_p_textout_white = (void*)0x406936;

__declspec(naked) void __stdcall ft_textout_white()
{
	__asm
	{
		call FT_TextOut
		jmp g_p_textout_white
	}
	
}

__declspec(naked) void __stdcall ft_textout_black()
{
	__asm
	{
		call FT_TextOut
		jmp g_p_textout_black
	}

}

#endif

///////////////替换字符串/////////////////////////////////////////////////
#ifdef ACR_TRANSLATE

void __stdcall copy_string(DWORD offset)
{
	DWORD oldlen = wstrlen((wchar_t*)offset);

	injector.Inject((void*)offset, oldlen);
}

PVOID phookaddr = (PVOID)0x49A48F;
__declspec(naked)void inject_string()
{
	__asm
	{
		pushad
		push ecx
		call copy_string
		popad
		jmp phookaddr
	}
}

#endif


//////////////////////////////////////////////////////////////////////////
//安装Hook 
void SetHook()
{
#ifdef ACR_DRAWTEXT
	DetourTransactionBegin();
	DetourAttach(&g_p_textout_white, ft_textout_white);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach(&g_p_textout_black, ft_textout_black);
	DetourTransactionCommit();
#endif


#ifdef ACR_GBKFONT
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldCreateFontIndirectA = DetourFindFunction("GDI32.dll", "CreateFontIndirectA");
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourTransactionCommit();
#endif


#ifdef ACR_TRANSLATE
	DetourTransactionBegin();
	DetourAttach(&phookaddr, inject_string);
	DetourTransactionCommit();

#endif


#ifdef ACR_EXTRA
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldSetWindowTextA = DetourFindFunction("USER32.dll", "SetWindowTextA");
	DetourAttach(&g_pOldSetWindowTextA, NewSetWindowTextA);
	DetourTransactionCommit();
#endif
}

void InitProc()
{
#ifdef ACR_DRAWTEXT
	TextColor color(251, 238, 245, 255);
	//TextColor effcl(0, 0, 0, 255);

	gdrawer.InitDrawer("simhei.ttf",89);
	gdrawer.SetTextColor(color);
	//gdrawer.ApplyEffect(Shadow, effcl, 2, 2.0);

	SetNopCode((BYTE*)g_p_textout_white, 6);
	SetNopCode((BYTE*)g_p_textout_black, 6);
#endif

#ifdef ACR_TRANSLATE
	injector.Init("");

#endif
	SetHook();
}

//需要一个导出函数
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
		InitProc();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


