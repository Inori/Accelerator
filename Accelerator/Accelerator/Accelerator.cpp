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
//#define ACR_EXTRA

////////////全局变量////////////////////////////////////////////////////////

#ifdef ACR_DRAWTEXT

GdipDrawer gdrawer;
#endif

#ifdef ACR_TRANSLATE

ScriptParser *parser;
Translator *translator;
TranslateEngine *engine;
LogFile *logfile;

#endif





////////////中文字符集////////////////////////////////////////////////////////
#ifdef ACR_GBKFONT

PVOID g_pOldCreateFontIndirectA = CreateFontIndirectA;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	//lplf->lfCharSet = ANSI_CHARSET;
	lplf->lfCharSet = GB2312_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;

	//修改后的字体，包括音符等特殊符号
	//strcpy(lplf->lfFaceName, "黑体");

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

PVOID g_pOldCreateFontIndirectW = CreateFontIndirectW;
typedef int (WINAPI *PfuncCreateFontIndirectW)(LOGFONTW *lplf);
int WINAPI NewCreateFontIndirectW(LOGFONTW *lplf)
{
	//lplf->lfCharSet = ANSI_CHARSET;
	lplf->lfCharSet = GB2312_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;

	//修改后的字体，包括音符等特殊符号
	//wcscpy(lplf->lfFaceName, L"黑体");

	return ((PfuncCreateFontIndirectW)g_pOldCreateFontIndirectW)(lplf);
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

#endif

///////////////修改少量系统文字///////////////////////////////////////////////////
#ifdef ACR_EXTRA
PVOID g_pOldSetWindowTextA = NULL;
typedef int (WINAPI *PfuncSetWindowTextA)(HWND hwnd, LPCTSTR lpString);
int WINAPI NewSetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if (!memcmp(lpString, "壞僲塉", 6))
	{
		strcpy((char*)(LPCTSTR)lpString, "架向星空之桥AA");
	}
	return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
}

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND (WINAPI *PfuncCreateWindowExA)(
	DWORD dwExStyle,//窗口的扩展风格
	LPCTSTR lpClassName,//指向注册类名的指针
	LPCTSTR lpWindowName,//指向窗口名称的指针
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
	LPCTSTR lpClassName,//指向注册类名的指针
	LPCTSTR lpWindowName,//指向窗口名称的指针
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
	const char* titlename = "夏之雨";

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

#if 1//krkrz サノバウィッチ

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
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&g_pOldCreateFontIndirectW, NewCreateFontIndirectW);
	DetourTransactionCommit();
	/*
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&g_pOldMultiByteToWideChar, NewMultiByteToWideChar);*/
	DetourTransactionCommit();
#endif


#ifdef ACR_TRANSLATE
	DetourTransactionBegin();
	DetourAttach((void**)&TVPUtf8ToWideCharString, newTVPUtf8ToWideCharString);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&pLoadLibraryW, newLoadLibraryW);
	DetourTransactionCommit();
#endif


#ifdef ACR_EXTRA
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	g_pOldSetWindowTextA = DetourFindFunction("USER32.dll", "SetWindowTextA");
	DetourAttach(&g_pOldSetWindowTextA, NewSetWindowTextA);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&g_pOldCreateWindowExA, NewCreateWindowExA);
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
	logfile = new LogFile("stringlog.txt", OPEN_ALWAYS);

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


