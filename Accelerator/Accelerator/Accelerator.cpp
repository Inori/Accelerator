#include "Accelerator.h"


///////////////////�������ã��������屾�α�����Ҫ��ɵĹ���///////////////////////

//�����ַ���
//#define ACR_GBKFONT

//��������
//#define ACR_DRAWTEXT

//��̬����
#define ACR_TRANSLATE


//�����������ڵ��������ܣ�
//1���޸Ĵ��ڱ���
#define ACR_EXTRA

////////////ȫ�ֱ���////////////////////////////////////////////////////////

#ifdef ACR_DRAWTEXT

GdipDrawer gdrawer;
#endif

#ifdef ACR_TRANSLATE

StringInjector injector;
LogFile logfile;

#endif





////////////�����ַ���////////////////////////////////////////////////////////
#ifdef ACR_GBKFONT

PVOID g_pOldCreateFontIndirectA = NULL;
typedef int (WINAPI *PfuncCreateFontIndirectA)(LOGFONTA *lplf);
int WINAPI NewCreateFontIndirectA(LOGFONTA *lplf)
{
	lplf->lfCharSet = ANSI_CHARSET;
	//lplf->lfCharSet = GB2312_CHARSET;

	//�޸ĺ�����壬�����������������
	strcpy(lplf->lfFaceName, "����");

	return ((PfuncCreateFontIndirectA)g_pOldCreateFontIndirectA)(lplf);
}

#endif

///////////////�޸�����ϵͳ����///////////////////////////////////////////////////
#ifdef ACR_EXTRA
PVOID g_pOldSetWindowTextA = NULL;
typedef int (WINAPI *PfuncSetWindowTextA)(HWND hwnd, LPCTSTR lpString);
int WINAPI NewSetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if (!memcmp(lpString, "�ăm�J", 6))
	{
		strcpy((char*)(LPCTSTR)lpString, "�����ǿ�֮��AA");
	}
	return ((PfuncSetWindowTextA)g_pOldSetWindowTextA)(hwnd, lpString);
}

PVOID g_pOldCreateWindowExA = CreateWindowExA;
typedef HWND (WINAPI *PfuncCreateWindowExA)(
	DWORD dwExStyle,//���ڵ���չ���
	LPCTSTR lpClassName,//ָ��ע��������ָ��
	LPCTSTR lpWindowName,//ָ�򴰿����Ƶ�ָ��
	DWORD dwStyle,//���ڷ��
	int x,//���ڵ�ˮƽλ��
	int y,//���ڵĴ�ֱλ��
	int nWidth,//���ڵĿ��
	int nHeight,//���ڵĸ߶�
	HWND hWndParent,//�����ڵľ��
	HMENU hMenu,//�˵��ľ�������Ӵ��ڵı�ʶ��
	HINSTANCE hInstance,//Ӧ�ó���ʵ���ľ��
	LPVOID lpParam//ָ�򴰿ڵĴ�������
	);

HWND WINAPI NewCreateWindowExA(
	DWORD dwExStyle,//���ڵ���չ���
	LPCTSTR lpClassName,//ָ��ע��������ָ��
	LPCTSTR lpWindowName,//ָ�򴰿����Ƶ�ָ��
	DWORD dwStyle,//���ڷ��
	int x,//���ڵ�ˮƽλ��
	int y,//���ڵĴ�ֱλ��
	int nWidth,//���ڵĿ��
	int nHeight,//���ڵĸ߶�
	HWND hWndParent,//�����ڵľ��
	HMENU hMenu,//�˵��ľ�������Ӵ��ڵı�ʶ��
	HINSTANCE hInstance,//Ӧ�ó���ʵ���ľ��
	LPVOID lpParam//ָ�򴰿ڵĴ�������
	)
{
	const char* titlename = "��֮��";

	return ((PfuncCreateWindowExA)g_pOldCreateWindowExA)(
		dwExStyle,//���ڵ���չ���
		lpClassName,//ָ��ע��������ָ��
		titlename,//ָ�򴰿����Ƶ�ָ��
		dwStyle,//���ڷ��
		x,//���ڵ�ˮƽλ��
		y,//���ڵĴ�ֱλ��
		nWidth,//���ڵĿ��
		nHeight,//���ڵĸ߶�
		hWndParent,//�����ڵľ��
		hMenu,//�˵��ľ�������Ӵ��ڵı�ʶ��
		hInstance,//Ӧ�ó���ʵ���ľ��
		lpParam//ָ�򴰿ڵĴ�������
		);
}
#endif


