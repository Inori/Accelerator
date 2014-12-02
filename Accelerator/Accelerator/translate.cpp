#include <Windows.h>

#include "translate.h"
#include "scriptparser.h"
#include "tools.h"



inline uint BKDRHash(const uchar *str, const size_t len)
{
	register size_t hash = 0; //ʹ��size_t������uint�����۳�ʱԽ��
	const size_t seed = 131; // 31 131 1313 13131 131313 etc..

	if (str == NULL) return 0;
	for (size_t i = 0; i < len; i++)
	{
		if (str[i] != 0)
			hash = hash * seed + str[i];
	}

	return (hash & 0x7FFFFFFF);
}

//////////////////////////////////////////////////////////////////////////////////////////////
Translator::Translator()
{
}

Translator::Translator(ScriptParser& parser)
{
	acr_index *index = parser.Parse();
	ulong str_count = parser.GetStrCount();

	insertString(index, str_count);
}

void Translator::insertString(acr_index *index, ulong index_count)
{
	//����Ͱ
	strmap.reserve(index_count);

	HashString cur;
	for (uint i = 0; i < index_count; i++)
	{
		cur.hash = index[i].hash;
		cur.new_str.str = (uchar*)index[i].new_str_off;
		cur.new_str.strlen = index[i].new_str_len;

		strmap.insert(StringMap::value_type(cur.hash, cur.new_str));
	}
}

memstr Translator::SearchStr(uint hash)
{
	memstr mstr;
	mstr.strlen = 0;
	mstr.str = NULL;

	//��hashֵ���Ҷ�Ӧ�ַ���
	StringMap::iterator iter;
	iter = strmap.find(hash);
	if (iter != strmap.end())
	{
		mstr = iter->second;
	}

	return mstr;
}

memstr Translator::Translate(memstr str)
{
	//���ȼ��㴫���ַ�����hash
	uint hash = BKDRHash(str.str, str.strlen);
	return SearchStr(hash);
}

Translator::~Translator()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
TranslateEngine::TranslateEngine()
{
}

TranslateEngine::TranslateEngine(Translator& translator)
{
	Init(translator);
}

bool TranslateEngine::Init(Translator& translator)
{
	this->translator = &translator;
	return true;
}


bool TranslateEngine::Inject(void *dst, ulong dstlen)
{
	bool matched = false;
	//��ԭ��λ�õ��ַ�������memstr
	memstr oldstr;
	oldstr.str = (uchar*)dst;
	oldstr.strlen = dstlen;

	//����
	memstr newstr = translator->Translate(oldstr);

	//���ƥ�䣬�����ַ���������ԭλ�ã����򲻲���
	static uchar zero[2] = {0};
	
	if (newstr.str != NULL)
	{
		memcopy(dst, newstr.str, newstr.strlen);
		memcopy((void*)((uchar*)dst + newstr.strlen), zero, 2); // �� "\0"
		matched = true;
	}

	return matched;
}

memstr TranslateEngine::MatchString(void *dst, ulong dstlen)
{
	//��ԭ��λ�õ��ַ�������memstr
	memstr oldstr;
	oldstr.str = (uchar*)dst;
	oldstr.strlen = dstlen;

	//����
	memstr newstr = translator->Translate(oldstr);

	return newstr;
}

memstr TranslateEngine::MatchStringByOffset(ulong offset)
{
	//����
	memstr newstr = translator->SearchStr(offset);
	return newstr;
}

TranslateEngine::~TranslateEngine()
{
	
}


//////////////////////////////////////////////////////////////////////////////////////////////


LogFile::LogFile()
{
}

LogFile::LogFile(string filename, uint open_mode)
{
	if (!Init(filename, open_mode))
		MessageBoxA(NULL, "Can not create log file!", "Error", MB_OK);
}

bool LogFile::Init(string filename, uint open_mode)
{
	hfile = CreateFileA(filename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, open_mode, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "create log file error!", "Error", MB_OK);
		return false;
	}

	//дutf16 BOM
	ulong writtenlen = 0;
	char *UTF16BOM = "\xFF\xFE";
	WriteFile(hfile, UTF16BOM, 2, &writtenlen, NULL);

	return true;
}

void LogFile::AddLog(wstring logstr)
{
	wstring line = logstr + L"\r\n";
	ulong writtenlen = 0;
	WriteFile(hfile, line.c_str(), 2 * line.size(), &writtenlen, NULL);
}

void LogFile::AddLog(string logstr, uint code_page)
{
	string line = logstr + "\r\n";
	wstring wline = AnsiToUnicode(line.c_str(), code_page);
	ulong writtenlen = 0;
	WriteFile(hfile, wline.c_str(), 2 * wline.size(), &writtenlen, NULL);
}


LogFile::~LogFile()
{
	CloseHandle(hfile);
}