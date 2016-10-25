/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

#ifndef STREAM_IO_H
#define STREAM_IO_H

/*!
\file
\brief NsIOStream class
*/
#include "Ps.h"
#include "PsString.h"
#include "PxFileBuf.h"
#include <string.h>
#include <stdlib.h>

#define safePrintf nvidia::string::sprintf_s

PX_PUSH_PACK_DEFAULT

namespace nvidia
{

/**
\brief A wrapper class for physx::PxFileBuf that provides both binary and ASCII streaming capabilities
*/
class StreamIO
{
	static const uint32_t MAX_STREAM_STRING = 1024;
public:
	/**
	\param [in] stream the physx::PxFileBuf through which all reads and writes will be performed
	\param [in] streamLen the length of the input data stream when de-serializing
	*/
	StreamIO(physx::PxFileBuf &stream,uint32_t streamLen) : mStreamLen(streamLen), mStream(stream) { }
	~StreamIO(void) { }

	PX_INLINE StreamIO& operator<<(bool v);
	PX_INLINE StreamIO& operator<<(char c);
	PX_INLINE StreamIO& operator<<(uint8_t v);
	PX_INLINE StreamIO& operator<<(int8_t v);

	PX_INLINE StreamIO& operator<<(const char *c);
	PX_INLINE StreamIO& operator<<(int64_t v);
	PX_INLINE StreamIO& operator<<(uint64_t v);
	PX_INLINE StreamIO& operator<<(double v);
	PX_INLINE StreamIO& operator<<(float v);
	PX_INLINE StreamIO& operator<<(uint32_t v);
	PX_INLINE StreamIO& operator<<(int32_t v);
	PX_INLINE StreamIO& operator<<(uint16_t v);
	PX_INLINE StreamIO& operator<<(int16_t v);
	PX_INLINE StreamIO& operator<<(const physx::PxVec3 &v);
	PX_INLINE StreamIO& operator<<(const physx::PxQuat &v);
	PX_INLINE StreamIO& operator<<(const physx::PxBounds3 &v);

	PX_INLINE StreamIO& operator>>(const char *&c);
	PX_INLINE StreamIO& operator>>(bool &v);
	PX_INLINE StreamIO& operator>>(char &c);
	PX_INLINE StreamIO& operator>>(uint8_t &v);
	PX_INLINE StreamIO& operator>>(int8_t &v);
	PX_INLINE StreamIO& operator>>(int64_t &v);
	PX_INLINE StreamIO& operator>>(uint64_t &v);
	PX_INLINE StreamIO& operator>>(double &v);
	PX_INLINE StreamIO& operator>>(float &v);
	PX_INLINE StreamIO& operator>>(uint32_t &v);
	PX_INLINE StreamIO& operator>>(int32_t &v);
	PX_INLINE StreamIO& operator>>(uint16_t &v);
	PX_INLINE StreamIO& operator>>(int16_t &v);
	PX_INLINE StreamIO& operator>>(physx::PxVec3 &v);
	PX_INLINE StreamIO& operator>>(physx::PxQuat &v);
	PX_INLINE StreamIO& operator>>(physx::PxBounds3 &v);

	uint32_t getStreamLen(void) const { return mStreamLen; }

	physx::PxFileBuf& getStream(void) { return mStream; }

	PX_INLINE void storeString(const char *c,bool zeroTerminate=false);

private:
	StreamIO& operator=( const StreamIO& );


	uint32_t			mStreamLen; // the length of the input data stream when de-serializing.
	physx::PxFileBuf	&mStream;
	char				mReadString[MAX_STREAM_STRING]; // a temp buffer for streaming strings on input.
};

#include "StreamIO.inl" // inline methods...

} // end of nvidia namespace

PX_POP_PACK

#endif // STREAM_IO_H
