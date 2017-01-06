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

#ifndef PXC_NPCONTACTPREPSHARED_H
#define PXC_NPCONTACTPREPSHARED_H

namespace physx
{
class PxcNpThreadContext;
struct PxsMaterialInfo;
class PxsMaterialManager;
class PxsConstraintBlockManager;
class PxcConstraintBlockStream;
struct PxsContactManagerOutput;

namespace Gu
{
	struct ContactPoint;
}

static const PxReal PXC_SAME_NORMAL = 0.999f; //Around 6 degrees

PxU32 writeCompressedContact(const Gu::ContactPoint* const PX_RESTRICT contactPoints, const PxU32 numContactPoints, PxcNpThreadContext* threadContext,
									PxU8& writtenContactCount, PxU8*& outContactPatches, PxU8*& outContactPoints, PxU16& compressedContactSize, PxReal*& contactForces, PxU32 contactForceByteSize,
									const PxsMaterialManager* materialManager, bool hasModifiableContacts, bool forceNoResponse, PxsMaterialInfo* PX_RESTRICT pMaterial, PxU8& numPatches,
									PxU32 additionalHeaderSize = 0,  PxsConstraintBlockManager* manager = NULL, PxcConstraintBlockStream* blockStream = NULL, bool insertAveragePoint = false,
									PxcDataStreamPool* pool = NULL, PxcDataStreamPool* patchStreamPool = NULL, PxcDataStreamPool* forcePool = NULL, const bool isMeshType = false);

}

#endif
