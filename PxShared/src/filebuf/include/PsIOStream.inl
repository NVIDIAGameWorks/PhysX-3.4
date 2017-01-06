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
\brief PsIOStream inline implementation
*/

PX_INLINE PsIOStream& PsIOStream::operator<<(bool v)
{
	if ( mBinary )
	{
		mStream.storeByte((uint8_t)v);
	}
	else
	{
		char scratch[6];
		storeString( physx::PxAsc::valueToStr(v, scratch, 6) );
	}
	return *this;
}


PX_INLINE PsIOStream& PsIOStream::operator<<(char c)
{
	mStream.storeByte((uint8_t)c);
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(uint8_t c)
{
	if ( mBinary )
	{
		mStream.storeByte((uint8_t)c);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(c, scratch, physx::PxAsc::IntStrLen) );
	}

	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(int8_t c)
{
	if ( mBinary )
	{
		mStream.storeByte((uint8_t)c);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(c, scratch, physx::PxAsc::IntStrLen) );
	}

	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(const char *c)
{
	if ( mBinary )
	{
		c = c ? c : ""; // it it is a null pointer, assign it to an empty string.
		uint32_t len = (uint32_t)strlen(c);
		PX_ASSERT( len < (MAX_STREAM_STRING-1));
		if ( len > (MAX_STREAM_STRING-1) )
		{
			len = MAX_STREAM_STRING-1;
		}
		mStream.storeDword(len);
		if ( len )
			mStream.write(c,len);
	}
	else
	{
		storeString(c);
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(uint64_t v)
{
	if ( mBinary )
	{
		mStream.storeDouble( (double) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(int64_t v)
{
	if ( mBinary )
	{
		mStream.storeDouble( (double) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(double v)
{
	if ( mBinary )
	{
		mStream.storeDouble( (double) v );
	}
	else
	{
		char scratch[physx::PxAsc::PxF64StrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::PxF64StrLen) );
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(float v)
{
	if ( mBinary )
	{
		mStream.storeFloat(v);
	}
	else
	{
		char scratch[physx::PxAsc::PxF32StrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::PxF32StrLen) );

	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(uint32_t v)
{
	if ( mBinary )
	{
		mStream.storeDword(v);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(int32_t v)
{
	if ( mBinary )
	{
		mStream.storeDword( (uint32_t) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(uint16_t v)
{
	if ( mBinary )
	{
		mStream.storeWord(v);
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(int16_t v)
{
	if ( mBinary )
	{
		mStream.storeWord( (uint16_t) v );
	}
	else
	{
		char scratch[physx::PxAsc::IntStrLen];
		storeString( physx::PxAsc::valueToStr(v, scratch, physx::PxAsc::IntStrLen) );
	}
	return *this;
}


PX_INLINE PsIOStream& PsIOStream::operator>>(uint32_t &v)
{
	if ( mBinary )
	{
		v = mStream.readDword();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(char &v)
{
	if ( mBinary )
	{
		v = (char)mStream.readByte();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(uint8_t &v)
{
	if ( mBinary )
	{
		v = mStream.readByte();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(int8_t &v)
{
	if ( mBinary )
	{
		v = (int8_t)mStream.readByte();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(int64_t &v)
{
	if ( mBinary )
	{
		v = mStream.readDword();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(uint64_t &v)
{
	if ( mBinary )
	{
		v = (uint64_t)mStream.readDouble();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(double &v)
{
	if ( mBinary )
	{
		v = mStream.readDouble();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(float &v)
{
	if ( mBinary )
	{
		v = mStream.readFloat();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(int32_t &v)
{
	if ( mBinary )
	{
		v = (int32_t)mStream.readDword();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(uint16_t &v)
{
	if ( mBinary )
	{
		v = mStream.readWord();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(int16_t &v)
{
	if ( mBinary )
	{
		v = (int16_t)mStream.readWord();
	}
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(bool &v)
{
	int8_t iv;
	iv = (int8_t)mStream.readByte();
	v = iv ? true : false;
	return *this;
}

#define NX_IOSTREAM_COMMA_SEPARATOR if(!mBinary) *this << ' ';

PX_INLINE PsIOStream& PsIOStream::operator<<(const physx::PxVec3 &v)
{
	*this << v.x; 
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.y;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.z;
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator<<(const physx::PxQuat &v)
{
	*this << v.x;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.y;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.z;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.w;
	return *this;
}


PX_INLINE PsIOStream& PsIOStream::operator<<(const physx::PxBounds3 &v)
{
	*this << v.minimum;
	NX_IOSTREAM_COMMA_SEPARATOR;
	*this << v.maximum;
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(physx::PxVec3 &v)
{
	*this >> v.x;
	*this >> v.y;
	*this >> v.z;
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(physx::PxQuat &v)
{
	*this>>v.x;
	*this>>v.y;
	*this>>v.z;
	*this>>v.w;
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(physx::PxBounds3 &v)
{
	*this >> v.minimum;
	*this >> v.maximum;
	return *this;
}

PX_INLINE PsIOStream& PsIOStream::operator>>(const char *&str)
{
	str = NULL; // by default no string streamed...
	if ( mBinary )
	{
		uint32_t len=0;
		*this >> len;

		PX_ASSERT( len < (MAX_STREAM_STRING-1) );
		if ( len < (MAX_STREAM_STRING-1) )
		{
			mStream.read(mReadString,len);
			mReadString[len] = 0;
			str = mReadString;
		}
	}
	return *this;
}


PX_INLINE void  PsIOStream::storeString(const char *c,bool zeroTerminate)
{
	while ( *c )
	{
		mStream.storeByte((uint8_t)*c);
		c++;
	}
	if ( zeroTerminate )
	{
		mStream.storeByte(0);
	}
}
