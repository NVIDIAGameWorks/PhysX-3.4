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

#ifndef SERIALIZER_COMMON_H
#define SERIALIZER_COMMON_H

#include <stdio.h> // FILE

#include "PxAssert.h"

#include "PsArray.h"
#include "PsHashMap.h"

#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "nvparameterized/NvSerializer.h"

#define ENABLE_DEBUG_ASSERTS 1

#if ENABLE_DEBUG_ASSERTS
#	define DEBUG_ASSERT(x) PX_ASSERT(x)
#else
#	define DEBUG_ASSERT(x)
#endif
#define DEBUG_ALWAYS_ASSERT() DEBUG_ASSERT(0)

#define NV_ERR_CHECK_RETURN(x) { Serializer::ErrorType err = x; if( Serializer::ERROR_NONE != err ) { DEBUG_ALWAYS_ASSERT(); return err; } }
#define NV_BOOL_ERR_CHECK_RETURN(x, err) { if( !(x) ) { DEBUG_ALWAYS_ASSERT(); return err; } }
#define NV_PARAM_ERR_CHECK_RETURN(x, err) { if( NvParameterized::ERROR_NONE != (NvParameterized::ErrorType)(x) ) { DEBUG_ALWAYS_ASSERT(); return err; } }

#define NV_ERR_CHECK_WARN_RETURN(x, ...) { \
	Serializer::ErrorType err = x; \
	if( Serializer::ERROR_NONE != err ) { \
		NV_PARAM_TRAITS_WARNING(mTraits, ##__VA_ARGS__); \
		DEBUG_ALWAYS_ASSERT(); \
		return err; \
	} \
}

#define NV_BOOL_ERR_CHECK_WARN_RETURN(x, err, ...) { \
	if( !(x) ) { \
		NV_PARAM_TRAITS_WARNING(mTraits, ##__VA_ARGS__); \
		DEBUG_ALWAYS_ASSERT(); \
		return err; \
	} \
}

#define NV_PARAM_ERR_CHECK_WARN_RETURN(x, err, ...) { \
	if( NvParameterized::ERROR_NONE != (NvParameterized::ErrorType)(x) ) \
	{ \
		NV_PARAM_TRAITS_WARNING(mTraits, ##__VA_ARGS__); \
		DEBUG_ALWAYS_ASSERT(); \
		return err; \
	} \
}

namespace NvParameterized
{

bool UpgradeLegacyObjects(Serializer::DeserializedData &data, bool &isUpdated, Traits *t);
Interface *UpgradeObject(Interface &obj, bool &isUpdated, Traits *t);

//This is used for releasing resources (I wish we had generic smart ptrs...)
class Releaser
{
	FILE *mFile;

	void *mBuf;
	Traits *mTraits;

	Interface *mObj;

	Releaser(const Releaser &) {}

public:

	Releaser()
	{
		reset();
	}

	Releaser(void *buf, Traits *traits)
	{
		reset(buf, traits);
	}

	Releaser(FILE *file)
	{
		reset(file);
	}

	Releaser(Interface *obj)
	{
		reset(obj);
	}

	void reset()
	{
		mFile = 0;
		mBuf = 0;
		mTraits = 0;
		mObj = 0;
	}

	void reset(Interface *obj)
	{
		reset();

		mObj = obj;
	}

	void reset(FILE *file)
	{
		reset();

		mFile = file;
	}

	void reset(void *buf, Traits *traits)
	{
		reset();

		mBuf = buf;
		mTraits = traits;
	}

	~Releaser()
	{
		if( mBuf )
			mTraits->free(mBuf);

		if( mFile )
			fclose(mFile);

		if( mObj )
			mObj->destroy();
	}
};

void *serializerMemAlloc(uint32_t size, Traits *traits);
void serializerMemFree(void *mem, Traits *traits);

// Checksum for some classes is invalid
bool DoIgnoreChecksum(const Interface &obj);

} // namespace NvParameterized

#endif
