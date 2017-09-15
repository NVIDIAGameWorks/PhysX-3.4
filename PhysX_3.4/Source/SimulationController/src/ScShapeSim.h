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


#ifndef PX_PHYSICS_SCP_SHAPESIM
#define PX_PHYSICS_SCP_SHAPESIM

#include "ScElementSim.h"
#include "ScShapeCore.h"
#include "CmPtrTable.h"
#include "ScRigidSim.h"
#include "PxsShapeSim.h"

namespace physx
{

	class PxsTransformCache;
namespace Gu
{
	class TriangleMesh;
	class HeightField;
}

/** Simulation object corresponding to a shape core object. This object is created when
    a ShapeCore object is added to the simulation, and destroyed when it is removed
*/

struct PxsRigidCore;

namespace Sc
{

	class RigidSim;
	class ShapeCore;
	class Scene;
	class BodySim;
	class StaticSim;

	PX_ALIGN_PREFIX(16)
	class ShapeSim : public ElementSim
	{
		ShapeSim &operator=(const ShapeSim &);
	public:

		// passing in a pointer for the shape to output its bounds allows us not to have to compute them twice.
		// A neater way to do this would be to ask the AABB Manager for the bounds after the shape has been 
		// constructed, but there is currently no spec for what the AABBMgr is allowed to do with the bounds, 
		// hence better not to assume anything.

														ShapeSim(RigidSim&, const ShapeCore& core);
		virtual											~ShapeSim();

		// ElementSim
		virtual			void							getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const;
		//~ElementSim

						void							reinsertBroadPhase();

		PX_FORCE_INLINE	const ShapeCore&				getCore()				const	{ return mCore; }

						// TODO: compile time coupling

		PX_INLINE		PxGeometryType::Enum			getGeometryType()		const	{ return mCore.getGeometryType();	}

		// This is just for getting a reference for the user, so we cast away const-ness

		PX_INLINE		PxShape*						getPxShape()			const	{ return const_cast<PxShape*>(mCore.getPxShape());	}
		
		PX_FORCE_INLINE	PxReal							getRestOffset()			const	{ return mCore.getRestOffset();		}
		PX_FORCE_INLINE	PxU32							getFlags()				const	{ return mCore.getFlags();			}
		PX_FORCE_INLINE	PxReal							getContactOffset()		const	{ return mCore.getContactOffset();	}

		PX_FORCE_INLINE	RigidSim&						getRbSim()				const	{ return static_cast<RigidSim&>(getActor());	}
						BodySim*						getBodySim()			const;

						PxsRigidCore&					getPxsRigidCore()		const;
						bool							actorIsDynamic()		const;

						void							onFilterDataChange();
						void							onRestOffsetChange();
						void							onFlagChange(PxShapeFlags oldFlags);
						void							onResetFiltering();
						void							onVolumeOrTransformChange(bool forceBoundsUpdate = false);
						void							onMaterialChange();  // remove when material properties are gone from PxcNpWorkUnit
						void							onContactOffsetChange();
						void							markBoundsForUpdate(bool forceBoundsUpdate, bool isDynamic);

						void							getAbsPoseAligned(PxTransform* PX_RESTRICT globalPose)	const;

		PX_FORCE_INLINE	PxU32							getID()								const	{ return mId;		}
		PX_FORCE_INLINE	PxU32							getTransformCacheID()				const	{ return getElementID();	}

						void							removeFromBroadPhase(bool wakeOnLostTouch);

						void							createSqBounds();
						void							destroySqBounds();

		PX_FORCE_INLINE PxU32							getSqBoundsId()						const	{ return mSqBoundsId; }
		PX_FORCE_INLINE void							setSqBoundsId(PxU32 id)						{ mSqBoundsId = id; }

						void							updateCached(PxU32 transformCacheFlags, Cm::BitMapPinned* shapeChangedMap);
						void							updateCached(PxsTransformCache& transformCache, Bp::BoundsArray& boundsArray);
						void							updateContactDistance(PxReal* contactDistance, const PxReal inflation, const PxVec3 angVel, const PxReal dt, Bp::BoundsArray& boundsArray);
						Ps::IntBool						updateSweptBounds();

		PX_FORCE_INLINE PxsShapeSim&					getLLShapeSim() 							{ return mLLShape; }
	private:
						PxsShapeSim						mLLShape;
						const ShapeCore&				mCore;
						PxU32							mId;
						PxU32							mSqBoundsId;

		PX_FORCE_INLINE	void							internalAddToBroadPhase();
		PX_FORCE_INLINE	void							internalRemoveFromBroadPhase();
						void							initSubsystemsDependingOnElementID();
	}
	PX_ALIGN_SUFFIX(16);

#if !PX_P64_FAMILY
//	PX_COMPILE_TIME_ASSERT(32==sizeof(Sc::ShapeSim)); // after removing bounds from shapes
//	PX_COMPILE_TIME_ASSERT((sizeof(Sc::ShapeSim) % 16) == 0); // aligned mem bounds are better for prefetching
#endif

} // namespace Sc

}

#endif
