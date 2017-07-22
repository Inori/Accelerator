#pragma once

#include "types.h"
#include <string>
#include <unordered_map>

#pragma pack (1)
typedef struct {
	char magic[8];			/* "FUYINPAK" */
	uint header_length;
	uint is_compressed;
	uint index_offset;
	uint index_length;
} header_t;

typedef struct {
	uint name_offset;
	uint name_length;
	uint offset;
	uint length;
	uint org_length;
} entry_t;

typedef struct {
	uint offset;
	uint length;
	uint org_length;
} info_t;

typedef struct {
	char name[MAX_PATH];
	info_t info;
} my_entry_t;

#pragma pack ()

typedef std::unordered_map<std::string, info_t> InfoMap;

class ReplaceMap
{
public:
	ReplaceMap();
	~ReplaceMap();

	bool FillMap(const std::string& strPkgName);

	bool GetInfo(const std::string& strKey, info_t* pOutInfo);
	
private:
	InfoMap m_oMap;
};