///////////////��������///////////////////////////////////////////////////////////


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

///////////////�滻�ַ���/////////////////////////////////////////////////
#ifdef ACR_TRANSLATE


wstring fix_note(wstring oldstr) //����������ע�ͽṹ
{
	//regex��������ʱ�����롱�ģ���˹���Ч�ʽϵͣ�ʹ��static�����ظ�����
	static wstring partten = L"//;(.+?);(.+?):";
	static wregex reg(partten);
	static wstring fmt = L"$1";

	wstring newstr;
	newstr = regex_replace(oldstr, reg, fmt);
	return newstr;
}

//00480256 | .  53            push    ebx
//00480257 | .  8BCF          mov     ecx, edi
//00480259 | .  897C24 14     mov     dword ptr[esp + 0x14], edi
//0048025D      FF52 10       call    dword ptr[edx + 0x10];  get_glyph. dword ptr[edx + 0x10] = 004B21F0

//004B21F0   $  55            push    ebp

typedef ulong (__stdcall *get_glyph_func_fn)(wchar_t *wstr);
get_glyph_func_fn get_glyph_func = (get_glyph_func_fn)0x4B21F0;

#define CACHE_LEN 512
uchar viewstr[CACHE_LEN];

void __stdcall modify_text(wchar_t *wstr)
{
	__asm pushad //�����ֳ����ǳ���Ҫ��
	
	if (wstr != NULL)
	{
		ulong oldlen = wstrlen(wstr);

		wstring fixed_oldstr = deleteenter(fix_note(wstr)); //ȥ���س�
		ulong fixed_oldlen = fixed_oldstr.size() * 2;

		memstr newstr = injector.MatchString((void*)fixed_oldstr.c_str(), fixed_oldlen); //����ƥ��

		wstring fixed_newstr = addenter(wstring((wchar_t*)newstr.str, newstr.strlen/2), 24); //��ӻس�
		
		
		if (newstr.str != NULL) //���ƥ��,�������ַ���
		{
			ulong newlen = fixed_newstr.size() * 2;
			memcpy(viewstr, fixed_newstr.c_str(), newlen);
			memset(&viewstr[newlen], 0, 2);
		}
		else //�����ƥ�䣬����ԭ�����ַ�������log��δƥ��ľ���
		{
			memcpy(viewstr, wstr, oldlen);
			memset(&viewstr[oldlen], 0, 2);

			logfile.AddLog(wstr);
		}
		
	}
	else
	{
		memset(viewstr, 0, CACHE_LEN); //����ڴ棬�������ִ����ظ���ʾ
	}
	
	__asm popad //�ָ��ֳ�
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



#endif


//////////////////////////////////////////////////////////////////////////
//��װHook 
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
	DetourAttach((void**)&get_glyph_func, modify_text);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&g_p_get_text, get_text);
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

	gdrawer.InitDrawer("simhei.ttf",89);
	gdrawer.SetTextColor(color);
	//gdrawer.ApplyEffect(Shadow, effcl, 2, 2.0);

	SetNopCode((BYTE*)g_p_textout_white, 6);
	SetNopCode((BYTE*)g_p_textout_black, 6);
#endif

#ifdef ACR_TRANSLATE

	injector.Init("Platonic16.acr");
	logfile.Init("stringlog.txt", OPEN_ALWAYS);


#endif
	SetHook();
}

//��Ҫһ����������
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


