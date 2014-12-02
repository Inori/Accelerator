#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H
#include <string>
#include <vector>
#include "types.h"

using std::string;
using std::vector;

typedef void *HANDLE;

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

//�ӿ�
class ScriptParser
{
public:
	virtual acr_index *Parse() = 0;
	virtual ulong GetStrCount() = 0;
};

//�����Ʒ������
class AcrParser: public ScriptParser
{
public:
	AcrParser();
	AcrParser(string fname);
	AcrParser(const AcrParser& orig);

	bool Init(string fname);
	acr_index *Parse();
	ulong GetStrCount();

	~AcrParser();
private:
	HANDLE hfile; //������
	acr_index *index_list;

	ulong real_size;
	byte *real_data; //ȥ���ļ�ͷ��Ĳ���,�����������ַ�������.��Ϸ�˳�ǰ�����ͷţ�

	ulong index_count;
	bool is_compressed;
};


//�ı��ļ���������Ҫ���ڵ��Խ׶�
class TextParser: public ScriptParser
{
public:
	TextParser();
	TextParser(string fname);
	TextParser(const TextParser& orig);

	acr_index *Parse();
	ulong GetStrCount();

	~TextParser();

private:
	FILE *fin;
	ulong file_size;

	vector<acr_index> index; //��������
	byte *real_data;  //�ַ�������

};















#endif 