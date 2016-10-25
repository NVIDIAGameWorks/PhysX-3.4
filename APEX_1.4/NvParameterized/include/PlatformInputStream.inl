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

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

template<typename T> PX_INLINE Serializer::ErrorType PlatformInputStream::read(T &x, bool doAlign)
{
	if( doAlign )
		align(mTargetParams.getAlignment<T>());

	if( mCurParams.getSize<T>() != mTargetParams.getSize<T>() )
	{
		PX_ALWAYS_ASSERT();
		return Serializer::ERROR_INVALID_PLATFORM;
	}

	mStream.read(&x, sizeof(T));

	if( mCurParams.endian != mTargetParams.endian )
		SwapBytes((char *)&x, sizeof(T), GetDataType<T>::value);

	return Serializer::ERROR_NONE;
}

//Deserialize array of primitive type (slow path)
template<typename T> PX_INLINE Serializer::ErrorType PlatformInputStream::readSimpleArraySlow(Handle &handle)
{
	int32_t n;
	handle.getArraySize(n);

	align(mTargetParams.getAlignment<T>());

	for(int32_t i = 0; i < n; ++i)
	{
		handle.set(i);
		T val;
		NV_ERR_CHECK_RETURN( read(val) );
		handle.setParam<T>(val);

		handle.popIndex();
	}

	return Serializer::ERROR_NONE;
}

template<typename T> PX_INLINE Serializer::ErrorType PlatformInputStream::readSimpleArray(Handle &handle)
{
	if( mTargetParams.getSize<T>() == mCurParams.getSize<T>()
			&& mTargetParams.getSize<T>() >= mTargetParams.getAlignment<T>()
			&& mCurParams.getSize<T>() >= mCurParams.getAlignment<T>() ) //No gaps between array elements on both platforms
	{
		//Fast path

		int32_t n;
		handle.getArraySize(n);

		align(mTargetParams.getAlignment<T>());

		const int32_t elemSize = (int32_t)sizeof(T);

		if( mStream.tellRead() + elemSize * n >= mStream.getFileLength() )
		{
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_INTERNAL_PTR;
		}

		PX_ASSERT(elemSize * n >= 0);
		char *p = (char *)mTraits->alloc(static_cast<uint32_t>(elemSize * n));
		mStream.read(p, static_cast<uint32_t>(elemSize * n));

		if( mCurParams.endian != mTargetParams.endian )
		{
			char *elem = p;
			for(int32_t i = 0; i < n; ++i)
			{
				SwapBytes(elem, elemSize, GetDataType<T>::value);
				elem += elemSize;
			}
		}

		handle.setParamArray<T>((const T *)p, n);

		mTraits->free(p);
	}
	else
	{
		return readSimpleArraySlow<T>(handle);
	}

	return Serializer::ERROR_NONE;
}

