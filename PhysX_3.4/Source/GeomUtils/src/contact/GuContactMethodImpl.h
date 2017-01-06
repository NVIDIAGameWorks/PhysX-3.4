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

#ifndef GU_CONTACTMETHODIMPL_H
#define GU_CONTACTMETHODIMPL_H

#include "foundation/PxAssert.h"
#include "PxPhysXCommonConfig.h"
#include "CmPhysXCommon.h"
#include "collision/PxCollisionDefs.h"

namespace physx
{
namespace Cm
{
	class RenderOutput;
}

namespace Gu
{
	class GeometryUnion;
	class ContactBuffer;
	struct NarrowPhaseParams;
	class PersistentContactManifold;
	class MultiplePersistentContactManifold;

	enum ManifoldFlags
	{
		IS_MANIFOLD			= (1<<0),
		IS_MULTI_MANIFOLD	= (1<<1)
	};

	struct Cache : public PxCache
	{
		Cache()
		{
		}

		PX_FORCE_INLINE void setManifold(void* manifold)
		{
			PX_ASSERT((size_t(manifold) & 0xF) == 0);
			mCachedData = reinterpret_cast<PxU8*>(manifold);
			mManifoldFlags |= IS_MANIFOLD;
		}

		PX_FORCE_INLINE void setMultiManifold(void* manifold)
		{
			PX_ASSERT((size_t(manifold) & 0xF) == 0);
			mCachedData = reinterpret_cast<PxU8*>(manifold);
			mManifoldFlags |= IS_MANIFOLD|IS_MULTI_MANIFOLD;
		}

		PX_FORCE_INLINE PxU8 isManifold()	const
		{
			return PxU8(mManifoldFlags & IS_MANIFOLD);
		}

		PX_FORCE_INLINE PxU8 isMultiManifold()	const
		{
			return PxU8(mManifoldFlags & IS_MULTI_MANIFOLD);
		}

		PX_FORCE_INLINE PersistentContactManifold& getManifold()
		{
			PX_ASSERT(isManifold());
			PX_ASSERT(!isMultiManifold());
			PX_ASSERT((uintptr_t(mCachedData) & 0xf) == 0);
			return *reinterpret_cast<PersistentContactManifold*>(mCachedData);
		}

		PX_FORCE_INLINE MultiplePersistentContactManifold& getMultipleManifold()
		{
			PX_ASSERT(isManifold());
			PX_ASSERT(isMultiManifold());
			PX_ASSERT((uintptr_t(mCachedData) & 0xf) == 0);
			return *reinterpret_cast<MultiplePersistentContactManifold*>(mCachedData);
		}
	};
}

#define GU_CONTACT_METHOD_ARGS				\
	const Gu::GeometryUnion& shape0,		\
	const Gu::GeometryUnion& shape1,		\
	const PxTransform& transform0,			\
	const PxTransform& transform1,			\
	const Gu::NarrowPhaseParams& params,	\
	Gu::Cache& cache,						\
	Gu::ContactBuffer& contactBuffer,		\
	Cm::RenderOutput* renderOutput

namespace Gu
{
	PX_PHYSX_COMMON_API bool contactSphereSphere(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactSphereCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactSphereBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactBoxBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactBoxConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactConvexConvex(GU_CONTACT_METHOD_ARGS);
	
	PX_PHYSX_COMMON_API bool contactSphereMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactBoxMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactConvexMesh(GU_CONTACT_METHOD_ARGS);

	PX_PHYSX_COMMON_API bool contactSphereHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactCapsuleHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactBoxHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactConvexHeightfield(GU_CONTACT_METHOD_ARGS);
	
	PX_PHYSX_COMMON_API bool contactSpherePlane(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactPlaneBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactPlaneCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool contactPlaneConvex(GU_CONTACT_METHOD_ARGS);

	//Legacy heightfield code path
	PX_PHYSX_COMMON_API bool legacyContactSphereHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool legacyContactCapsuleHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool legacyContactBoxHeightfield(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool legacyContactConvexHeightfield(GU_CONTACT_METHOD_ARGS);

	PX_PHYSX_COMMON_API bool pcmContactSphereMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactConvexMesh(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxHeightField(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactConvexHeightField(GU_CONTACT_METHOD_ARGS);

	PX_PHYSX_COMMON_API bool pcmContactPlaneCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactPlaneBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactPlaneConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereSphere(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSpherePlane(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactSphereConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleCapsule(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactCapsuleConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxBox(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactBoxConvex(GU_CONTACT_METHOD_ARGS);
	PX_PHYSX_COMMON_API bool pcmContactConvexConvex(GU_CONTACT_METHOD_ARGS);
}
}

#endif
