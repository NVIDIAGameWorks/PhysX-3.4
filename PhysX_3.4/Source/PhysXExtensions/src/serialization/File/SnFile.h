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


#ifndef SN_FILE_H
#define SN_FILE_H

// fopen_s - returns 0 on success, non-zero on failure

#if PX_MICROSOFT_FAMILY

#include <stdio.h>

namespace physx
{
namespace sn
{
PX_INLINE PxI32 fopen_s(FILE** file, const char* name, const char* mode)
{
	static const PxU32 MAX_LEN = 300; 
	char buf[MAX_LEN+1];

	PxU32 i;
	for(i = 0; i<MAX_LEN && name[i]; i++)
		buf[i] = name[i] == '/' ? '\\' : name[i];
	buf[i] = 0;

	return i == MAX_LEN ? -1 : ::fopen_s(file, buf, mode);
};

} // namespace sn
} // namespace physx

#elif PX_UNIX_FAMILY || PX_PS4 || PX_NX

#include <stdio.h>

namespace physx
{
namespace sn
{
PX_INLINE PxI32 fopen_s(FILE** file, const char* name, const char* mode)
{
	FILE* fp = ::fopen(name, mode);
	if(fp)
	{
		*file = fp;
		return PxI32(0);
	}
	return -1;
}
} // namespace sn
} // namespace physx
#else
#error "Platform not supported!"
#endif

#endif //SN_FILE_H

