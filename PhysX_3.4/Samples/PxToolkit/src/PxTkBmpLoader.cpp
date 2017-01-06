// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "PxTkBmpLoader.h"
#include "PxTkFile.h"


using namespace PxToolkit;

#define MAKETWOCC(a,b) ( (static_cast<char>(a)) | ((static_cast<char>(b))<< 8) ) 

static bool isBigEndian() { int i = 1; return *(reinterpret_cast<char*>(&i))==0; }

static unsigned short endianSwap(unsigned short nValue)
{
   return (((nValue>> 8)) | (nValue << 8));

}

static unsigned int endianSwap(unsigned int i)
{
  unsigned char b1, b2, b3, b4;

  b1 = i & 255;
  b2 = ( i >> 8 ) & 255;
  b3 = ( i>>16 ) & 255;
  b4 = ( i>>24 ) & 255;

  return (static_cast<unsigned int>(b1) << 24) + (static_cast<unsigned int>(b2) << 16) + (static_cast<unsigned int>(b3) << 8) + b4;
}

// -------------------------------------------------------------------

#pragma pack(1)

struct BMPHEADER {
	unsigned short		Type;
	unsigned int		Size;
	unsigned short		Reserved1;
	unsigned short		Reserved2;
	unsigned int		OffBits;
};

// Only Win3.0 BMPINFO (see later for OS/2)
struct BMPINFO {
	unsigned int		Size;
	unsigned int		Width;
	unsigned int		Height;
	unsigned short		Planes;
	unsigned short		BitCount;
	unsigned int		Compression;
	unsigned int		SizeImage;
	unsigned int		XPelsPerMeter;
	unsigned int		YPelsPerMeter;
	unsigned int		ClrUsed;
	unsigned int		ClrImportant;
};

#pragma pack()

// Compression Type
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

// -------------------------------------------------------------------
BmpLoader::BmpLoader() :
	mWidth		(0),
	mHeight		(0),
	mRGB		(NULL),
	mHasAlpha	(false)
{
}

// -------------------------------------------------------------------
BmpLoader::~BmpLoader() 
{
	if (mRGB) free(mRGB);
}

// -------------------------------------------------------------------
bool BmpLoader::loadBmp(PxFileHandle f)
{
	if (!f) return false;

	if (mRGB) {
		free(mRGB);
		mRGB = NULL;
	}
	mWidth = 0;
	mHeight = 0;

	size_t num;
	BMPHEADER header;
	num = fread(&header, 1, sizeof(BMPHEADER), f);
	if(isBigEndian()) header.Type = endianSwap(header.Type);
	if (num != sizeof(BMPHEADER)) { fclose(f); return false; }
	if (header.Type != MAKETWOCC('B','M')) { fclose(f); return false; }

	BMPINFO info;
	num = fread(&info, 1, sizeof(BMPINFO), f);
	if (num != sizeof(BMPINFO)) { fclose(f); return false; }
	if(isBigEndian()) info.Size = endianSwap(info.Size);
	if(isBigEndian()) info.BitCount = endianSwap(info.BitCount);
	if(isBigEndian()) info.Compression = endianSwap(info.Compression);
	if(isBigEndian()) info.Width = endianSwap(info.Width);
	if(isBigEndian()) info.Height = endianSwap(info.Height);

	if (info.Size != sizeof(BMPINFO)) { fclose(f); return false; }
	if (info.BitCount != 24 && info.BitCount != 32) { fclose(f); return false; }
	if (info.Compression != BI_RGB) { fclose(f); return false; }

	mWidth	= info.Width;
	mHeight	= info.Height;

	int bytesPerPixel = 0;
	if(info.BitCount == 24)
	{
		mHasAlpha = false;
		bytesPerPixel = 3;
	}
	else if(info.BitCount == 32)
	{
		mHasAlpha = true;
		bytesPerPixel = 4;
	}
	else assert(0);

	mRGB = static_cast<unsigned char*>(malloc(mWidth * mHeight * bytesPerPixel));

	int lineLen = (((info.Width * (info.BitCount>>3)) + 3)>>2)<<2;
	unsigned char* line = static_cast<unsigned char*>(malloc(lineLen));

	for(int i = info.Height-1; i >= 0; i--)
	{
		num = fread(line, 1, static_cast<size_t>(lineLen), f);
		if (num != static_cast<size_t>(lineLen)) { fclose(f); return false; }
		unsigned char* src = line;
		unsigned char* dest = mRGB + i*info.Width*bytesPerPixel;
		for(unsigned int j = 0; j < info.Width; j++)
		{
			unsigned char b = *src++;
			unsigned char g = *src++;
			unsigned char r = *src++;
			unsigned char a = mHasAlpha ? *src++ : 0;
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;
			if(mHasAlpha)
				*dest++ = a;
		}
	}

	free(line);
	return true;
}

// -------------------------------------------------------------------
bool BmpLoader::loadBmp(const char *filename)
{
	PxFileHandle f = NULL;
	PxToolkit::fopen_s(&f, filename, "rb");
	bool ret = loadBmp(f); 
	if(f) fclose(f);
	return ret;
}
