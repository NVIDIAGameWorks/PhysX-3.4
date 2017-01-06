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

#ifndef CCT_OBSTACLE_CONTEXT
#define CCT_OBSTACLE_CONTEXT

/* Exclude from documentation */
/** \cond */

#include "characterkinematic/PxControllerObstacles.h"
#include "PsUserAllocated.h"
#include "PsArray.h"
#include "CmPhysXCommon.h"

namespace physx
{
	struct PxRaycastHit;

namespace Cct
{
	class CharacterControllerManager;

    typedef PxU32  Handle;
	class HandleManager : public Ps::UserAllocated
	{
		public:
									HandleManager();
									~HandleManager();

						Handle		Add(void* object);
						void		Remove(Handle handle);
						void*		GetObject(Handle handle)	const;	// Returns object according to handle.
						bool		UpdateObject(Handle handle, void* object);

		PX_FORCE_INLINE	PxU32		GetMaxNbObjects()			const	{ return mMaxNbObjects;		}	//!< Returns max number of objects
		PX_FORCE_INLINE	PxU32		GetNbObjects()				const	{ return mCurrentNbObjects;	}	//!< Returns current number of objects
		PX_FORCE_INLINE	void**		GetObjects()				const	{ return mObjects;			}	//!< Gets the complete list of objects
		PX_FORCE_INLINE	void*		PickObject(Handle handle)	const	{ return mObjects[mOutToIn[PxU16(handle)]]; }

		private:
		// Physical list
						void**		mObjects;			//!< Physical list, with no holes but unsorted.
						PxU32		mCurrentNbObjects;	//!< Current number of objects in the physical list.
						PxU32		mMaxNbObjects;		//!< Maximum possible number of objects in the physical list.

		// Cross-references
						PxU16*		mOutToIn;			//!< Maps virtual indices (handles) to real ones.
						PxU16*		mInToOut;			//!< Maps real indices to virtual ones (handles).
						PxU16*      mStamps;

		// Recycled locations
						PxU32		mNbFreeIndices;		//!< Current number of free indices

		// Internal methods
						bool		SetupLists(void** objects=NULL, PxU16* oti=NULL, PxU16* ito=NULL, PxU16* stamps=NULL);
	};

	class ObstacleContext : public PxObstacleContext, public Ps::UserAllocated
	{
		public:
												ObstacleContext(CharacterControllerManager& );
		virtual									~ObstacleContext();

		// PxObstacleContext
		virtual	void							release();
		virtual PxControllerManager&			getControllerManager() const;
		virtual	ObstacleHandle					addObstacle(const PxObstacle& obstacle);
		virtual	bool							removeObstacle(ObstacleHandle handle);
		virtual	bool							updateObstacle(ObstacleHandle handle, const PxObstacle& obstacle);
		virtual	PxU32							getNbObstacles()		const;
		virtual	const PxObstacle*				getObstacle(PxU32 i)	const;
		virtual	const PxObstacle*				getObstacleByHandle(ObstacleHandle handle)	const;
		//~PxObstacleContext

				const PxObstacle*				raycastSingle(PxRaycastHit& hit, const PxVec3& origin, const PxVec3& unitDir, const PxReal distance, ObstacleHandle& obstacleHandle)	const;
				const PxObstacle*				raycastSingle(PxRaycastHit& hit, const ObstacleHandle& obstacleHandle, const PxVec3& origin, const PxVec3& unitDir, const PxReal distance)	const; // raycast just one obstacle handle

				void							onOriginShift(const PxVec3& shift);

				struct InternalBoxObstacle
				{
					InternalBoxObstacle(ObstacleHandle handle, const PxBoxObstacle& data) : mHandle(handle), mData(data)	{}

					ObstacleHandle	mHandle;
					PxBoxObstacle	mData;
				};
				Ps::Array<InternalBoxObstacle>	mBoxObstacles;

				struct InternalCapsuleObstacle
				{
					InternalCapsuleObstacle(ObstacleHandle handle, const PxCapsuleObstacle& data) : mHandle(handle), mData(data)	{}

					ObstacleHandle		mHandle;
					PxCapsuleObstacle	mData;
				};
				Ps::Array<InternalCapsuleObstacle>	mCapsuleObstacles;

	private:
				ObstacleContext&				operator=(const ObstacleContext&);
				HandleManager					mHandleManager;
				CharacterControllerManager&		mCCTManager;
	};


} // namespace Cct

}

/** \endcond */
#endif
