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


#ifndef PX_PHYSICS_SC_OBJECT_ID_TRACKER
#define PX_PHYSICS_SC_OBJECT_ID_TRACKER

#include "CmPhysXCommon.h"
#include "CmIDPool.h"
#include "CmBitMap.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Sc
{

	class ObjectIDTracker : public Ps::UserAllocated
	{
		PX_NOCOPY(ObjectIDTracker)
	public:
		ObjectIDTracker() : mPendingReleasedIDs(PX_DEBUG_EXP("objectIDTrackerIDs")) {}

		PX_INLINE PxU32			createID()						{ return mIDPool.getNewID();								}
		PX_INLINE void			releaseID(PxU32 id)				
		{ 
			markIDAsDeleted(id); 
			mPendingReleasedIDs.pushBack(id);	
		}
		PX_INLINE Ps::IntBool	isDeletedID(PxU32 id)	const	{ return mDeletedIDsMap.boundedTest(id);					}
		PX_FORCE_INLINE PxU32	getDeletedIDCount()		const	{ return mPendingReleasedIDs.size();						}
		PX_INLINE void			clearDeletedIDMap()				{ mDeletedIDsMap.clear();									}
		PX_INLINE void			resizeDeletedIDMap(PxU32 id, PxU32 numIds)	
		{ 
			mDeletedIDsMap.resize(id); 
			mPendingReleasedIDs.reserve(numIds);
		}
		PX_INLINE void			processPendingReleases()
		{
			for(PxU32 i=0; i < mPendingReleasedIDs.size(); i++)
			{
				 mIDPool.freeID(mPendingReleasedIDs[i]);
			}
			mPendingReleasedIDs.clear();
		}
		PX_INLINE void reset()
		{
			processPendingReleases();
			mPendingReleasedIDs.reset();

			// Don't free stuff in IDPool, we still need the list of free IDs

			// And it does not seem worth freeing the memory of the bitmap
		}

		PX_INLINE PxU32 getMaxID()
		{
			return mIDPool.getMaxID();
		}
	private:
		PX_INLINE void markIDAsDeleted(PxU32 id) { PX_ASSERT(!isDeletedID(id)); mDeletedIDsMap.growAndSet(id); }


	private:
		Cm::IDPool					mIDPool;
		Cm::BitMap					mDeletedIDsMap;
		Ps::Array<PxU32>			mPendingReleasedIDs;  // Buffer for released IDs to make sure newly created objects do not re-use these IDs immediately
	};

}
}

#endif
