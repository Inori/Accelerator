#include "Accelerator.h"
#include "stdio.h"

///////////////////功能设置：用来定义本次编译需要完成的功能///////////////////////

//中文字符集
#define ACR_GBKFONT

//绘制字体
//#define ACR_DRAWTEXT

//动态汉化
#define ACR_TRANSLATE


//包括以下在内的其他功能：
//1、修改窗口标题
//2、修正封包文件名
#define ACR_EXTRA


////////////全局变量////////////////////////////////////////////////////////

#ifdef ACR_DRAWTEXT

GdipDrawer gdrawer;
#endif

#ifdef ACR_TRANSLATE

ScriptParser *parser;
Translator *translator;
TranslateEngine *engine;
LogFile *logfile;
ReplaceMap g_oImageMap;
#endif





////////////中文字符集////////////////////////////////////////////////////////
#ifdef ACR_GBKFONT

PVOID g_pOldCreateFontIndirectA = CreateFontIndirectA;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = ANSI_CHARSET;
	//lplf->lfCharSet = SHIFTJIS_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;

	//修改后的字体，包括音符等特殊符号
	strcpy(lplf->lfFaceName, "楷体");

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

PVOID g_pOldCreateFontIndirectW = CreateFontIndirectW;
typedef int (WINAPI *PfuncCreateFontIndirectW)(LOGFONTW *lplf);
int WINAPI NewCreateFontIndirectW(LOGFONTW *lplf)
{
	lplf->lfCharSet = ANSI_CHARSET;
	//lplf->lfCharSet = SHIFTJIS_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;

	//修改后的字体，包括音符等特殊符号
	wcscpy(lplf->lfFaceName, L"楷体");

	return ((PfuncCreateFontIndirectW)g_pOldCreateFontIndirectW)(lplf);
}

PVOID g_pOldCreateFontA = CreateFontA;
typedef int (WINAPI *PfuncCreateFontA)(int nHeight,
	int nWidth,
	int nEscapement,
	int nOrientation,
	int fnWeight,
	DWORD fdwltalic,
	DWORD fdwUnderline,
	DWORD fdwStrikeOut,
	DWORD fdwCharSet,
	DWORD fdwOutputPrecision,
	DWORD fdwClipPrecision,
	DWORD fdwQuality,
	DWORD fdwPitchAndFamily,
	LPCTSTR lpszFace);
int WINAPI NewCreateFontA(int nHeight,
	int nWidth,
	int nEscapement,
	int nOrientation,
	int fnWeight,
	DWORD fdwltalic,
	DWORD fdwUnderline,
	DWORD fdwStrikeOut,
	DWORD fdwCharSet,
	DWORD fdwOutputPrecision,
	DWORD fdwClipPrecision,
	DWORD fdwQuality,
	DWORD fdwPitchAndFamily,
	LPCTSTR lpszFace)
{
	fdwCharSet = ANSI_CHARSET;
	LPCTSTR lpFontName = "楷体";
	return ((PfuncCreateFontA)g_pOldCreateFontA)(nHeight,
		nWidth,
		nEscapement,
		nOrientation,
		fnWeight,
		fdwltalic,
		fdwUnderline,
		fdwStrikeOut,
		fdwCharSet,
		fdwOutputPrecision,
		fdwClipPrecision,
		fdwQuality,
		fdwPitchAndFamily,
		lpFontName);
}

PVOID g_pOldGetGlyphOutlineW = GetGlyphOutlineW;
typedef DWORD (WINAPI *PfuncGetGlyphOutlineW)(
	HDC            hdc,
	UINT           uChar,
	UINT           uFormat,
	LPGLYPHMETRICS lpgm,
	DWORD          cbBuffer,
	LPVOID         lpvBuffer,
	const MAT2     *lpmat2
);

