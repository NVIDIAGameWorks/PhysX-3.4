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

#ifndef PSFILEBUFFER_PSIOSTREAM_H
#define PSFILEBUFFER_PSIOSTREAM_H

/*!
\file
\brief PsIOStream class
*/
#include "filebuf/PxFileBuf.h"

#include "Ps.h"
#include "PsString.h"
#include <string.h>
#include <stdlib.h>
#include "PsAsciiConversion.h"

#define safePrintf physx::shdfnd::snprintf

PX_PUSH_PACK_DEFAULT

namespace physx
{
	namespace general_PxIOStream2
	{

/**
\brief A wrapper class for physx::PxFileBuf that provides both binary and ASCII streaming capabilities
*/
class PsIOStream
{
	static const uint32_t MAX_STREAM_STRING = 1024;
public:
	/**
	\param [in] stream the physx::PxFileBuf through which all reads and writes will be performed
	\param [in] streamLen the length of the input data stream when de-serializing
	*/
	PsIOStream(physx::PxFileBuf &stream,uint32_t streamLen) : mBinary(true), mStreamLen(streamLen), mStream(stream) { }
	~PsIOStream(void) { }

	/**
	\brief Set the stream to binary or ASCII

	\param [in] state if true, stream is binary, if false, stream is ASCII

	If the stream is binary, stream access is passed straight through to the respecitve 
	physx::PxFileBuf methods.  If the stream is ASCII, all stream reads and writes are converted to
	human readable ASCII.
	*/
	PX_INLINE void setBinary(bool state) { mBinary = state; }
	PX_INLINE bool getBinary() { return mBinary; }

	PX_INLINE PsIOStream& operator<<(bool v);
	PX_INLINE PsIOStream& operator<<(char c);
	PX_INLINE PsIOStream& operator<<(uint8_t v);
	PX_INLINE PsIOStream& operator<<(int8_t v);

	PX_INLINE PsIOStream& operator<<(const char *c);
	PX_INLINE PsIOStream& operator<<(int64_t v);
	PX_INLINE PsIOStream& operator<<(uint64_t v);
	PX_INLINE PsIOStream& operator<<(double v);
	PX_INLINE PsIOStream& operator<<(float v);
	PX_INLINE PsIOStream& operator<<(uint32_t v);
	PX_INLINE PsIOStream& operator<<(int32_t v);
	PX_INLINE PsIOStream& operator<<(uint16_t v);
	PX_INLINE PsIOStream& operator<<(int16_t v);
	PX_INLINE PsIOStream& operator<<(const physx::PxVec3 &v);
	PX_INLINE PsIOStream& operator<<(const physx::PxQuat &v);
	PX_INLINE PsIOStream& operator<<(const physx::PxBounds3 &v);

	PX_INLINE PsIOStream& operator>>(const char *&c);
	PX_INLINE PsIOStream& operator>>(bool &v);
	PX_INLINE PsIOStream& operator>>(char &c);
	PX_INLINE PsIOStream& operator>>(uint8_t &v);
	PX_INLINE PsIOStream& operator>>(int8_t &v);
	PX_INLINE PsIOStream& operator>>(int64_t &v);
	PX_INLINE PsIOStream& operator>>(uint64_t &v);
	PX_INLINE PsIOStream& operator>>(double &v);
	PX_INLINE PsIOStream& operator>>(float &v);
	PX_INLINE PsIOStream& operator>>(uint32_t &v);
	PX_INLINE PsIOStream& operator>>(int32_t &v);
	PX_INLINE PsIOStream& operator>>(uint16_t &v);
	PX_INLINE PsIOStream& operator>>(int16_t &v);
	PX_INLINE PsIOStream& operator>>(physx::PxVec3 &v);
	PX_INLINE PsIOStream& operator>>(physx::PxQuat &v);
	PX_INLINE PsIOStream& operator>>(physx::PxBounds3 &v);

	uint32_t getStreamLen(void) const { return mStreamLen; }

	physx::PxFileBuf& getStream(void) { return mStream; }

	PX_INLINE void storeString(const char *c,bool zeroTerminate=false);

private:
	PsIOStream& operator=( const PsIOStream& );


	bool      mBinary; // true if we are serializing binary data.  Otherwise, everything is assumed converted to ASCII
	uint32_t     mStreamLen; // the length of the input data stream when de-serializing.
	physx::PxFileBuf &mStream;
	char			mReadString[MAX_STREAM_STRING]; // a temp buffer for streaming strings on input.
};

#include "PsIOStream.inl" // inline methods...

	} // end of namespace
	using namespace general_PxIOStream2;
} // end of physx namespace

PX_POP_PACK

#endif // PSFILEBUFFER_PSIOSTREAM_H
