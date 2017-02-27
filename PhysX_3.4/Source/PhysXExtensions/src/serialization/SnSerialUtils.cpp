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
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "SnSerialUtils.h"
#include "PsString.h"
#include "PxSerialization.h"
#include "PxPhysicsVersion.h"
#include "PsBasicTemplates.h"

using namespace physx;

namespace
{

#define SN_NUM_BINARY_PLATFORMS 13
const PxU32 sBinaryPlatformTags[SN_NUM_BINARY_PLATFORMS] =
{
	PX_MAKE_FOURCC('W','_','3','2'),
	PX_MAKE_FOURCC('W','_','6','4'),
	PX_MAKE_FOURCC('L','_','3','2'),
	PX_MAKE_FOURCC('L','_','6','4'),
	PX_MAKE_FOURCC('M','_','3','2'),
	PX_MAKE_FOURCC('M','_','6','4'),
	PX_MAKE_FOURCC('M','O','C','A'),
	PX_MAKE_FOURCC('A','N','D','R'),
	PX_MAKE_FOURCC('A','I','O','S'),
	PX_MAKE_FOURCC('A','A','6','4'),
	PX_MAKE_FOURCC('X','O','N','E'),
	PX_MAKE_FOURCC('N','X','3','2'),
	PX_MAKE_FOURCC('N','X','6','4')
};

const char* sBinaryPlatformNames[SN_NUM_BINARY_PLATFORMS] =
{
	"win32",
	"win64",
	"linux32",
	"linux64",
	"macOSX32",
	"macOSX64",
	"ps4",
	"android",
	"ios",
	"ios64",
	"xboxone",
	"nx32",
	"nx64"
};

#define SN_NUM_BINARY_COMPATIBLE_VERSIONS 1

//
// Important: if you adjust the following structure, please adjust the comment for PX_BINARY_SERIAL_VERSION as well
//
const Ps::Pair<PxU32, PxU32> sBinaryCompatibleVersions[SN_NUM_BINARY_COMPATIBLE_VERSIONS] =
{
	Ps::Pair<PxU32, PxU32>(PX_PHYSICS_VERSION, PX_BINARY_SERIAL_VERSION)	
};

}

namespace physx { namespace Sn {

PxU32 getBinaryPlatformTag()
{
#if PX_WINDOWS && PX_X86
	return sBinaryPlatformTags[0];
#elif PX_WINDOWS && PX_X64
	return sBinaryPlatformTags[1];
#elif PX_LINUX && (PX_X86 || PX_ARM)
	return sBinaryPlatformTags[2];
#elif PX_LINUX && (PX_X64 || PX_A64)
	return sBinaryPlatformTags[3];
#elif PX_OSX && PX_X86
	return sBinaryPlatformTags[4];
#elif PX_OSX && PX_X64
	return sBinaryPlatformTags[5];
#elif PX_PS4
	return sBinaryPlatformTags[6];
#elif PX_ANDROID
	return sBinaryPlatformTags[7];
#elif PX_IOS && PX_ARM
	return sBinaryPlatformTags[8];
#elif PX_IOS && PX_A64
	return sBinaryPlatformTags[9];
#elif PX_XBOXONE
	return sBinaryPlatformTags[10];
#elif PX_NX && !PX_A64
	return sBinaryPlatformTags[11];
#elif PX_NX && PX_A64
	return sBinaryPlatformTags[12];
#else
	#error Unknown binary platform
#endif
}

bool isBinaryPlatformTagValid(physx::PxU32 platformTag)
{
	PxU32 platformIndex = 0;
	while (platformIndex < SN_NUM_BINARY_PLATFORMS && platformTag != sBinaryPlatformTags[platformIndex]) platformIndex++;
	return platformIndex < SN_NUM_BINARY_PLATFORMS;
}

const char* getBinaryPlatformName(physx::PxU32 platformTag)
{
	PxU32 platformIndex = 0;
	while (platformIndex < SN_NUM_BINARY_PLATFORMS && platformTag != sBinaryPlatformTags[platformIndex]) platformIndex++;
	return (platformIndex == SN_NUM_BINARY_PLATFORMS) ? "unknown" : sBinaryPlatformNames[platformIndex];
}

bool checkCompatibility(const PxU32 version, const PxU32 binaryVersion)
{		
	for(PxU32 i =0; i<SN_NUM_BINARY_COMPATIBLE_VERSIONS; i++)
	{
		if(version == sBinaryCompatibleVersions[i].first && binaryVersion == sBinaryCompatibleVersions[i].second)
			return true;
	}
	return false;
}

void getCompatibilityVersionsStr(char* buffer, PxU32 lenght)
{
	size_t len = 0;
	for(PxU32 i =0; i<SN_NUM_BINARY_COMPATIBLE_VERSIONS; i++)
	{
		physx::shdfnd::snprintf(buffer + len,  lenght - len, "%x-%d\n", sBinaryCompatibleVersions[i].first, sBinaryCompatibleVersions[i].second);	
		len = strlen(buffer);
	}	
}

} // Sn
} // physx

