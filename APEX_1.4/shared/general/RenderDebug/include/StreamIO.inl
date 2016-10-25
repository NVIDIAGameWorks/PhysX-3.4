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
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.


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

/*!
\file
\brief StreamIO inline implementation
*/

PX_INLINE StreamIO& StreamIO::operator<<(bool v)
{
	mStream.storeByte(uint8_t(v));
	return *this;
}


PX_INLINE StreamIO& StreamIO::operator<<(char c)
{
	mStream.storeByte(uint8_t(c));
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(uint8_t c)
{
	mStream.storeByte(uint8_t(c));
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(int8_t c)
{
	mStream.storeByte(uint8_t(c));
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(const char *c)
{
	c = c ? c : ""; // it it is a null pointer, assign it to an empty string.
	uint32_t len = uint32_t(strlen(c));
	PX_ASSERT( len < (MAX_STREAM_STRING-1));
	if ( len > (MAX_STREAM_STRING-1) )
	{
		len = MAX_STREAM_STRING-1;
	}
	mStream.storeDword(len);
	if ( len )
	{
		mStream.write(c,len);
	}
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(uint64_t v)
{
	mStream.storeDouble(static_cast<double>(v));
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(int64_t v)
{
	mStream.storeDouble(static_cast<double>(v));
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(double v)
{
	mStream.storeDouble(static_cast<double>(v));
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(float v)
{
	mStream.storeFloat(v);
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(uint32_t v)
{
	mStream.storeDword(v);
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(int32_t v)
{
	mStream.storeDword( uint32_t(v) );
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(uint16_t v)
{
	mStream.storeWord(v);
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(int16_t v)
{
	mStream.storeWord( uint16_t(v) );
	return *this;
}


PX_INLINE StreamIO& StreamIO::operator>>(uint32_t &v)
{
	v = mStream.readDword();
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(char &v)
{
	v = char(mStream.readByte());
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(uint8_t &v)
{
	v = mStream.readByte();
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(int8_t &v)
{
	v = int8_t(mStream.readByte());
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(int64_t &v)
{
	v = mStream.readDword();
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(uint64_t &v)
{
	v = uint64_t(mStream.readDouble());
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(double &v)
{
	v = mStream.readDouble();
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(float &v)
{
	v = mStream.readFloat();
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(int32_t &v)
{
	v = int32_t(mStream.readDword());
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(uint16_t &v)
{
	v = mStream.readWord();
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(int16_t &v)
{
	v = int16_t(mStream.readWord());
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(bool &v)
{
	int8_t iv;
	iv = int8_t(mStream.readByte());
	v = iv ? true : false;
	return *this;
}

#define IOSTREAM_COMMA_SEPARATOR 

PX_INLINE StreamIO& StreamIO::operator<<(const physx::PxVec3 &v)
{
	*this << v.x; 
	IOSTREAM_COMMA_SEPARATOR;
	*this << v.y;
	IOSTREAM_COMMA_SEPARATOR;
	*this << v.z;
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator<<(const physx::PxQuat &v)
{
	*this << v.x;
	IOSTREAM_COMMA_SEPARATOR;
	*this << v.y;
	IOSTREAM_COMMA_SEPARATOR;
	*this << v.z;
	IOSTREAM_COMMA_SEPARATOR;
	*this << v.w;
	return *this;
}


PX_INLINE StreamIO& StreamIO::operator<<(const physx::PxBounds3 &v)
{
	*this << v.minimum;
	IOSTREAM_COMMA_SEPARATOR;
	*this << v.maximum;
	return *this;
}



PX_INLINE StreamIO& StreamIO::operator>>(physx::PxVec3 &v)
{
	*this >> v.x;
	*this >> v.y;
	*this >> v.z;
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(physx::PxQuat &v)
{
	*this>>v.x;
	*this>>v.y;
	*this>>v.z;
	*this>>v.w;
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(physx::PxBounds3 &v)
{
	*this >> v.minimum;
	*this >> v.maximum;
	return *this;
}

PX_INLINE StreamIO& StreamIO::operator>>(const char *&str)
{
	str = NULL; // by default no string streamed...
	uint32_t len=0;
	*this >> len;
	PX_ASSERT( len < (MAX_STREAM_STRING-1) );
	if ( len < (MAX_STREAM_STRING-1) )
	{
		mStream.read(mReadString,len);
		mReadString[len] = 0;
		str = mReadString;
	}
	return *this;
}


PX_INLINE void  StreamIO::storeString(const char *c,bool zeroTerminate)
{
	while ( *c )
	{
		mStream.storeByte(uint8_t(*c));
		c++;
	}
	if ( zeroTerminate )
	{
		mStream.storeByte(0);
	}
}
