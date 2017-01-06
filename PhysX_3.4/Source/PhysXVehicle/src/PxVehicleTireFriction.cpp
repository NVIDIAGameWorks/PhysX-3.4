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

#include "foundation/PxMemory.h"
#include "PxVehicleTireFriction.h"
#include "CmPhysXCommon.h"
#include "PsFoundation.h"

namespace physx
{

PX_FORCE_INLINE PxU32 computeByteSize(const PxU32 maxNbTireTypes, const PxU32 maxNbSurfaceTypes)
{
	PxU32 byteSize = ((sizeof(PxU32)*(maxNbTireTypes*maxNbSurfaceTypes) + 15) & ~15);
	byteSize += ((sizeof(PxMaterial*)*maxNbSurfaceTypes + 15) & ~15);
	byteSize += ((sizeof(PxVehicleDrivableSurfaceType)*maxNbSurfaceTypes + 15) & ~15);
	byteSize += ((sizeof(PxVehicleDrivableSurfaceToTireFrictionPairs) + 15) & ~ 15);
	return byteSize;
}

PxVehicleDrivableSurfaceToTireFrictionPairs* PxVehicleDrivableSurfaceToTireFrictionPairs::allocate
(const PxU32 maxNbTireTypes, const PxU32 maxNbSurfaceTypes)
{
	PX_CHECK_AND_RETURN_VAL(maxNbSurfaceTypes <= eMAX_NB_SURFACE_TYPES, "maxNbSurfaceTypes must be less than eMAX_NB_SURFACE_TYPES", NULL);

	PxU32 byteSize = computeByteSize(maxNbTireTypes, maxNbSurfaceTypes);
	PxU8* ptr = static_cast<PxU8*>(PX_ALLOC(byteSize, "PxVehicleDrivableSurfaceToTireFrictionPairs"));
	PxMemSet(ptr, 0, byteSize);
	PxVehicleDrivableSurfaceToTireFrictionPairs* pairs = reinterpret_cast<PxVehicleDrivableSurfaceToTireFrictionPairs*>(ptr);

	pairs->mPairs = NULL;
	pairs->mDrivableSurfaceMaterials = NULL;
	pairs->mDrivableSurfaceTypes = NULL;
	pairs->mNbTireTypes = 0;
	pairs->mMaxNbTireTypes = maxNbTireTypes;
	pairs->mNbSurfaceTypes = 0;
	pairs->mMaxNbSurfaceTypes = maxNbSurfaceTypes;

	return pairs;
}

void PxVehicleDrivableSurfaceToTireFrictionPairs::setup
(const PxU32 numTireTypes, const PxU32 numSurfaceTypes, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes)
{
	PX_CHECK_AND_RETURN(numTireTypes <= mMaxNbTireTypes, "numTireTypes must be less than mMaxNumSurfaceTypes");
	PX_CHECK_AND_RETURN(numSurfaceTypes <= mMaxNbSurfaceTypes, "numSurfaceTypes must be less than mMaxNumSurfaceTypes");

	PxU8* ptr = reinterpret_cast<PxU8*>(this);

	const PxU32 maxNbTireTypes = mMaxNbTireTypes;
	const PxU32 maxNbSurfaceTypes = mMaxNbSurfaceTypes;
	PxU32 byteSize = computeByteSize(mMaxNbTireTypes, mMaxNbSurfaceTypes);
	PxMemSet(ptr, 0, byteSize);
	mMaxNbTireTypes = maxNbTireTypes;
	mMaxNbSurfaceTypes = maxNbSurfaceTypes;

	PxVehicleDrivableSurfaceToTireFrictionPairs* pairs = reinterpret_cast<PxVehicleDrivableSurfaceToTireFrictionPairs*>(ptr);
	ptr += ((sizeof(PxVehicleDrivableSurfaceToTireFrictionPairs) + 15) & ~ 15);

	mPairs = reinterpret_cast<PxReal*>(ptr);
	ptr += ((sizeof(PxU32)*(numTireTypes*numSurfaceTypes) + 15) & ~15);
	mDrivableSurfaceMaterials = reinterpret_cast<const PxMaterial**>(ptr);
	ptr += ((sizeof(PxMaterial*)*numSurfaceTypes + 15) & ~15);
	mDrivableSurfaceTypes = reinterpret_cast<PxVehicleDrivableSurfaceType*>(ptr);
	ptr += ((sizeof(PxVehicleDrivableSurfaceType)*numSurfaceTypes +15) & ~15);

	for(PxU32 i=0;i<numSurfaceTypes;i++)
	{
		mDrivableSurfaceTypes[i] = drivableSurfaceTypes[i];
		mDrivableSurfaceMaterials[i] = drivableSurfaceMaterials[i];
	}
	for(PxU32 i=0;i<numTireTypes*numSurfaceTypes;i++)
	{
		mPairs[i]=1.0f;
	}

	pairs->mNbTireTypes=numTireTypes;
	pairs->mNbSurfaceTypes=numSurfaceTypes;
}

void PxVehicleDrivableSurfaceToTireFrictionPairs::release()
{
	PX_FREE(this);
}

void PxVehicleDrivableSurfaceToTireFrictionPairs::setTypePairFriction(const PxU32 surfaceType, const PxU32 tireType, const PxReal value)
{
	PX_CHECK_AND_RETURN(tireType<mNbTireTypes, "Invalid tireType");
	PX_CHECK_AND_RETURN(surfaceType<mNbSurfaceTypes, "Invalid surfaceType");

	*(mPairs + mNbTireTypes*surfaceType + tireType) = value;
}

PxReal PxVehicleDrivableSurfaceToTireFrictionPairs::getTypePairFriction(const PxU32 surfaceType, const PxU32 tireType) const 
{
	PX_CHECK_AND_RETURN_VAL(tireType<mNbTireTypes, "Invalid tireType", 0.0f);
	PX_CHECK_AND_RETURN_VAL(surfaceType<mNbSurfaceTypes, "Invalid surfaceType", 0.0f);

	return *(mPairs + mNbTireTypes*surfaceType + tireType);
}




}//physx

