#include "stringmap.h"
#include "tools.h"
#include "zlib.h"

inline uint BKDRHash(const uchar *str, const size_t len)
{
	register size_t hash = 0; //使用size_t而不是uint避免累乘时越界
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

Translator::Translator(vector<HashString> hashstrlist)
{
	InsertString(hashstrlist);
}


void Translator::InsertString(vector<HashString> hashstrlist)
{
	//扩充桶
	strmap.reserve(hashstrlist.size());

	for each(auto hashstr in hashstrlist)
	{
		strmap.insert(StringMap::value_type(hashstr.hash, hashstr.new_str));
	}
}



memstr Translator::searchstr(uint hash)
{
	memstr mstr;
	mstr.strlen = 0;
	mstr.str = NULL;

	//以hash值查找对应字符串
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
	//首先计算传入字符串的hash
	uint hash = BKDRHash(str.str, str.strlen);
	return searchstr(hash);
}

Translator::~Translator()
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

	//写utf16 BOM
	ulong writtenlen = 0;
	char *UTF16BOM = "\xFF\xFE";
	WriteFile(hfile, UTF16BOM, 2, &writtenlen, NULL);

	return true;
}

void LogFile::AddLog(wstring logstr)
{
	wstring line = logstr + L"\r\n";
	ulong writtenlen = 0;
	WriteFile(hfile, line.c_str(), 2*line.size(), &writtenlen, NULL);
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

//////////////////////////////////////////////////////////////////////////////////////////////
ScriptParser::ScriptParser()
{
	is_compressed = false;
}

ScriptParser::ScriptParser(string fname)
{
	is_compressed = false;
	Init(fname);
}



bool ScriptParser::Init(string fname)
{
	hfile = CreateFileA(fname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "Can not open script file!", "Error", MB_OK);
		return false;
	}
	
	DWORD fsize = GetFileSize(hfile, NULL);
	data = new BYTE[fsize];
	DWORD readlen = 0;
	ReadFile(hfile, data, fsize, &readlen, NULL);

	return true;
}

acr_index *ScriptParser::Parse()
{
	acr_header *header = (acr_header*)data;
	is_compressed = header->compress_flag;
	
	BYTE *comp_data = data + sizeof(acr_header);

	if (is_compressed)
	{
		DWORD reallen = header->orgsize;
		DWORD complen = header->compsize;
		real_data = new BYTE[reallen];
		if (uncompress(real_data, &reallen, comp_data, complen) != Z_OK)
		{
			MessageBoxA(NULL, "zlib uncompress failed!", "Error", MB_OK);
			return NULL;
		}

	}
	else
	{
		real_data = comp_data;
	}
		

	DWORD index_count = header->index_count;
	str_count = index_count;
	index_list = new acr_index[index_count];
	acr_index *real_index = (acr_index *)real_data;

	//计算字符串在内存中的实际地址
	for (DWORD i = 0; i < index_count; i++)
	{
		index_list[i].hash = real_index->hash;
		index_list[i].old_str_off = (real_index->old_str_off + (DWORD)data);
		index_list[i].old_str_len = real_index->old_str_len;
		index_list[i].new_str_off = (real_index->new_str_off + (DWORD)data);
		index_list[i].new_str_len = real_index->new_str_len;
		real_index++;
	}
	return index_list;
}

DWORD ScriptParser::GetStrCount()
{
	return str_count;
}

ScriptParser::~ScriptParser()
{
	if (is_compressed)
		delete[] real_data;

	delete[]data;
	delete[]index_list;
	CloseHandle(hfile);
}

//////////////////////////////////////////////////////////////////////////////////////////////
StringInjector::StringInjector()
{
}

StringInjector::StringInjector(string fname)
{
	Init(fname);
}

bool StringInjector::Init(string fname)
{
	if (!parser.Init(fname))
	{
		MessageBoxA(NULL, "parser init failed!", "Error", MB_OK);
		return false;
	}

	inittranslator();
}

vector<HashString> StringInjector::gethashstrlist()
{
	vector<HashString> list;
	acr_index *index = parser.Parse();
	ulong str_count = parser.GetStrCount();

	list.reserve(str_count * sizeof(HashString)); //尽量减少内存复制，提高效率

	HashString cur;
	for (uint i = 0; i < str_count; i++)
	{
		cur.hash = index[i].hash;
		cur.new_str.str = (uchar*)index[i].new_str_off;
		cur.new_str.strlen = index[i].new_str_len;

		list.push_back(cur);
	}

	return list;
}

void StringInjector::inittranslator()
{
	//获取字符表
	vector<HashString> list = gethashstrlist();
	translator.InsertString(list);
}


bool StringInjector::Inject(void *dst, ulong dstlen)
{
	bool matched = false;
	//从原来位置的字符串构造memstr
	memstr oldstr;
	oldstr.str = (uchar*)dst;
	oldstr.strlen = dstlen;

	//翻译
	memstr newstr = translator.Translate(oldstr);

	//如果匹配，则新字符串拷贝到原位置，否则不操作
	static uchar zero[2] = {0};
	
	if (newstr.str != NULL)
	{
		memcopy(dst, newstr.str, newstr.strlen);
		memcopy((void*)((uchar*)dst + newstr.strlen), zero, 2); // 补 "\0"
		matched = true;
	}

	return matched;
}

memstr StringInjector::MatchString(void *dst, ulong dstlen)
{
	//从原来位置的字符串构造memstr
	memstr oldstr;
	oldstr.str = (uchar*)dst;
	oldstr.strlen = dstlen;

	//翻译
	memstr newstr = translator.Translate(oldstr);

	return newstr;
}

StringInjector::~StringInjector()
{
	
}