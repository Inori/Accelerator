#include "stringmap.h"
#include "tools.h"
#include "zlib.h"

uint BKDRHash(const uchar *str, const uint len)
{
	register uint hash = 0;
	const uint seed = 131; // 31 131 1313 13131 131313 etc..

	if (str == NULL) return 0;
	for (uint i = 0; i < len; i++)
	{
		if (str[i] != 0)
			hash = hash * seed + (*str++);
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

LogFile::LogFile(string filename)
{
	if (!Init(filename))
		MessageBoxA(NULL, "Can not create log file!", "Error", MB_OK);
}

bool LogFile::Init(string filename)
{
	hfile = fopen(filename.c_str(), "rb+");
	if (!hfile)
		return false;
	return true;
}

void LogFile::AddLog(string logstr)
{
	fprintf(hfile, "%s\r\n", logstr.c_str());
}

LogFile::~LogFile()
{
	fclose(hfile);
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
	}
	return index_list;
}

DWORD ScriptParser::GetStrCount()
{
	return str_count;
}

void ScriptParser::Over()
{
	if (is_compressed)
		delete[] real_data;

	delete[]data;
	delete[]index_list;
}

ScriptParser::~ScriptParser()
{
	Over();
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
	if (!log.Init("log.txt"))
	{
		MessageBoxA(NULL, "logfile init failed!", "Error", MB_OK);
		return false;
	}
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


void StringInjector::Inject(void *dst, ulong dstlen)
{
	//从原来位置的字符串构造memstr
	memstr oldstr;
	oldstr.str = (uchar*)dst;
	oldstr.strlen = dstlen;

	//翻译
	memstr newstr = translator.Translate(oldstr);

	//如果匹配，则新字符串拷贝到原位置，否则不操作
	if (newstr.str != NULL)
		memcopy(dst, newstr.str, newstr.strlen);
}


StringInjector::~StringInjector()
{
	//parser.Over();
}