DWORD WINAPI NewGetGlyphOutlineW(
	HDC			   hdc,
	UINT           uChar,
	UINT           uFormat,
	LPGLYPHMETRICS lpgm,
	DWORD          cbBuffer,
	LPVOID         lpvBuffer,
	const MAT2     *lpmat2)
{

	HFONT hOldFont = NULL;
	if (uChar == 0xEEA1)  //a1ee=☆
	{
		uChar = 0x8140;

		LOGFONTW lpFont = { 0 };
		lpFont.lfWidth = 0xFFFFFFEC;
		lpFont.lfCharSet = SHIFTJIS_CHARSET;
		lpFont.lfQuality = ANTIALIASED_QUALITY;
		//wcscpy(lpFont.lfFaceName, L"MS ゴシック");
		HFONT hFont = CreateFontIndirectW(&lpFont);
		if (hFont && hdc)
		{
			hOldFont = (HFONT)SelectObject(hdc, hFont);
		}
	}

	DWORD dwRet = ((PfuncGetGlyphOutlineW)g_pOldGetGlyphOutlineW)(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	if (hOldFont && hdc)
	{
		SelectObject(hdc, hOldFont);
	}

	return dwRet;
}

PVOID g_pOldMultiByteToWideChar = MultiByteToWideChar;
typedef int (WINAPI *PfuncMultiByteToWideChar)(UINT CodePage,DWORD dwFlags,LPCSTR lpMultiByteStr,int cbMultiByte,LPWSTR lpWideCharStr,int cchWideChar);
int WINAPI NewMultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar
	)
{
	if (CodePage == 932)
		CodePage = 936;
	return ((PfuncMultiByteToWideChar)g_pOldMultiByteToWideChar)(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}


PVOID g_pOldWideCharToMultiByte = WideCharToMultiByte;
typedef int(WINAPI *PfuncWideCharToMultiByte)(
	UINT CodePage,
	DWORD dwFlags, 
	LPCWSTR lpWideCharStr, 
	int cchWideChar, 
	LPSTR lpMultiByteStr,
	int cchMultiByte,
	LPCSTR lpDefaultChar, 
	LPBOOL pfUsedDefaultChar
);
int WINAPI NewWideCharToMultiByte(
	UINT CodePage,
	DWORD dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cchMultiByte,
	LPCSTR lpDefaultChar,
	LPBOOL pfUsedDefaultChar
)
{
	CodePage = 932;
	return ((PfuncWideCharToMultiByte)g_pOldWideCharToMultiByte)(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cchMultiByte, lpDefaultChar, pfUsedDefaultChar);
}

#endif

///////////////修改少量系统文字///////////////////////////////////////////////////
#ifdef ACR_EXTRA

LPCSTR g_szGameWinTitleA = "Steam Prison —— ★乙女浮世绘★";
LPCWSTR g_szGameWinTitleW = L"Steam Prison —— ★乙女浮世绘★";

PVOID g_pOldSetWindowTextA = SetWindowTextA;
typedef int (WINAPI *PfuncSetWindowTextA)(HWND hwnd, LPCSTR lpString);
int WINAPI NewSetWindowTextA(HWND hwnd, LPCSTR lpString)
{
	//int nRet = SetWindowTextW(hwnd, g_szGameWinTitleW);
	int nRet = ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, g_szGameWinTitleA);
	return nRet;//((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
}

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND (WINAPI *PfuncCreateWindowExA)(
	DWORD dwExStyle,//窗口的扩展风格
	LPCSTR lpClassName,//指向注册类名的指针
	LPCSTR lpWindowName,//指向窗口名称的指针
	DWORD dwStyle,//窗口风格
	int x,//窗口的水平位置
	int y,//窗口的垂直位置
	int nWidth,//窗口的宽度
	int nHeight,//窗口的高度
	HWND hWndParent,//父窗口的句柄
	HMENU hMenu,//菜单的句柄或是子窗口的标识符
	HINSTANCE hInstance,//应用程序实例的句柄
	LPVOID lpParam//指向窗口的创建数据
	);

