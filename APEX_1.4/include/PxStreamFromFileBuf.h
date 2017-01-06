/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_STREAM_FROM_FILE_BUF_H
#define PX_STREAM_FROM_FILE_BUF_H

/*!
\file
\brief Conversion utilities between PhysX 2.8 and 3.0 data types
*/

#include "ApexDefs.h"
#include "foundation/PxIO.h"
#include "filebuf/PxFileBuf.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief A wrapper class that provides an NvStream API for a PxFileBuf
*/
class PxStreamFromFileBuf : 
	public physx::PxInputStream, public physx::PxOutputStream
{
private:
	physx::PxFileBuf& mFileBuf;
	PxStreamFromFileBuf& operator=(const PxStreamFromFileBuf&)
	{
		return *this;
	}

public:
	//! \brief Constructor that takes a PxFileBuf
	PxStreamFromFileBuf(physx::PxFileBuf& fb) : mFileBuf(fb) {}

	//! \brief Read data from the stream
	uint32_t	read(void* dest, uint32_t count)
	{
		return mFileBuf.read(dest, count);
	}

	//! \brief Read data to the stream
	uint32_t write(const void* src, uint32_t count)
	{
		return mFileBuf.write(src, count);
	}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // PX_STREAM_FROM_FILE_BUF_H
