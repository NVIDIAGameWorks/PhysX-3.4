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

#ifndef PX_TRAITS_INTERNAL_H
#define PX_TRAITS_INTERNAL_H

#include "nvparameterized/NvParameterizedTraits.h"
#include "PsString.h"

namespace NvParameterized
{

PX_PUSH_PACK_DEFAULT

#ifndef WITHOUT_APEX_SERIALIZATION
#ifdef _MSC_VER
#pragma warning(disable:4127) // conditional expression is constant
#endif
#define NV_PARAM_TRAITS_WARNING(_traits, _format, ...) do { \
		char _tmp[256]; \
		physx::shdfnd::snprintf(_tmp, sizeof(_tmp), _format, ##__VA_ARGS__); \
		_traits->traitsWarn(_tmp); \
	} while(0)

#else
#define NV_PARAM_TRAITS_WARNING(...)
#endif

// Determines how default converter handles included references
struct RefConversionMode
{
	enum Enum
	{
		// Simply move it to new object (and remove from old)
		REF_CONVERT_COPY = 0,

		// Same as REF_CONVERT_COPY but also update legacy references
		REF_CONVERT_UPDATE,

		// Skip references
		REF_CONVERT_SKIP,

		REF_CONVERT_LAST
	};
};

// Specifies preferred version for element at position specified in longName
struct PrefVer
{
	const char *longName;
	uint32_t ver;
};

// A factory function to create an instance of default conversion.
Conversion *internalCreateDefaultConversion(Traits *traits, const PrefVer *prefVers = 0, RefConversionMode::Enum refMode = RefConversionMode::REF_CONVERT_COPY);

PX_POP_PACK

}

#endif