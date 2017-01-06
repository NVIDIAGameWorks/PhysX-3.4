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

#include "foundation/PxMat33.h"
#include "foundation/PxQuat.h"
#include "foundation/PxTransform.h"
#include "PsUtilities.h"
#include "PsUserAllocated.h"
#include "PsFPU.h"

namespace physx
{
namespace shdfnd
{

bool checkValid(const float& f)
{
	return PxIsFinite(f);
}
bool checkValid(const PxVec3& v)
{
	return PxIsFinite(v.x) && PxIsFinite(v.y) && PxIsFinite(v.z);
}

bool checkValid(const PxTransform& t)
{
	return checkValid(t.p) && checkValid(t.q);
}

bool checkValid(const PxQuat& q)
{
	return PxIsFinite(q.x) && PxIsFinite(q.y) && PxIsFinite(q.z) && PxIsFinite(q.w);
}
bool checkValid(const PxMat33& m)
{
	return PxIsFinite(m(0, 0)) && PxIsFinite(m(1, 0)) && PxIsFinite(m(2, 0)) && PxIsFinite(m(0, 1)) &&
	       PxIsFinite(m(1, 1)) && PxIsFinite(m(2, 1)) && PxIsFinite(m(0, 3)) && PxIsFinite(m(1, 3)) &&
	       PxIsFinite(m(2, 3));
}
bool checkValid(const char* string)
{
	static const PxU32 maxLength = 4096;
	return strnlen(string, maxLength) != maxLength;
}

} // namespace shdfnd
} // namespace physx
