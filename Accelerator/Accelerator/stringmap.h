/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: �����ַ���������
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

//�����ڴ��ַ������Լ��ݶ��ֽںͿ��ַ��ַ���
typedef struct _memstr
{
	uint strlen; //�ַ����ֽڳ���
	uchar *str;    //�ַ�����ַ
}memstr;

typedef struct _HashString
{
	uint hash;
	memstr old_str; //Ϊ�Ժ���ͬʱ��ʾ����������
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


//ȡ������
#pragma pack(1)

//�ű�����ļ�ͷ
typedef struct _acr_header
{
	ulong index_count; //���������������ַ�����
	ulong compress_flag; //�Ƿ�ѹ���� 0û��ѹ��
	ulong compsize; //�����ѹ����Ϊѹ�����С���������orgsize
	ulong orgsize; //�����ѹ����Ϊѹ��ǰ��ѹ�����֡���С������Ϊʵ�ʴ�С
}acr_header;

typedef struct _acr_index
{
	ulong hash; //oldstr��hashֵ������map����
	ulong old_str_off; //oldstr ��ַ
	ulong old_str_len; //oldstr �ֽڳ���
	ulong new_str_off; //newstr ��ַ
	ulong new_str_len; //newstr ����
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
	BYTE *data; //��Ϸ�˳�ǰ�����ͷţ�
	BYTE *real_data; //ȥ���ļ�ͷ��Ĳ���,�����������ַ�������.��Ϸ�˳�ǰ�����ͷţ�

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