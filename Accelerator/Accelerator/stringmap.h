/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: 定义字符串处理类
\******************************************************************************/

#ifndef STRINGMAP_H
#define STRINGMAP_H

#include <Windows.h>
#include <vector>
#include <string>
#include <unordered_map>

#include "types.h"
using namespace std;
///////////////////////////////////////////////////////////////////////////////



//定义内存字符串，以兼容多字节和宽字符字符串
typedef struct _memstr
{
	uint strlen; //字符串字节长度
	uchar *str;    //字符串地址
}memstr;

typedef struct _HashString
{
	uint hash;
	memstr old_str; //为以后能同时显示日文做兼容
	memstr new_str;
}HashString;

//BKDR hash function
inline uint BKDRHash(const uchar *str, const uint len);

typedef unordered_map<uint, memstr> StringMap;

class Translator
{
public:
	Translator();
	Translator(vector<HashString> hashstrlist);

	memstr SearchStr(uint hash);

	void InsertString(vector<HashString> hashstrlist);
	memstr Translate(memstr str);

	~Translator();
private:
	
	StringMap strmap;
};

///////////////////////////////////////////////////////////////////////////////

class LogFile
{
public:
	LogFile();
	LogFile(string filename, uint open_mode);

	bool Init(string filename, uint open_mode);
	void AddLog(wstring logstr);
	void AddLog(string logstr, uint code_page);

	~LogFile();

private:
	HANDLE hfile;
};

///////////////////////////////////////////////////////////////////////////////


//取消对齐
#pragma pack(1)

//脚本封包文件头
typedef struct _acr_header
{
	ulong index_count; //包含索引数，即字符串数
	ulong compress_flag; //是否压缩。 0没有压缩
	ulong compsize; //如果有压缩，为压缩后大小，否则等于orgsize
	ulong orgsize; //如果有压缩，为压缩前【压缩部分】大小，否则为实际大小
}acr_header;

typedef struct _acr_index
{
	ulong hash; //oldstr的hash值，用于map查找
	ulong old_str_off; //oldstr 地址
	ulong old_str_len; //oldstr 字节长度
	ulong new_str_off; //newstr 地址
	ulong new_str_len; //newstr 长度
}acr_index;

#pragma pack()


class ScriptParser
{
public:
	ScriptParser();
	ScriptParser(string fname);

	bool Init(string fname);
	acr_index *Parse();
	DWORD GetStrCount();

	~ScriptParser();
private:
	HANDLE hfile;
	acr_index *index_list;
	BYTE *data; //游戏退出前不可释放！
	BYTE *real_data; //去除文件头后的部分,包括索引和字符串数据.游戏退出前不可释放！

	DWORD str_count;
	bool is_compressed;
};




///////////////////////////////////////////////////////////////////////////////


class StringInjector
{
public:
	StringInjector();
	StringInjector(string fname);

	bool Init(string fname);
	
	bool Inject(void *dst, ulong dstlen);  //直接向目标地址注入匹配的字符串，注入成功返回真，否则返回假
	memstr MatchString(void *dst, ulong dstlen);  //不进行注入，直接返回匹配的字符串供手动编写代码处理。
	memstr MatchStringWithOffset(ulong offset);  //不进行注入，根据地址直接返回匹配的字符串供手动编写代码处理。

	~StringInjector();

private:
	vector<HashString> gethashstrlist();
	void inittranslator();

	Translator translator;
	ScriptParser parser;
};








#endif