HWND WINAPI NewCreateWindowExA(
	DWORD dwExStyle,//窗口的扩展风格
	LPCSTR lpClassName,//指向注册类名的指针
	LPCSTR lpWindowName,//指向窗口名称的指针
	DWORD dwStyle,//窗口风格
	int x,//窗口的水平位置
	int y,//窗口的垂直位置
	int nWidth,//窗口的宽度
	int nHeight,//窗口的高度
	HWND hWndParent,//父窗口的句柄
	HMENU hMenu,//菜单的句柄或是子窗口的标识符
	HINSTANCE hInstance,//应用程序实例的句柄
	LPVOID lpParam//指向窗口的创建数据
	)
{
	const char* titlename = g_szGameWinTitleA;

	return ((PfuncCreateWindowExA)g_pOldCreateWindowExA)(
		dwExStyle,//窗口的扩展风格
		lpClassName,//指向注册类名的指针
		titlename,//指向窗口名称的指针
		dwStyle,//窗口风格
		x,//窗口的水平位置
		y,//窗口的垂直位置
		nWidth,//窗口的宽度
		nHeight,//窗口的高度
		hWndParent,//父窗口的句柄
		hMenu,//菜单的句柄或是子窗口的标识符
		hInstance,//应用程序实例的句柄
		lpParam//指向窗口的创建数据
		);
}

PVOID g_pOldSetWindowTextW = SetWindowTextW;
typedef int (WINAPI *PfuncSetWindowTextW)(HWND hwnd, LPCWSTR lpString);
int WINAPI NewSetWindowTextW(HWND hwnd, LPCWSTR lpString)
{
	if (!memcmp(lpString, "壞僲塉", 6))
	{
		strcpy((char*)(LPCWSTR)lpString, "架向星空之桥AA");
	}
	return ((PfuncSetWindowTextW)g_pOldSetWindowTextW)(hwnd, lpString);
}

PVOID g_pOldCreateWindowExW = CreateWindowExW;
typedef HWND(WINAPI *PfuncCreateWindowExW)(
	DWORD dwExStyle,//窗口的扩展风格
	LPCWSTR lpClassName,//指向注册类名的指针
	LPCWSTR lpWindowName,//指向窗口名称的指针
	DWORD dwStyle,//窗口风格
	int x,//窗口的水平位置
	int y,//窗口的垂直位置
	int nWidth,//窗口的宽度
	int nHeight,//窗口的高度
	HWND hWndParent,//父窗口的句柄
	HMENU hMenu,//菜单的句柄或是子窗口的标识符
	HINSTANCE hInstance,//应用程序实例的句柄
	LPVOID lpParam//指向窗口的创建数据
	);

HWND WINAPI NewCreateWindowExW(
	DWORD dwExStyle,//窗口的扩展风格
	LPCWSTR lpClassName,//指向注册类名的指针
	LPCWSTR lpWindowName,//指向窗口名称的指针
	DWORD dwStyle,//窗口风格
	int x,//窗口的水平位置
	int y,//窗口的垂直位置
	int nWidth,//窗口的宽度
	int nHeight,//窗口的高度
	HWND hWndParent,//父窗口的句柄
	HMENU hMenu,//菜单的句柄或是子窗口的标识符
	HINSTANCE hInstance,//应用程序实例的句柄
	LPVOID lpParam//指向窗口的创建数据
)
{
	const WCHAR* titlename = g_szGameWinTitleW;

	return ((PfuncCreateWindowExW)g_pOldCreateWindowExW)(
		dwExStyle,//窗口的扩展风格
		lpClassName,//指向注册类名的指针
		titlename,//指向窗口名称的指针
		dwStyle,//窗口风格
		x,//窗口的水平位置
		y,//窗口的垂直位置
		nWidth,//窗口的宽度
		nHeight,//窗口的高度
		hWndParent,//父窗口的句柄
		hMenu,//菜单的句柄或是子窗口的标识符
		hInstance,//应用程序实例的句柄
		lpParam//指向窗口的创建数据
		);
}

