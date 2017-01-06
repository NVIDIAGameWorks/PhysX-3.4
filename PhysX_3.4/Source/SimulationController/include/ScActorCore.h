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


#ifndef PX_COLLISION_ACTOR_CORE
#define PX_COLLISION_ACTOR_CORE

#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PxMetaData.h"
#include "PxActor.h"

namespace physx
{

class PxActor;

namespace Sc
{

	class Scene;
	class ActorSim;

	class ActorCore : public Ps::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
													ActorCore(const PxEMPTY) :	mSim(NULL), mActorFlags(PxEmpty)
													{
													}
		static			void						getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
													ActorCore(PxActorType::Enum actorType, PxU8 actorFlags, 
															  PxClientID owner, PxU8 behavior, PxDominanceGroup dominanceGroup);
		/*virtual*/									~ActorCore();

		PX_FORCE_INLINE	ActorSim*					getSim()						const	{ return mSim;							}
		PX_FORCE_INLINE	void						setSim(ActorSim* sim)
													{
														PX_ASSERT((sim==NULL) ^ (mSim==NULL));
														mSim = sim;
													}

		PX_FORCE_INLINE	PxActorFlags				getActorFlags()					const	{ return mActorFlags;					}
						void						setActorFlags(PxActorFlags af);

		PX_FORCE_INLINE	PxDominanceGroup			getDominanceGroup()				const
													{
														return PxDominanceGroup(mDominanceGroup);	
													}
						void						setDominanceGroup(PxDominanceGroup g);

		PX_FORCE_INLINE	void						setOwnerClient(PxClientID inId)
													{
														const PxU32 aggid = mAggregateIDOwnerClient & 0x00ffffff;
														mAggregateIDOwnerClient = (PxU32(inId)<<24) | aggid;

													}
		PX_FORCE_INLINE	PxClientID					getOwnerClient()				const
													{
														return mAggregateIDOwnerClient>>24;
													}

		PX_FORCE_INLINE	PxActorClientBehaviorFlags	getClientBehaviorFlags()		const	{ return mClientBehaviorFlags;			}
		PX_FORCE_INLINE	void						setClientBehaviorFlags(PxActorClientBehaviorFlags b)	{ mClientBehaviorFlags = b;	}

		PX_FORCE_INLINE	PxActorType::Enum			getActorCoreType()				const 	{ return PxActorType::Enum(mActorType);	}

						void						reinsertShapes();
// PX_AGGREGATE
		PX_FORCE_INLINE	void						setAggregateID(PxU32 id)
													{
														PX_ASSERT(id==0xffffffff || id<(1<<24));
														const PxU32 ownerClient = mAggregateIDOwnerClient & 0xff000000;
														mAggregateIDOwnerClient = (id & 0x00ffffff) | ownerClient;
													}
		PX_FORCE_INLINE	PxU32						getAggregateID()				const
													{
														const PxU32 id = mAggregateIDOwnerClient & 0x00ffffff;
														return id == 0x00ffffff ? PX_INVALID_U32 : id;
													}
//~PX_AGGREGATE
	private:
						ActorSim*					mSim;						// 
						PxU32						mAggregateIDOwnerClient;	// PxClientID (8bit) | aggregate ID (24bit)
		// PT: TODO: the remaining members could be packed into just a 16bit mask
						PxActorFlags				mActorFlags;				// PxActor's flags (PxU8) => only 4 bits used
						PxU8						mActorType;					// Actor type (8 bits, but 3 would be enough)
						PxActorClientBehaviorFlags	mClientBehaviorFlags;		// PxU8 => only 4 bits used
						PxU8						mDominanceGroup;			// Dominance group (8 bits, but 5 would be enough because "must be < 32")
	};

#if PX_P64_FAMILY
	PX_COMPILE_TIME_ASSERT(sizeof(Sc::ActorCore)==16);
#else
	PX_COMPILE_TIME_ASSERT(sizeof(Sc::ActorCore)==12);
#endif

} // namespace Sc

}

//////////////////////////////////////////////////////////////////////////

#endif
