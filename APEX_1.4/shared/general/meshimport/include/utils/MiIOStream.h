/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MI_IOSTREAM_H
#define MI_IOSTREAM_H

/*!
\file
\brief MiIOStream class
*/
#include "MiFileBuf.h"
#include <string.h>
#include <stdlib.h>
#include "MiAsciiConversion.h"

#define safePrintf string::sprintf_s

MI_PUSH_PACK_DEFAULT

namespace mimp
{

	/**
\brief A wrapper class for MiFileBuf that provides both binary and ASCII streaming capabilities
*/
class MiIOStream
{
	static const MiU32 MAX_STREAM_STRING = 1024;
public:
	/**
	\param [in] stream the MiFileBuf through which all reads and writes will be performed
	\param [in] streamLen the length of the input data stream when de-serializing
	*/
	MiIOStream(MiFileBuf &stream,MiU32 streamLen) : mBinary(true), mStreamLen(streamLen), mStream(stream) { };
	~MiIOStream(void) { };

	/**
	\brief Set the stream to binary or ASCII

	\param [in] state if true, stream is binary, if false, stream is ASCII

	If the stream is binary, stream access is passed straight through to the respecitve 
	MiFileBuf methods.  If the stream is ASCII, all stream reads and writes are converted to
	human readable ASCII.
	*/
	MI_INLINE void setBinary(bool state) { mBinary = state; }
	MI_INLINE bool getBinary() { return mBinary; }

	MI_INLINE MiIOStream& operator<<(bool v);
	MI_INLINE MiIOStream& operator<<(char c);
	MI_INLINE MiIOStream& operator<<(MiU8 v);
	MI_INLINE MiIOStream& operator<<(MiI8 v);

	MI_INLINE MiIOStream& operator<<(const char *c);
	MI_INLINE MiIOStream& operator<<(MiI64 v);
	MI_INLINE MiIOStream& operator<<(MiU64 v);
	MI_INLINE MiIOStream& operator<<(MiF64 v);
	MI_INLINE MiIOStream& operator<<(MiF32 v);
	MI_INLINE MiIOStream& operator<<(MiU32 v);
	MI_INLINE MiIOStream& operator<<(MiI32 v);
	MI_INLINE MiIOStream& operator<<(MiU16 v);
	MI_INLINE MiIOStream& operator<<(MiI16 v);

	MI_INLINE MiIOStream& operator>>(const char *&c);
	MI_INLINE MiIOStream& operator>>(bool &v);
	MI_INLINE MiIOStream& operator>>(char &c);
	MI_INLINE MiIOStream& operator>>(MiU8 &v);
	MI_INLINE MiIOStream& operator>>(MiI8 &v);
	MI_INLINE MiIOStream& operator>>(MiI64 &v);
	MI_INLINE MiIOStream& operator>>(MiU64 &v);
	MI_INLINE MiIOStream& operator>>(MiF64 &v);
	MI_INLINE MiIOStream& operator>>(MiF32 &v);
	MI_INLINE MiIOStream& operator>>(MiU32 &v);
	MI_INLINE MiIOStream& operator>>(MiI32 &v);
	MI_INLINE MiIOStream& operator>>(MiU16 &v);
	MI_INLINE MiIOStream& operator>>(MiI16 &v);

	MiU32 getStreamLen(void) const { return mStreamLen; }

	MiFileBuf& getStream(void) { return mStream; }

	MI_INLINE void storeString(const char *c,bool zeroTerminate=false);

private:
	MiIOStream& operator=( const MiIOStream& );


	bool      mBinary; // true if we are serializing binary data.  Otherwise, everything is assumed converted to ASCII
	MiU32     mStreamLen; // the length of the input data stream when de-serializing.
	MiFileBuf &mStream;
	char			mReadString[MAX_STREAM_STRING]; // a temp buffer for streaming strings on input.
};

#include "MiIOStream.inl" // inline methods...

}; // end of physx namespace

MI_POP_PACK

#endif // MI_IOSTREAM_H
