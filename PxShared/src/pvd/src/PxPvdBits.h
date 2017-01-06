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

#ifndef PXPVDSDK_PXPVDBITS_H
#define PXPVDSDK_PXPVDBITS_H

#include "PxPvdObjectModelBaseTypes.h"

namespace physx
{
namespace pvdsdk
{

// Marshallers cannot assume src is aligned, but they can assume dest is aligned.
typedef void (*TSingleMarshaller)(const uint8_t* src, uint8_t* dest);
typedef void (*TBlockMarshaller)(const uint8_t* src, uint8_t* dest, uint32_t numItems);

template <uint8_t ByteCount>
static inline void doSwapBytes(uint8_t* __restrict inData)
{
	for(uint32_t idx = 0; idx < ByteCount / 2; ++idx)
	{
		uint32_t endIdx = ByteCount - idx - 1;
		uint8_t theTemp = inData[idx];
		inData[idx] = inData[endIdx];
		inData[endIdx] = theTemp;
	}
}

template <uint8_t ByteCount>
static inline void doSwapBytes(uint8_t* __restrict inData, uint32_t itemCount)
{
	uint8_t* end = inData + itemCount * ByteCount;
	for(; inData < end; inData += ByteCount)
		doSwapBytes<ByteCount>(inData);
}

static inline void swapBytes(uint8_t* __restrict dataPtr, uint32_t numBytes, uint32_t itemWidth)
{
	uint32_t numItems = numBytes / itemWidth;
	switch(itemWidth)
	{
	case 1:
		break;
	case 2:
		doSwapBytes<2>(dataPtr, numItems);
		break;
	case 4:
		doSwapBytes<4>(dataPtr, numItems);
		break;
	case 8:
		doSwapBytes<8>(dataPtr, numItems);
		break;
	case 16:
		doSwapBytes<16>(dataPtr, numItems);
		break;
	default:
		PX_ASSERT(false);
		break;
	}
}

template <uint8_t TByteCount, bool TShouldSwap>
struct PvdByteSwapper
{
	void swapBytes(uint8_t* __restrict inData)
	{
		doSwapBytes<TByteCount>(inData);
	}
	void swapBytes(uint8_t* __restrict inData, uint32_t itemCount)
	{
		doSwapBytes<TByteCount>(inData, itemCount);
	}
	void swapBytes(uint8_t* __restrict dataPtr, uint32_t numBytes, uint32_t itemWidth)
	{
		physx::pvdsdk::swapBytes(dataPtr, numBytes, itemWidth);
	}
};

struct PvdNullSwapper
{

	void swapBytes(uint8_t* __restrict)
	{
	}
	void swapBytes(uint8_t* __restrict, uint32_t)
	{
	}
	void swapBytes(uint8_t* __restrict, uint32_t, uint32_t)
	{
	}
};
// Anything that doesn't need swapping gets the null swapper
template <uint8_t TByteCount>
struct PvdByteSwapper<TByteCount, false> : public PvdNullSwapper
{
};
// A 1 byte byte swapper can't really do anything.
template <>
struct PvdByteSwapper<1, true> : public PvdNullSwapper
{
};

static inline void swapBytes(uint8_t&)
{
}
static inline void swapBytes(int8_t&)
{
}
static inline void swapBytes(uint16_t& inData)
{
	doSwapBytes<2>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(int16_t& inData)
{
	doSwapBytes<2>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(uint32_t& inData)
{
	doSwapBytes<4>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(int32_t& inData)
{
	doSwapBytes<4>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(float& inData)
{
	doSwapBytes<4>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(uint64_t& inData)
{
	doSwapBytes<8>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(int64_t& inData)
{
	doSwapBytes<8>(reinterpret_cast<uint8_t*>(&inData));
}
static inline void swapBytes(double& inData)
{
	doSwapBytes<8>(reinterpret_cast<uint8_t*>(&inData));
}

static inline bool checkLength(const uint8_t* inStart, const uint8_t* inStop, uint32_t inLength)
{
	return static_cast<uint32_t>(inStop - inStart) >= inLength;
}
}
}
#endif // PXPVDSDK_PXPVDBITS_H