PVOID g_pOldCreateFileA = CreateFileA;
typedef HANDLE (WINAPI *PfuncCreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
	);


HANDLE WINAPI NewCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	if (!lpFileName)
	{
		return INVALID_HANDLE_VALUE;
	}

	string strOldName(lpFileName);
	string strDirName = strOldName.substr(0, strOldName.find_last_of("\\") + 1);
	string strName = strOldName.substr(strOldName.find_last_of("\\") + 1);
	string strNewName;

	if (strName == "scene.int")
	{
		//MessageBoxA(NULL, "Found", "Asuka", MB_OK);
		strNewName = strDirName + "cnscene.int";
	}
	else if (strName == "fes.int")
	{
		strNewName = strDirName + "cnfes.int";
	}
	else
	{
		strNewName = strOldName;
	}
	return ((PfuncCreateFileA)(g_pOldCreateFileA))(
		strNewName.c_str(),
		dwDesiredAccess,
		dwShareMode,
		lpSecurityAttributes,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		hTemplateFile);
}

#endif


///////////////绘制文字///////////////////////////////////////////////////////////


#ifdef ACR_DRAWTEXT



void WINAPI FT_TextOut(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int strlen)
{
	wstring ws(AnsiToUnicode(lpString, 936), strlen);
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

#if 0 //EntisGLS 夏之雨
wstring fix_note(wstring oldstr) //修正句子中注释结构
{
	//regex是在运行时“编译”的，因此构造效率较低，使用static避免重复构造
	static wstring partten = L"//;(.+?);(.+?):";
	static wregex reg(partten);
	static wstring fmt = L"$1";

	wstring newstr;
	newstr = regex_replace(oldstr, reg, fmt);
	return newstr;
}

//004B21F0  /$  55            push    ebp
//004B21F1  |.  8BEC          mov     ebp, esp
//004B21F3  |.  6A FF         push    -0x1
//004B21F5  |.  68 6D345000   push    0050346D                         ;  SE 处理程序安装
//004B21FA  |.  64:A1 0000000>mov     eax, dword ptr fs:[0]
//004B2200  |.  50            push    eax
//004B2201  |.  64:8925 00000>mov     dword ptr fs:[0], esp
//004B2208  |.  81EC 84010000 sub     esp, 0x184
//004B220E  |.  53            push    ebx
//004B220F  |.  56            push    esi
//004B2210  |.  57            push    edi
//004B2211  |.  8B7D 08       mov     edi, [arg.1]
//004B2214  |.  33DB          xor     ebx, ebx
//004B2216  |.  8BF1          mov     esi, ecx
//004B2218  |.  3BFB          cmp     edi, ebx
//004B221A  |.  89B5 44FFFFFF mov     [local.47], esi

//查找 sub     esp, 0x184


typedef ulong (__stdcall *get_glyph_func_fn)(wchar_t *wstr);
get_glyph_func_fn get_glyph_func = (get_glyph_func_fn)0x4B21F0;

#define CACHE_LEN 512
uchar viewstr[CACHE_LEN];

void __stdcall modify_text(wchar_t *wstr)
{
	__asm pushad //保护现场，非常重要！
	
	if (wstr != NULL)
	{
		ulong oldlen = wstrlen(wstr);

		wstring fixed_oldstr = deleteenter(fix_note(wstr)); //去掉回车
		ulong fixed_oldlen = fixed_oldstr.size() * 2;

		memstr newstr = injector.MatchString((void*)fixed_oldstr.c_str(), fixed_oldlen); //进行匹配

		wstring fixed_newstr = addenter(wstring((wchar_t*)newstr.str, newstr.strlen/2), 24); //添加回车
		
		
		if (newstr.str != NULL) //如果匹配,复制新字符串
		{
			ulong newlen = fixed_newstr.size() * 2;
			memcpy(viewstr, fixed_newstr.c_str(), newlen);
			memset(&viewstr[newlen], 0, 2);
		}
		else //如果不匹配，复制原来的字符串，并log出未匹配的句子
		{
			memcpy(viewstr, wstr, oldlen);
			memset(&viewstr[oldlen], 0, 2);

			logfile.AddLog(wstr);
		}
		
	}
	else
	{
		memset(viewstr, 0, CACHE_LEN); //清空内存，否则会出现大量重复显示
	}
	
	__asm popad //恢复现场
	get_glyph_func((wchar_t*)viewstr);
}


//0047F589   .  8B8C24 280100>mov     ecx, dword ptr[esp + 0x128]
//0047F590   .  33F6          xor     esi, esi
//0047F592   .  51            push    ecx
//0047F593   .  8D4C24 18     lea     ecx, dword ptr[esp + 0x18]
//0047F597   .  C64424 17 00  mov     byte ptr[esp + 0x17], 0x0
//0047F59C   .  E8 2F52FAFF   call    004247D0

void __stdcall prefix(wchar_t *wstr)
{
	if (wstr != NULL)
	{
		wstring oldstr = wstr;
		if (oldstr.find(L"\\r") != oldstr.npos)
		{
			wstring newstr = replace_all(oldstr, L"\\r", L"//");
			wcscpy(wstr, newstr.c_str());
		}
	}	
}

void* g_p_get_text = (void*)0x47F590;
__declspec(naked) void __stdcall get_text()
{
	__asm
	{
		pushad
		push ecx
		call prefix
		popad
		jmp g_p_get_text
	}

}
#endif //EntisGLS 夏之雨

#if 0 //Solfa 祝樱
//00494D20 / $  55            push    ebp;  ebx = string_offset
void* g_p_get_offset = (void*)0x494D20;
ulong real_offset; //字符串在文件中实际地址
__declspec(naked) void __stdcall get_offset()
{
	__asm
	{
		pushad
		mov real_offset, ebx
		popad
		jmp g_p_get_offset
	}

}


#define CACHE_LEN 512
uchar viewstr[CACHE_LEN];

ulong __stdcall get_text()
{
	if (real_offset != 0)
	{
		memstr newstr = engine.MatchStringByOffset(real_offset); //进行匹配

		if (newstr.str != NULL) //如果匹配,复制新字符串
		{
			ulong newlen = newstr.strlen;
			memcpy(viewstr, newstr.str, newlen);
			memset(&viewstr[newlen], 0x1B, 1);
			memset(&viewstr[newlen+1], 0x00, 1);

			return (ulong)(viewstr + newlen); //0x1B的地址
		}
		else //如果不匹配，复制原来的字符串，并log出未匹配的句子
		{
			memset(viewstr, 0, CACHE_LEN); //清空内存
			//logfile.AddLog(wstr);
		}

	}
	else
	{
		memset(viewstr, 0, CACHE_LEN); //清空内存，否则会出现大量重复显示
	}

	return 0;
}


//00494DEF | .E8 BCDDFCFF   call    <switch M2B>;  ebx = *text

void* g_p_modify_offset = (void*)0x494DEF;
ulong str_end;
__declspec(naked) void __stdcall modify_offset()
{
	__asm
	{
		pushad
		call get_text
		test eax, eax
		jz End
		mov str_end, eax
		popad
		lea ebx, dword ptr [viewstr]
		mov ecx, str_end
		jmp g_p_modify_offset
End:
		popad
		jmp g_p_modify_offset
	}

}
#endif //Solfa 祝樱

#if 0//krkrz サノバウィッチ

bool is_alpha_string(wchar_t *wstr, uint len)
{
	if (!wstr)
		return false;

	for (uint i = 0; i < len; i++)
	{
		if (wstr[i] >= 0x100)
			return false;
	}
	return true;
}

bool is_control_string(wstring wstr)
{
	if (wstr.find(L"_") != wstr.npos
		|| wstr.find(L"ラベル") != wstr.npos
		|| wstr.find(L".") != wstr.npos)
		return true;
	
	return false;
}

//0041D1F0 > / .  55            push    ebp;  TVPUtf8ToWideCharString
//0041D1F1 | .    8BEC          mov     ebp, esp
//0041D1F3 | .    8B55 0C       mov     edx, [arg.2]
//0041D1F6 | .    8B4D 08       mov     ecx, [arg.1]
//0041D1F9 | .    E8 824FFEFF   call    00402180
//0041D1FE | .    5D            pop     ebp
//0041D1FF  \.    C2 0800       retn    0x8


typedef int (__stdcall *fnTVPUtf8ToWideCharString)(const char * & in, wchar_t *out);
fnTVPUtf8ToWideCharString TVPUtf8ToWideCharString = (fnTVPUtf8ToWideCharString)0x0041D1F0;

HMODULE g_hdll;

typedef unordered_map<uint, int> UintMap;
UintMap check_map;

int __stdcall newTVPUtf8ToWideCharString(const char * & in, wchar_t *out)
{
	//__asm pushad //保护现场！

	static int caller_addr = 0;
	__asm
	{
		mov eax, [esp+0x54]; //主调函数返回地址，0x5C由OD调试得到
		mov caller_addr, eax;
	}

	int len = TVPUtf8ToWideCharString(in, out);

	if ((caller_addr & 0xFFFF0000) != (uint)g_hdll) //如果主调函数不在psbfile.dll中则返回
		return len;
	if (!in || !out || len == 0)
		return len;
	if (is_alpha_string(out, len))
		return len;

	static wstring pre_wstr;
	wstring wstr(out, len);

	if (is_control_string(wstr))
		return len;
	
	//logfile->AddLog(wstr);
	if (wstr != pre_wstr)
	{
		uint hash = BKDRHash((uchar*)wstr.c_str(), wstr.size() * 2);
		bool is_in_dic = check_map.find(hash) == check_map.end() ? false : true;
		if (wstr[0] == L'「' && pre_wstr[0] != L'「' && !is_in_dic) //说明上一句是人名
			logfile->AddLog(pre_wstr); //写名字

		if (!is_in_dic)
		{
			logfile->AddLog(wstr);
			check_map.insert(UintMap::value_type(hash, 0));
		}
		
	}
	pre_wstr = wstr;
	
	//__asm popad //恢复现场
	return len;

	/*
	if (wstr != NULL)
	{
		ulong oldlen = wstrlen(wstr);

		wstring fixed_oldstr = deleteenter(fix_note(wstr)); //去掉回车
		ulong fixed_oldlen = fixed_oldstr.size() * 2;

		memstr newstr = injector.MatchString((void*)fixed_oldstr.c_str(), fixed_oldlen); //进行匹配

		wstring fixed_newstr = addenter(wstring((wchar_t*)newstr.str, newstr.strlen / 2), 24); //添加回车


		if (newstr.str != NULL) //如果匹配,复制新字符串
		{
			ulong newlen = fixed_newstr.size() * 2;
			memcpy(viewstr, fixed_newstr.c_str(), newlen);
			memset(&viewstr[newlen], 0, 2);
		}
		else //如果不匹配，复制原来的字符串，并log出未匹配的句子
		{
			memcpy(viewstr, wstr, oldlen);
			memset(&viewstr[oldlen], 0, 2);

			logfile.AddLog(wstr);
		}

	}
	else
	{
		memset(viewstr, 0, CACHE_LEN); //清空内存，否则会出现大量重复显示
	}
	*/
	
}


typedef HMODULE(WINAPI *fnLoadLibraryW)(LPCWSTR lpLibFileName);
fnLoadLibraryW pLoadLibraryW = LoadLibraryW;
HMODULE WINAPI newLoadLibraryW(LPCWSTR lpLibFileName)
{
	static bool isRecord = false;

	if (lpLibFileName)
	{
		wstring dllname = lpLibFileName;
		if (dllname.find(L"psbfile.dll") != dllname.npos && !isRecord)
		{
			isRecord = true;
			g_hdll = pLoadLibraryW(lpLibFileName);
			return g_hdll;
		}
	}

	return pLoadLibraryW(lpLibFileName);
}


#endif //krkrz サノバウィッチ

#if 1 //Cat2System

bool ReadPngFile(const std::string& strName, int nOffset, PNG_DATA* pPngData)
{
	if (!pPngData || strName.empty())
	{
		return false;
	}

	FILE* fPng = fopen(strName.c_str(), "rb");
	if (!fPng)
	{
		return false;
	}

	if (nOffset)
	{
		fseek(fPng, nOffset, SEEK_SET);
	}

	if (!PngFile::ReadPngFile(fPng, pPngData))
	{
		fclose(fPng);
		if (pPngData->pRgba)
		{
			free(pPngData->pRgba);
		}

		MessageBoxA(NULL, strName.c_str(), "PNG Error", MB_OK);
		return false;
	}

	fclose(fPng);
	return true;
}

#define IMAGE_CRC_INIT (0L)
#define CN_IMAGE_PKG_NAME "cnimage.int"
void ReplaceImage(byte* pBuffer, uint nWidth, uint nHeight)
{
	if (!pBuffer)
	{
		return;
	}

	uint nImageSize = nWidth * nHeight * 4;
	uint nCrc = crc32(IMAGE_CRC_INIT, pBuffer, nImageSize);
	char szImageName[MAX_PATH] = { 0 };
	sprintf(szImageName, "%08X.png", nCrc);
	std::string strName(szImageName);

	PNG_DATA oPngData = { 0 };

	std::string strSingleFile = "cnimage\\";
	strSingleFile += strName;
	if (ReadPngFile(strSingleFile, 0, &oPngData))
	{
		memcpy(pBuffer, oPngData.pRgba, nImageSize);
		free(oPngData.pRgba);
		return;
	}

	info_t oImageInfo = { 0 };
	if (!g_oImageMap.GetInfo(strName, &oImageInfo))
	{
		return;
	}

	uint nOffset = oImageInfo.offset;
	if (ReadPngFile(CN_IMAGE_PKG_NAME, nOffset, &oPngData))
	{
		memcpy(pBuffer, oPngData.pRgba, nImageSize);
		free(oPngData.pRgba);
		return;
	}
}

void WriteImage(byte* pBuffer, uint nWidth, uint nHeight)
{
	uint nImageSize = nWidth * nHeight * 4;

	PNG_DATA pic = { 0 };
	pic.eFlag = HAVE_ALPHA;
	pic.nWidth = nWidth;
	pic.nHeight = nHeight;
	pic.pRgba = pBuffer;

	uint nCrc = crc32(IMAGE_CRC_INIT, pBuffer, nImageSize);
	char szImageName[MAX_PATH] = { 0 };
	sprintf(szImageName, "%08X.png", nCrc);
	std::string strName(szImageName);
	PngFile::WritePngFile(strName.c_str(), &pic);
}



void __stdcall ProcessImage(byte* pBuffer, uint nWidth, uint nHeight)
{
	ReplaceImage(pBuffer, nWidth, nHeight);
	//WriteImage(pBuffer, nWidth, nHeight);
}

//
//00548ECF | .  3942 04 | cmp     dword ptr[edx + 0x4], eax
//00548ED2 | .  7E 37 | jle     short 00548F0B
//00548ED4 | > 8B5424 5C | / mov     edx, dword ptr[esp + 0x5C];  edx = stride
//00548ED8 | .  85D2 || test    edx, edx
//00548EDA | .  74 1D || je      short 00548EF9
//00548EDC | .  8D6424 00 || lea     esp, dword ptr[esp]
//00548EE0 | > 0FB603 || / movzx   eax, byte ptr[ebx]
//00548EE3 | .  8D49 01 || | lea     ecx, dword ptr[ecx + 0x1]
//00548EE6 | .  43 || | inc     ebx
//00548EE7 | .  8A4430 78 || | mov     al, byte ptr[eax + esi + 0x78]
//00548EEB | .  02440F FF || | add     al, byte ptr[edi + ecx - 0x1]
//00548EEF | .  8841 FF || | mov     byte ptr[ecx - 0x1], al
//00548EF2 | .  4A || | dec     edx
//00548EF3 | . ^ 75 EB || \jnz     short 00548EE0
//00548EF5 | .  8B4424 18 || mov     eax, dword ptr[esp + 0x18]
//00548EF9 | > 8B5424 24 || mov     edx, dword ptr[esp + 0x24]
//00548EFD | .  40 || inc     eax
//00548EFE | .  2BEF || sub     ebp, edi
//00548F00 | .  894424 18 || mov     dword ptr[esp + 0x18], eax
//00548F04 | .  8BCD || mov     ecx, ebp
//00548F06 | .  3B42 04 || cmp     eax, dword ptr[edx + 0x4]
//00548F09 | . ^ 7C C9 | \jl      short 00548ED4
//00548F0B | > 8B4424 54 | mov     eax, dword ptr[esp + 0x54]
//00548F0F | .  8B5C24 60 | mov     ebx, dword ptr[esp + 0x60]
//00548F13 | .  40 | inc     eax
//

void* g_p_copy_image = (void*)0x00548F0B;
void __declspec(naked) copy_image()
{
	__asm
	{
		pushad
		pushfd

		mov eax, dword ptr[esp + 0x48]
		mov	ebx, dword ptr[eax + 0x4]  //ebx = height
		mov edx, dword ptr[esp + 0x80]
		add ecx, edx  //ecx = buffer
		shr edx, 2  //edx = width
		push ebx
		push edx
		push ecx
		call ProcessImage

		popfd
		popad
		jmp g_p_copy_image
	}
}


#endif //Cat2System

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
	DetourAttach(&g_pOldCreateFontIndirectA, NewCreateFontIndirectA);
	DetourAttach(&g_pOldCreateFontIndirectW, NewCreateFontIndirectW);
	DetourAttach(&g_pOldCreateFontA, NewCreateFontA);
	//DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);
	//DetourAttach(&g_pOldWideCharToMultiByte, NewWideCharToMultiByte);
	DetourTransactionCommit();
#endif


#ifdef ACR_TRANSLATE
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&g_p_copy_image, copy_image);

	DetourTransactionCommit();
