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

#ifndef PXPVDSDK_PXPVDBYTESTREAMS_H
#define PXPVDSDK_PXPVDBYTESTREAMS_H
#include "PxPvdObjectModelBaseTypes.h"

namespace physx
{
namespace pvdsdk
{

static inline uint32_t strLen(const char* inStr)
{
	uint32_t len = 0;
	if(inStr)
	{
		while(*inStr)
		{
			++len;
			++inStr;
		}
	}
	return len;
}

class PvdInputStream
{
  protected:
	virtual ~PvdInputStream()
	{
	}

  public:
	// Return false if you can't write the number of bytes requested
	// But make an absolute best effort to read the data...
	virtual bool read(uint8_t* buffer, uint32_t& len) = 0;

	template <typename TDataType>
	bool read(TDataType* buffer, uint32_t numItems)
	{
		uint32_t expected = numItems;
		uint32_t amountToRead = numItems * sizeof(TDataType);
		read(reinterpret_cast<uint8_t*>(buffer), amountToRead);
		numItems = amountToRead / sizeof(TDataType);
		PX_ASSERT(numItems == expected);
		return expected == numItems;
	}

	template <typename TDataType>
	PvdInputStream& operator>>(TDataType& data)
	{
		uint32_t dataSize = static_cast<uint32_t>(sizeof(TDataType));
		bool success = read(reinterpret_cast<uint8_t*>(&data), dataSize);
		// PX_ASSERT( success );
		// PX_ASSERT( dataSize == sizeof( data ) );
		(void)success;
		return *this;
	}
};

struct ByteSwappingPvdInputStream
{
  protected:
	ByteSwappingPvdInputStream& operator=(ByteSwappingPvdInputStream& other);

  public:
	PvdInputStream& mStream;
	ByteSwappingPvdInputStream(PvdInputStream& stream) : mStream(stream)
	{
	}

	template <typename TDataType>
	bool read(TDataType* buffer, uint32_t& numItems)
	{
		bool retval = mStream.read(buffer, numItems);
		for(uint32_t idx = 0; idx < numItems; ++idx)
			swapBytes(buffer[idx]);
		return retval;
	}

	template <typename TDataType>
	ByteSwappingPvdInputStream& operator>>(TDataType& data)
	{
		mStream >> data;
		swapBytes(data);
		return *this;
	}
};

class PvdOutputStream
{
  protected:
	virtual ~PvdOutputStream()
	{
	}

  public:
	// Return false if you can't write the number of bytes requested
	// But make an absolute best effort to write the data...
	virtual bool write(const uint8_t* buffer, uint32_t len) = 0;
	virtual bool directCopy(PvdInputStream& inStream, uint32_t len) = 0;

	template <typename TDataType>
	bool write(const TDataType* buffer, uint32_t numItems)
	{
		return write(reinterpret_cast<const uint8_t*>(buffer), numItems * sizeof(TDataType));
	}

	template <typename TDataType>
	PvdOutputStream& operator<<(const TDataType& data)
	{
		bool success = write(reinterpret_cast<const uint8_t*>(&data), sizeof(data));
		PX_ASSERT(success);
		(void)success;
		return *this;
	}

	PvdOutputStream& operator<<(const char* inString)
	{
		if(inString && *inString)
		{
			uint32_t len(strLen(inString));
			write(inString, len);
		}
		return *this;
	}
};
}
}
#endif // PXPVDSDK_PXPVDBYTESTREAMS_H
