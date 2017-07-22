#include "replacemap.h"
#include <cstdio>

ReplaceMap::ReplaceMap()
{
}

ReplaceMap::~ReplaceMap()
{
}

bool ReplaceMap::FillMap(const std::string& strPkgName)
{
	if (strPkgName.empty())
	{
		return false;
	}

	FILE* fPkg = fopen(strPkgName.c_str(), "rb");
	if (!fPkg)
	{
		return false;
	}


	header_t oHeader;
	fread(&oHeader, sizeof(header_t), 1, fPkg);

	fseek(fPkg, oHeader.index_offset, SEEK_SET);

	ulong entry_count = oHeader.index_length / sizeof(entry_t);
	entry_t *pEntries = new entry_t[entry_count];

	fread(pEntries, oHeader.index_length, 1, fPkg);

	my_entry_t * pMyEntry = new my_entry_t[entry_count];
	memset(pMyEntry, 0, sizeof(my_entry_t) * entry_count);
	for (int i = 0; i < entry_count; i++)
	{
		fseek(fPkg, pEntries[i].name_offset, SEEK_SET);
		fread(pMyEntry[i].name, pEntries[i].name_length, 1, fPkg);

		pMyEntry[i].info.offset = pEntries[i].offset;
		pMyEntry[i].info.length = pEntries[i].length;
		pMyEntry[i].info.org_length = pEntries[i].org_length;

		m_oMap.insert(InfoMap::value_type(pMyEntry[i].name, pMyEntry[i].info));
	}

	delete[] pMyEntry;
	delete[] pEntries;
	fclose(fPkg);

}

bool ReplaceMap::GetInfo(const std::string& strKey, info_t* pOutInfo)
{
	if (!pOutInfo)
	{
		return false;
	}

	InfoMap::iterator iter = m_oMap.find(strKey);
	if (iter == m_oMap.end())
	{
		return false;
	}

	*pOutInfo = iter->second;
	return true;
}
