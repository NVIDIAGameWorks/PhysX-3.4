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


#ifndef PX_PHYSICS_SCENEQUERYMANAGER
#define PX_PHYSICS_SCENEQUERYMANAGER
/** \addtogroup physics 
@{ */

#include "PxSceneDesc.h"
#include "CmBitMap.h"
#include "PsArray.h"
#include "SqPruner.h"
#include "PsMutex.h"
#include "PxActor.h" // needed for offset table
#include "ScbActor.h" // needed for offset table
// threading
#include "PsSync.h"

namespace physx
{

class PxRigidActor;
class NpShape;
class NpBatchQuery;

namespace Scb
{
	class Scene;
	class Shape;
}

namespace Sc
{
	class ActorCore;
}

namespace Sq
{
	typedef size_t PrunerData;
	#define SQ_INVALID_PRUNER_DATA	0xffffffff

	struct ActorShape;
	struct PrunerPayload;
	class Pruner;

	struct OffsetTable
	{
		PX_FORCE_INLINE	OffsetTable() {}
		PX_FORCE_INLINE const Scb::Actor&		convertPxActor2Scb(const PxActor& actor) const		{ return *Ps::pointerOffset<const Scb::Actor*>(&actor, pxActorToScbActor[actor.getConcreteType()]); }
		PX_FORCE_INLINE const Sc::ActorCore&	convertScbActor2Sc(const Scb::Actor& actor) const	{ return *Ps::pointerOffset<const Sc::ActorCore*>(&actor, scbToSc[actor.getScbType()]); }

		ptrdiff_t pxActorToScbActor[PxConcreteType::ePHYSX_CORE_COUNT];
		ptrdiff_t scbToSc[ScbType::eTYPE_COUNT];
	};
	extern OffsetTable gOffsetTable;

	// PT: extended pruner structure. We might want to move the additional data to the pruner itself later.
	struct PrunerExt
	{
														PrunerExt();
														~PrunerExt();

						void							init(PxPruningStructureType::Enum type, PxU64 contextID);
						void							flushMemory();
						void							preallocate(PxU32 nbShapes);
						void							flushShapes(PxU32 index);

						void							addToDirtyList(PrunerHandle handle);
						Ps::IntBool						isDirty(PrunerHandle handle)	const;
						void							removeFromDirtyList(PrunerHandle handle);
						void							growDirtyList(PrunerHandle handle);

		PX_FORCE_INLINE	PxPruningStructureType::Enum	type()			const	{ return mPrunerType;	}
		PX_FORCE_INLINE	const Pruner*					pruner()		const	{ return mPruner;		}
		PX_FORCE_INLINE	Pruner*							pruner()				{ return mPruner;		}
		PX_FORCE_INLINE PxU32							timestamp()		const	{ return mTimestamp;	}
		PX_FORCE_INLINE void							invalidateTimestamp()	{ mTimestamp++;			}

		private:
						Pruner*							mPruner;
						Cm::BitMap						mDirtyMap;
						Ps::Array<PrunerHandle>			mDirtyList;
						PxPruningStructureType::Enum	mPrunerType;
						PxU32							mTimestamp;

						PX_NOCOPY(PrunerExt)

		friend class SceneQueryManager;
	};

	struct DynamicBoundsSync : public Sc::SqBoundsSync
	{
		virtual void sync(const PrunerHandle* handles, const PxU32* indices, const PxBounds3* bounds, PxU32 count);
		Pruner*	mPruner;
		PxU32*	mTimestamp;
	};

	class SceneQueryManager : public Ps::UserAllocated
	{
		PX_NOCOPY(SceneQueryManager)
	public:
														SceneQueryManager(Scb::Scene& scene, PxPruningStructureType::Enum staticStructure, 
															PxPruningStructureType::Enum dynamicStructure, PxU32 dynamicTreeRebuildRateHint,
															const PxSceneLimits& limits);
														~SceneQueryManager();

						PrunerData						addPrunerShape(const NpShape& shape, const PxRigidActor& actor, bool dynamic, const PxBounds3* bounds=NULL, bool hasPrunerStructure = false);
						void							removePrunerShape(PrunerData shapeData);
						const PrunerPayload&			getPayload(PrunerData shapeData) const;

	public:
		PX_FORCE_INLINE	Scb::Scene&						getScene()						const	{ return mScene;			}
		PX_FORCE_INLINE	PxU32							getDynamicTreeRebuildRateHint()	const	{ return mRebuildRateHint;	}

		PX_FORCE_INLINE	const PrunerExt&				get(PruningIndex::Enum index)	const	{ return mPrunerExt[index];	}
		PX_FORCE_INLINE	PrunerExt&						get(PruningIndex::Enum index)			{ return mPrunerExt[index];	}

						void							preallocate(PxU32 staticShapes, PxU32 dynamicShapes);
						void							markForUpdate(PrunerData s);
						void							setDynamicTreeRebuildRateHint(PxU32 dynTreeRebuildRateHint);
						
						void							flushUpdates();
						void							forceDynamicTreeRebuild(bool rebuildStaticStructure, bool rebuildDynamicStructure);

						DynamicBoundsSync&				getDynamicBoundsSync()					{ return mDynamicBoundsSync; }

		// Force a rebuild of the aabb/loose octree etc to allow raycasting on multiple threads.
						void							processSimUpdates();
						void							validateSimUpdates();
						void							afterSync(bool commit);
						void							shiftOrigin(const PxVec3& shift);

						void							flushMemory();
	private:
						PrunerExt						mPrunerExt[PruningIndex::eCOUNT];

						PxU32							mRebuildRateHint;

						Scb::Scene&						mScene;

						// threading
						shdfnd::Mutex					mSceneQueryLock;  // to make sure only one query updates the dirty pruner structure if multiple queries run in parallel

						DynamicBoundsSync				mDynamicBoundsSync;

						void							flushShapes();
	};

	///////////////////////////////////////////////////////////////////////////////

	// PT: TODO: replace PrunerData with just PxU32 to save memory on Win64. Breaks binary compatibility though.
	// PT: was previously called 'ActorShape' but does not contain an actor or shape pointer, contrary to the Np-level struct with the same name.
	// PT: it only contains a pruner index (0 or 1) and a pruner handle. Hence the new name.
	PX_FORCE_INLINE PrunerData createPrunerData(PxU32 index, PrunerHandle h)	{ return PrunerData((h << 1) | index);	}
	PX_FORCE_INLINE PxU32 getPrunerIndex(PrunerData data)						{ return PxU32(data & 1);				}
	PX_FORCE_INLINE PrunerHandle getPrunerHandle(PrunerData data)				{ return PrunerHandle(data >> 1);		}

	///////////////////////////////////////////////////////////////////////////////


} // namespace Sq

}

/** @} */
#endif
