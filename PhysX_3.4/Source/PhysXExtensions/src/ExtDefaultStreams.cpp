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

#include "foundation/PxPreprocessor.h"
#include "foundation/PxAssert.h"
#include "foundation/PxAllocatorCallback.h"
#include "foundation/PxMemory.h"

#include <errno.h>
#include "PsFoundation.h"
#include "PxDefaultStreams.h"
#include "SnFile.h"
#include "CmPhysXCommon.h"
#include "PsUtilities.h"
#include "PsBitUtils.h"

using namespace physx;

PxDefaultMemoryOutputStream::PxDefaultMemoryOutputStream(PxAllocatorCallback &allocator) 
:	mAllocator	(allocator)	
,	mData		(NULL)
,	mSize		(0)
,	mCapacity	(0)
{
}

PxDefaultMemoryOutputStream::~PxDefaultMemoryOutputStream()
{
	if(mData)
		mAllocator.deallocate(mData);
}

PxU32 PxDefaultMemoryOutputStream::write(const void* src, PxU32 size)
{
	PxU32 expectedSize = mSize + size;
	if(expectedSize > mCapacity)
	{
		mCapacity = PxMax(Ps::nextPowerOfTwo(expectedSize), 4096u);

		PxU8* newData = reinterpret_cast<PxU8*>(mAllocator.allocate(mCapacity,"PxDefaultMemoryOutputStream",__FILE__,__LINE__));
		PX_ASSERT(newData!=NULL);

		PxMemCopy(newData, mData, mSize);
		if(mData)
			mAllocator.deallocate(mData);

		mData = newData;
	}
	PxMemCopy(mData+mSize, src, size);
	mSize += size;
	return size;
}

///////////////////////////////////////////////////////////////////////////////

PxDefaultMemoryInputData::PxDefaultMemoryInputData(PxU8* data, PxU32 length) :
	mSize	(length),
	mData	(data),
	mPos	(0)
{
}

PxU32 PxDefaultMemoryInputData::read(void* dest, PxU32 count)
{
	PxU32 length = PxMin<PxU32>(count, mSize-mPos);
	PxMemCopy(dest, mData+mPos, length);
	mPos += length;
	return length;
}

PxU32 PxDefaultMemoryInputData::getLength() const
{
	return mSize;
}

void PxDefaultMemoryInputData::seek(PxU32 offset)
{
	mPos = PxMin<PxU32>(mSize, offset);
}

PxU32 PxDefaultMemoryInputData::tell() const
{
	return mPos;
}

PxDefaultFileOutputStream::PxDefaultFileOutputStream(const char* filename)
{
	mFile = NULL;
	sn::fopen_s(&mFile, filename, "wb");
	// PT: when this fails, check that:
	// - the path is correct
	// - the file does not already exist. If it does, check that it is not write protected.
	if(NULL == mFile)
	{
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, 
			"Unable to open file %s, errno 0x%x\n",filename,errno);
	}
	PX_ASSERT(mFile);
}

PxDefaultFileOutputStream::~PxDefaultFileOutputStream()
{
	if(mFile)
		fclose(mFile);
}

PxU32 PxDefaultFileOutputStream::write(const void* src, PxU32 count)
{
	return mFile ? PxU32(fwrite(src, 1, count, mFile)) : 0;
}

bool PxDefaultFileOutputStream::isValid()
{
	return mFile != NULL;
}

///////////////////////////////////////////////////////////////////////////////

PxDefaultFileInputData::PxDefaultFileInputData(const char* filename)
{
	mFile = NULL;
	sn::fopen_s(&mFile, filename, "rb");

	if(mFile)
	{
		fseek(mFile, 0, SEEK_END);
		mLength = PxU32(ftell(mFile));
		fseek(mFile, 0, SEEK_SET);
	}
	else
	{
		mLength = 0;
	}
}

PxDefaultFileInputData::~PxDefaultFileInputData()
{
	if(mFile)
		fclose(mFile);
}

PxU32 PxDefaultFileInputData::read(void* dest, PxU32 count)
{
	PX_ASSERT(mFile);
	const size_t size = fread(dest, 1, count, mFile);
	// there should be no assert here since by spec of PxInputStream we can read fewer bytes than expected
	return PxU32(size);
}

PxU32 PxDefaultFileInputData::getLength() const
{
	return mLength;
}

void PxDefaultFileInputData::seek(PxU32 pos)
{
	fseek(mFile, long(pos), SEEK_SET);
}

PxU32 PxDefaultFileInputData::tell() const
{
	return PxU32(ftell(mFile));
}

bool PxDefaultFileInputData::isValid() const
{
	return mFile != NULL;
}
