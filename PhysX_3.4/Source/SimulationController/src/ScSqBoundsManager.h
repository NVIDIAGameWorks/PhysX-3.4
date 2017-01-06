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

#ifndef PX_PHYSICS_SCP_SQ_BOUNDS_MANAGER
#define PX_PHYSICS_SCP_SQ_BOUNDS_MANAGER

#include "CmPhysXCommon.h"
#include "foundation/PxBounds3.h"
#include "PsArray.h"
#include "PsUserAllocated.h"
#include "PsHashSet.h"
//#include "SqPruner.h"

namespace physx
{

namespace Sq
{
typedef PxU32 PrunerHandle;	// PT: we should get this from SqPruner.h but it cannot be included from here
}

namespace Sc
{
struct SqBoundsSync;
struct SqRefFinder;
class ShapeSim;

class SqBoundsManager : public Ps::UserAllocated
{
	PX_NOCOPY(SqBoundsManager)
public:
									SqBoundsManager();

	void							addShape(ShapeSim& shape);
	void							removeShape(ShapeSim& shape);
	void							syncBounds(SqBoundsSync& sync, SqRefFinder& finder, const PxBounds3* bounds, PxU64 contextID);	

private:

	Ps::Array<ShapeSim*>			mShapes;		// 
	Ps::Array<Sq::PrunerHandle>		mRefs;			// SQ pruner references
	Ps::Array<PxU32>				mBoundsIndices;	// indices into the Sc bounds array
	Ps::CoalescedHashSet<ShapeSim*>	mRefless;		// shapesims without references
};
}
}

#endif