#endif


#ifdef ACR_EXTRA
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	
	//DetourAttach(&g_pOldSetWindowTextA, NewSetWindowTextA);
	//DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);
	//DetourAttach(&g_pOldCreateWindowExW, NewCreateWindowExW);

	DetourAttach(&g_pOldCreateFileA, NewCreateFileA);

	DetourTransactionCommit();
	
#endif
}

void InitProc()
{
#ifdef ACR_DRAWTEXT
	TextColor color(251, 238, 245, 255);
	//TextColor effcl(0, 0, 0, 255);

	gdrawer.InitDrawer("simhei.ttf",80);
	gdrawer.SetTextColor(color);
	//gdrawer.ApplyEffect(Shadow, effcl, 2, 2.0);

	SetNopCode((BYTE*)g_p_textout_white, 6);
	SetNopCode((BYTE*)g_p_textout_black, 6);
#endif

#ifdef ACR_TRANSLATE

	//parser = new AcrParser("shukufuku.acr");
	//parser = new TextParser("Shukufuku.txt");
	//translator = new Translator(*parser);
	//engine.Init(*translator);
	//logfile = new LogFile("stringlog.txt", OPEN_ALWAYS);

	g_oImageMap.FillMap(CN_IMAGE_PKG_NAME);
#endif
	SetHook();
}

void UnInst()
{
#ifdef ACR_TRANSLATE
	//delete parser;
	//delete translator;
	delete logfile;
#endif
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
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		UnInst();
		break;
	}
	return TRUE;
}


