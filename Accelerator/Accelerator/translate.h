/******************************************************************************\
*  Copyright (C) 2014 Fuyin
*  ALL RIGHTS RESERVED
*  Author: Fuyin
*  Description: �����ַ���������
\******************************************************************************/

#ifndef STRINGMAP_H
#define STRINGMAP_H

#include <vector>
#include <string>
#include <unordered_map>
#include "scriptparser.h"
#include "types.h"

using std::vector;
using std::string;
using std::wstring;
using std::unordered_map;

class ScriptParser;
///////////////////////////////////////////////////////////////////////////////

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
inline uint BKDRHash(const uchar *str, const uint len);

typedef unordered_map<uint, memstr> StringMap;

//�ַ��������뷭��
class Translator
{
public:
	Translator();
	Translator(ScriptParser& parser);

	memstr SearchStr(uint hash); //����һ��DWORD���ֵ��в����ַ���,DWORD�������ַ���hashֵ��Ҳ�������ڴ��ַ������ֵ
	memstr Translate(memstr str); //���������ַ�����������ַ���

	~Translator();
private:
	void insertString(acr_index *index, ulong index_count);
	StringMap strmap;
};



///////////////////////////////////////////////////////////////////////////////


class TranslateEngine
{
public:
	TranslateEngine();
	TranslateEngine(Translator& translator);

	bool Init(Translator& translator);
	
	bool Inject(void *dst, ulong dstlen);  //ֱ����Ŀ���ַע��ƥ����ַ�����ע��ɹ������棬���򷵻ؼ�
	memstr MatchString(void *dst, ulong dstlen);  //����ƥ����ַ������ֶ���д���봦��
	memstr MatchStringByOffset(ulong offset);  //���ݵ�ֱַ�ӷ���ƥ����ַ������ֶ���д���봦��

	~TranslateEngine();

private:
	Translator *translator;
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




#endif