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



#ifndef DY_ARTICULATION_SCALAR_H
#define DY_ARTICULATION_SCALAR_H

// Scalar helpers for articulations

#include "foundation/PxUnionCast.h"
#include "DyArticulationUtils.h"
#include "DySpatial.h"
#include "PsFPU.h"

namespace physx
{

namespace Dy
{

PX_FORCE_INLINE Cm::SpatialVector&	velocityRef(FsData &m, PxU32 i)
{	
	return reinterpret_cast<Cm::SpatialVector&>(getVelocity(m)[i]); 
}

PX_FORCE_INLINE Cm::SpatialVector&	deferredVelRef(FsData &m, PxU32 i)
{	
	return reinterpret_cast<Cm::SpatialVector&>(getDeferredVel(m)[i]); 
}

PX_FORCE_INLINE PxVec3& deferredSZRef(FsData &m, PxU32 i)
{	
	return reinterpret_cast<PxVec3 &>(getDeferredSZ(m)[i]); 
}

PX_FORCE_INLINE const PxVec3& deferredSZ(const FsData &s, PxU32 i) 
{	
	return reinterpret_cast<const PxVec3 &>(getDeferredSZ(s)[i]); 
}

PX_FORCE_INLINE Cm::SpatialVector& deferredZRef(FsData &s)
{
	return unsimdRef(s.deferredZ);
}


PX_FORCE_INLINE const Cm::SpatialVector& deferredZ(const FsData &s)
{
	return unsimdRef(s.deferredZ);
}

PX_FORCE_INLINE const PxVec3& getJointOffset(const FsJointVectors& j) 
{	
	return reinterpret_cast<const PxVec3& >(j.jointOffset);		
}

PX_FORCE_INLINE const PxVec3& getParentOffset(const FsJointVectors& j) 
{	
	return reinterpret_cast<const PxVec3&>(j.parentOffset);		
}




PX_FORCE_INLINE const Cm::SpatialVector* getDSI(const FsRow& row)
{	
	return PxUnionCast<const Cm::SpatialVector*,const Cm::SpatialVectorV*>(row.DSI); //reinterpret_cast<const Cm::SpatialVector*>(row.DSI); 
}

}

}

#endif //DY_ARTICULATION_SCALAR_H
