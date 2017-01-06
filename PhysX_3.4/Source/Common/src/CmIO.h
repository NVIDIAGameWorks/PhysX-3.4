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


#ifndef PX_PHYSICS_COMMON_IO
#define PX_PHYSICS_COMMON_IO

#include "foundation/PxIO.h"
#include "foundation/PxAssert.h"
#include "foundation/PxMemory.h"
#include "CmPhysXCommon.h"

namespace physx
{

	// wrappers for IO classes so that we can add extra functionality (byte counting, buffering etc)

namespace Cm
{

class InputStreamReader
{
public:

	InputStreamReader(PxInputStream& stream) : mStream(stream) {	}
	PxU32	read(void* dest, PxU32 count)	
	{	
		PxU32 readLength = mStream.read(dest, count);

		// zero the buffer if we didn't get all the data
		if(readLength<count)
			PxMemZero(reinterpret_cast<PxU8*>(dest)+readLength, count-readLength);
	
		return readLength;
	}

	template <typename T> T get()			
	{		
		T val;	
		PxU32 length = mStream.read(&val, sizeof(T));
		PX_ASSERT(length == sizeof(T));
		PX_UNUSED(length);
		return val; 
	}


protected:
	PxInputStream &mStream;
private:
	InputStreamReader& operator=(const InputStreamReader&);
};


class InputDataReader : public InputStreamReader
{
public:
	InputDataReader(PxInputData& data) : InputStreamReader(data) {}
	InputDataReader &operator=(const InputDataReader &);

	PxU32	length() const					{		return getData().getLength();		}
	void	seek(PxU32 offset)				{		getData().seek(offset);				}
	PxU32	tell()							{		return getData().tell();			}

private:
	PxInputData& getData()					{		return static_cast<PxInputData&>(mStream); }
	const PxInputData& getData() const		{		return static_cast<const PxInputData&>(mStream); }
};


class OutputStreamWriter
{
public:

	PX_INLINE OutputStreamWriter(PxOutputStream& stream) 
	:	mStream(stream)
	,	mCount(0)
	{}

	PX_INLINE	PxU32	write(const void* src, PxU32 offset)		
	{		
		PxU32 count = mStream.write(src, offset);
		mCount += count;
		return count;
	}

	PX_INLINE	PxU32	getStoredSize()
	{
		return mCount;
	}

	template<typename T> void put(const T& val)	
	{		
		PxU32 length = write(&val, sizeof(T));		
		PX_ASSERT(length == sizeof(T));
		PX_UNUSED(length);
	}

private:

	OutputStreamWriter& operator=(const OutputStreamWriter&);
	PxOutputStream& mStream;
	PxU32 mCount;
};



}
}

#endif
