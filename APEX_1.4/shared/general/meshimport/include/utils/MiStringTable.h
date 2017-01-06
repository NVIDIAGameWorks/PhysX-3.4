/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MI_STRING_TABLE_H

#define MI_STRING_TABLE_H

#pragma warning( push )
#pragma warning( disable: 4555 ) // expression has no effect; expected expression with side-effect
#pragma warning( pop )

#include <assert.h>
#include "MiPlatformConfig.h"

//********** String Table
#pragma warning(push)
#pragma warning(disable:4996)

namespace mimp
{

class StringSort
{
public:
	bool operator()(const char *str1,const char *str2) const
	{
		return MESH_IMPORT_STRING::stricmp(str1,str2) < 0;
	}
};

typedef STDNAME::map< const char *, const char *, StringSort> StringMap;

class StringTable : public mimp::MeshImportAllocated
{
public:

	StringTable(void)
	{
	};

	~StringTable(void)
	{
		release();
	}

	void release(void)
	{
		for (StringMap::iterator i=mStrings.begin(); i!=mStrings.end(); ++i)
        {
            const char *string = (*i).second;
			MI_FREE( (void *)string );
        }
        mStrings.clear();
	}

	const char * Get(const char *str,bool &first)
	{
		str = str ? str : "";
		const char *ret;
		StringMap::iterator found = mStrings.find(str);
		if ( found == mStrings.end() )
		{
			MiU32 slen = (MiU32)strlen(str);
			char *string = (char *)MI_ALLOC(slen+1);
			strcpy(string,str);
			mStrings[string] = string;
			ret = string;
			first = true;
		}
		else
		{
			first = false;
			ret = (*found).second;
		}
        return ret;
	};


private:
    StringMap       mStrings;
//	char				mScratch[512];
};

class StringTableInt
{
public:
	typedef STDNAME::map< size_t, MiU32 > StringIntMap;

	bool Get(const char *str,MiU32 &v)
	{
		bool ret = false;
		bool first;
		str = mStringTable.Get(str,first);
		size_t index = (size_t)str;

		StringIntMap::iterator found = mStringInt.find(index);
		if ( found != mStringInt.end() )
		{
			v = (*found).second;
			ret = true;
		}
		return ret;
	}

	MiU32 Get(const char *str)
	{
		MiU32 ret=0;
		Get(str,ret);
		return ret;
	}

	void Add(const char *str,MiU32 v)
	{
		bool first;
		str = mStringTable.Get(str,first);
		size_t index = (size_t)str;
		StringIntMap::iterator found = mStringInt.find(index);
		if ( found != mStringInt.end() )
		{
			assert(0);
		}
		else
		{
			mStringInt[index] = v;
		}
	}

private:
	StringTable		mStringTable;
	StringIntMap	mStringInt;
};

}; // end of namespace

#pragma warning(pop)

#endif
