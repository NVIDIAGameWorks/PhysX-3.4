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

#ifndef PXPVDSDK_PXPVDINTERNALBYTESTREAMS_H
#define PXPVDSDK_PXPVDINTERNALBYTESTREAMS_H

#include "PxPvdByteStreams.h"
#include "PxPvdFoundation.h"

namespace physx
{
namespace pvdsdk
{
struct MemPvdOutputStream : public PvdOutputStream
{
	ForwardingMemoryBuffer mBuffer;
	MemPvdOutputStream(const char* memName) : mBuffer(memName)
	{
	}

	virtual bool write(const uint8_t* buffer, uint32_t len)
	{
		mBuffer.write(buffer, len);
		return true;
	}

	virtual bool directCopy(PvdInputStream& inStream, uint32_t len)
	{
		uint32_t offset = mBuffer.size();
		mBuffer.growBuf(len);
		uint32_t readLen = len;
		inStream.read(mBuffer.begin() + offset, readLen);
		if(readLen != len)
			physx::intrinsics::memZero(mBuffer.begin() + offset, len);
		return readLen == len;
	}

	const uint8_t* begin() const
	{
		return mBuffer.begin();
	}
	uint32_t size() const
	{
		return mBuffer.size();
	}
	void clear()
	{
		mBuffer.clear();
	}
	DataRef<const uint8_t> toRef() const
	{
		return DataRef<const uint8_t>(mBuffer.begin(), mBuffer.end());
	}
};

struct MemPvdInputStream : public PvdInputStream
{
	const uint8_t* mBegin;
	const uint8_t* mEnd;
	bool mGood;

	MemPvdInputStream(const MemPvdOutputStream& stream) : mGood(true)
	{
		mBegin = stream.mBuffer.begin();
		mEnd = stream.mBuffer.end();
	}

	MemPvdInputStream(const uint8_t* beg = NULL, const uint8_t* end = NULL)
	{
		mBegin = beg;
		mEnd = end;
		mGood = true;
	}

	uint32_t size() const
	{
		return mGood ? static_cast<uint32_t>(mEnd - mBegin) : 0;
	}
	bool isGood() const
	{
		return mGood;
	}

	void setup(uint8_t* start, uint8_t* stop)
	{
		mBegin = start;
		mEnd = stop;
	}

	void nocopyRead(uint8_t*& buffer, uint32_t& len)
	{
		if(len == 0 || mGood == false)
		{
			len = 0;
			buffer = NULL;
			return;
		}
		uint32_t original = len;
		len = PxMin(len, size());
		if(mGood && len != original)
			mGood = false;
		buffer = const_cast<uint8_t*>(mBegin);
		mBegin += len;
	}

	virtual bool read(uint8_t* buffer, uint32_t& len)
	{
		if(len == 0)
			return true;
		uint32_t original = len;
		len = PxMin(len, size());

		physx::intrinsics::memCopy(buffer, mBegin, len);
		mBegin += len;
		if(len < original)
			physx::intrinsics::memZero(buffer + len, original - len);
		mGood = mGood && len == original;
		return mGood;
	}
};
}
}
#endif // PXPVDSDK_PXPVDINTERNALBYTESTREAMS_H
