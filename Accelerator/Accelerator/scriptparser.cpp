
#include <Windows.h>

#include <new>
#include "zlib.h"
#include "scriptparser.h"




//////////////////////////////////////////////////////////////////////////////////
AcrParser::AcrParser()
{
	is_compressed = false;
}

AcrParser::AcrParser(string fname)
{
	is_compressed = false;
	Init(fname);
}

AcrParser::AcrParser(const AcrParser& orig) : 
hfile(orig.hfile), 
real_size(orig.real_size),
index_count(orig.index_count), 
is_compressed(orig.is_compressed)
{
	index_list = new acr_index[index_count];
	memcpy(index_list, orig.index_list, orig.index_count * sizeof(acr_index));

	real_data = new byte[real_size];
	memcpy(real_data, orig.real_data, orig.real_size);
}

bool AcrParser::Init(string fname)
{
	hfile = CreateFileA(fname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "Can not open script file!", "Error", MB_OK);
		return false;
	}

	return true;
}

acr_index *AcrParser::Parse()
{
	byte *data;
	ulong fsize = GetFileSize(hfile, NULL);

	try
	{
		data = new byte[fsize];
		ulong readlen = 0;
		ReadFile(hfile, data, fsize, &readlen, NULL);
	}
	catch (std::bad_alloc e)
	{
		MessageBoxA(NULL, "Exception", e.what(), MB_OK);
		return NULL;
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, "Exception", e.what(), MB_OK);
		return NULL;
	}
	catch (...)
	{
		MessageBoxA(NULL, "Exception", "Unknown read file error.", MB_OK);
		return NULL;
	}


	acr_header *header = (acr_header*)data;
	is_compressed = header->compress_flag;

	byte *comp_data = data + sizeof(acr_header);

	if (is_compressed)
	{
		real_size = header->orgsize;
		ulong comp_size = header->compsize;
		real_data = new byte[real_size];
		if (uncompress(real_data, &real_size, comp_data, comp_size) != Z_OK)
		{
			MessageBoxA(NULL, "zlib uncompress failed!", "Error", MB_OK);
			return NULL;
		}
		delete[] data;
	}
	else
	{
		real_data = comp_data;
		real_size = fsize - sizeof(acr_header);
	}


	index_count = header->index_count;
	index_list = new acr_index[index_count];
	acr_index *real_index = (acr_index *)real_data;

	//计算字符串在内存中的实际地址
	for (ulong i = 0; i < index_count; i++)
	{
		index_list[i].hash = real_index->hash;
		index_list[i].old_str_off = (real_index->old_str_off + (ulong)data);
		index_list[i].old_str_len = real_index->old_str_len;
		index_list[i].new_str_off = (real_index->new_str_off + (ulong)data);
		index_list[i].new_str_len = real_index->new_str_len;
		real_index++;
	}
	return index_list;
}

ulong AcrParser::GetStrCount()
{
	return index_count;
}

AcrParser::~AcrParser()
{
	delete[] real_data;
	delete[] index_list;
	CloseHandle(hfile);
}

/////////////////////////////////////////////////////////////////////////////////////////


TextParser::TextParser()
{
}

TextParser::TextParser(string fname)
{
	fin = fopen(fname.c_str(), "rb");
	if (!fin)
	{
		MessageBox(NULL, "Error", "Open script file failed!", MB_OK);
		ExitProcess(NULL);
	}
	
	fseek(fin, 0, SEEK_END);
	file_size = ftell(fin);
	fseek(fin, 0, SEEK_SET);

}

TextParser::TextParser(const TextParser& orig)
{

}

TextParser::~TextParser()
{
}