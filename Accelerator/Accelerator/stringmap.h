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

using namespace std;
///////////////////////////////////////////////////////////////////////////////

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

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
uint BKDRHash(const uchar *str, const uint len);

typedef unordered_map<uint, memstr> StringMap;

class Translator
{
public:
	Translator();
	Translator(vector<HashString> hashstrlist);

	void InsertString(vector<HashString> hashstrlist);
	memstr Translate(memstr str);

	~Translator();
private:
	memstr searchstr(uint hash);
	
	StringMap strmap;
};

///////////////////////////////////////////////////////////////////////////////

class LogFile
{
public:
	LogFile();
	LogFile(string filename);

	bool Init(string filename);
	void AddLog(string logstr);

	~LogFile();

private:
	FILE *hfile;
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

//const char* logfilename = "log.txt";

class StringInjector
{
public:
	StringInjector();
	StringInjector(string fname);

	bool Init(string fname);
	void Inject(void *dst, ulong dstlen);

	~StringInjector();

private:
	vector<HashString> gethashstrlist();
	void inittranslator();

	Translator translator;
	ScriptParser parser;
	//LogFile log;
};








#endif