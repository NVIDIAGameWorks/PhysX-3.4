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


#ifndef PX_COLLISION_ACTORPAIR
#define PX_COLLISION_ACTORPAIR

#include "ScRigidSim.h"
#include "ScContactStream.h"
#include "ScNPhaseCore.h"

namespace physx
{
namespace Sc
{

	class ActorPairContactReportData
	{
	public:
		ActorPairContactReportData() : 
			mStrmResetStamp						(0xffffffff),
			mActorAID							(0xffffffff),
			mActorBID							(0xffffffff),
			mPxActorA							(NULL),
			mPxActorB							(NULL),
			mActorAClientID						(0xff),
			mActorBClientID						(0xff),
			mActorAClientBehavior				(0),
			mActorBClientBehavior				(0)
			{}

		ContactStreamManager	mContactStreamManager;
		PxU32					mStrmResetStamp;
		PxU32					mActorAID;
		PxU32					mActorBID;
		PxActor*				mPxActorA;
		PxActor*				mPxActorB;
		PxClientID				mActorAClientID;
		PxClientID				mActorBClientID;
		PxU8					mActorAClientBehavior;
		PxU8					mActorBClientBehavior;
	};


	/**
	\brief Class shared by all shape interactions for a pair of actors.

	This base class is used if no shape pair of an actor pair has contact reports requested.
	*/
	class ActorPair
	{
	public:

		enum ActorPairFlags
		{
			eIS_REPORT_PAIR							= (1<<0),
			eNEXT_FREE								= (1<<1)
		};

		PX_FORCE_INLINE					ActorPair() : mInternalFlags(0), mTouchCount(0), mRefCount(0) {}
		PX_FORCE_INLINE					~ActorPair() {}

		PX_FORCE_INLINE	Ps::IntBool		isReportPair() const { return (mInternalFlags & eIS_REPORT_PAIR); }

		PX_FORCE_INLINE	void			incTouchCount() { mTouchCount++; PX_ASSERT(mTouchCount); }
		PX_FORCE_INLINE	void			decTouchCount() { PX_ASSERT(mTouchCount); mTouchCount--; }
		PX_FORCE_INLINE	PxU32			getTouchCount() const { return mTouchCount; }

		PX_FORCE_INLINE	void			incRefCount() { ++mRefCount; PX_ASSERT(mRefCount>0); }
		PX_FORCE_INLINE	PxU32			decRefCount() { PX_ASSERT(mRefCount>0); return --mRefCount; }
		PX_FORCE_INLINE	PxU32			getRefCount() const { return mRefCount; }

	private:
		ActorPair& operator=(const ActorPair&);


	protected:
						PxU16			mInternalFlags;
						PxU16			mTouchCount;
						PxU16			mRefCount;
						PxU16			mPad;  // instances of this class are stored in a pool which needs an item size of at least size_t
	};


	/**
	\brief Class shared by all shape interactions for a pair of actors if contact reports are requested.

	This class is used if at least one shape pair of an actor pair has contact reports requested.

	\note If a pair of actors had contact reports requested for some of the shape interactions but all of them switch to not wanting contact reports
	any longer, then the ActorPairReport instance is kept being used and won't get replaced by a simpler ActorPair instance.
	*/
	class ActorPairReport : public ActorPair
	{
	public:

		enum ActorPairReportFlags
		{
			eIS_IN_CONTACT_REPORT_ACTOR_PAIR_SET = ActorPair::eNEXT_FREE	// PT: whether the pair is already stored in the 'ContactReportActorPairSet' or not
		};

		PX_FORCE_INLINE					ActorPairReport(RigidSim&, RigidSim&);
		PX_FORCE_INLINE					~ActorPairReport();

		PX_INLINE ContactStreamManager&	createContactStreamManager(NPhaseCore&);
		PX_FORCE_INLINE ContactStreamManager& getContactStreamManager() const { PX_ASSERT(mReportData); return mReportData->mContactStreamManager; }
		PX_FORCE_INLINE	RigidSim&		getActorA() const { return mActorA; }
		PX_FORCE_INLINE	RigidSim&		getActorB() const { return mActorB; }
		PX_INLINE		PxU32			getActorAID() const { PX_ASSERT(mReportData); return mReportData->mActorAID; }
		PX_INLINE		PxU32			getActorBID() const { PX_ASSERT(mReportData); return mReportData->mActorBID; }
		PX_INLINE		PxActor*		getPxActorA() const { PX_ASSERT(mReportData); return mReportData->mPxActorA; }
		PX_INLINE		PxActor*		getPxActorB() const { PX_ASSERT(mReportData); return mReportData->mPxActorB; }
		PX_INLINE		PxClientID		getActorAClientID() const { PX_ASSERT(mReportData); return mReportData->mActorAClientID; }
		PX_INLINE		PxClientID		getActorBClientID() const { PX_ASSERT(mReportData); return mReportData->mActorBClientID; }
		PX_INLINE		PxU8			getActorAClientBehavior() const { PX_ASSERT(mReportData); return mReportData->mActorAClientBehavior; }
		PX_INLINE		PxU8			getActorBClientBehavior() const { PX_ASSERT(mReportData); return mReportData->mActorBClientBehavior; }
		PX_FORCE_INLINE	bool			streamResetNeeded(PxU32 cmpStamp) const;
		PX_INLINE		bool			streamResetStamp(PxU32 cmpStamp);

		PX_FORCE_INLINE	PxU16			isInContactReportActorPairSet() const { return PxU16(mInternalFlags & eIS_IN_CONTACT_REPORT_ACTOR_PAIR_SET); }
		PX_FORCE_INLINE	void			setInContactReportActorPairSet() { mInternalFlags |= eIS_IN_CONTACT_REPORT_ACTOR_PAIR_SET; }
		PX_FORCE_INLINE	void			clearInContactReportActorPairSet() { mInternalFlags &= ~eIS_IN_CONTACT_REPORT_ACTOR_PAIR_SET; }

		PX_FORCE_INLINE void			createContactReportData(NPhaseCore&);
		PX_FORCE_INLINE void			releaseContactReportData(NPhaseCore&);
		PX_FORCE_INLINE const ActorPairContactReportData* hasReportData() const { return mReportData; }

		PX_FORCE_INLINE	void			convert(ActorPair& aPair) { PX_ASSERT(!aPair.isReportPair()); mTouchCount = PxU16(aPair.getTouchCount()); mRefCount = PxU16(aPair.getRefCount()); }

		PX_FORCE_INLINE static ActorPairReport& cast(ActorPair& aPair) { PX_ASSERT(aPair.isReportPair()); return static_cast<ActorPairReport&>(aPair); }

	private:
		ActorPairReport& operator=(const ActorPairReport&);

						RigidSim&		mActorA;
						RigidSim&		mActorB;

			ActorPairContactReportData* mReportData;
	};

} // namespace Sc


PX_FORCE_INLINE Sc::ActorPairReport::ActorPairReport(RigidSim& actor0, RigidSim& actor1) : ActorPair(),
mActorA			(actor0),
mActorB			(actor1),
mReportData		(NULL)
{
	PX_ASSERT(mInternalFlags == 0);
	mInternalFlags = ActorPair::eIS_REPORT_PAIR;
}


PX_FORCE_INLINE Sc::ActorPairReport::~ActorPairReport()
{
	PX_ASSERT(mReportData == NULL);
}


PX_FORCE_INLINE bool Sc::ActorPairReport::streamResetNeeded(PxU32 cmpStamp) const
{
	return (cmpStamp != mReportData->mStrmResetStamp);
}


PX_INLINE bool Sc::ActorPairReport::streamResetStamp(PxU32 cmpStamp) 
{
	PX_ASSERT(mReportData);
	const bool ret = streamResetNeeded(cmpStamp);
	mReportData->mStrmResetStamp = cmpStamp; 
	return ret; 
}


PX_INLINE Sc::ContactStreamManager&	Sc::ActorPairReport::createContactStreamManager(NPhaseCore& npCore)
{
	// Lazy create report data
	if(!mReportData)
		createContactReportData(npCore);

	return mReportData->mContactStreamManager;
}


PX_FORCE_INLINE void Sc::ActorPairReport::createContactReportData(NPhaseCore& npCore)
{
	PX_ASSERT(!mReportData);
	Sc::ActorPairContactReportData* reportData = npCore.createActorPairContactReportData(); 
	mReportData = reportData;

	if(reportData)
	{
		reportData->mActorAID = mActorA.getID();
		reportData->mActorBID = mActorB.getID();

		reportData->mPxActorA = mActorA.getPxActor();
		reportData->mPxActorB = mActorB.getPxActor();

		const ActorCore& actorCoreA = mActorA.getActorCore();
		const ActorCore& actorCoreB = mActorB.getActorCore();

		reportData->mActorAClientID = actorCoreA.getOwnerClient();
		reportData->mActorBClientID = actorCoreB.getOwnerClient();

		reportData->mActorAClientBehavior = actorCoreA.getClientBehaviorFlags();
		reportData->mActorBClientBehavior = actorCoreB.getClientBehaviorFlags();
	}
}


PX_FORCE_INLINE void Sc::ActorPairReport::releaseContactReportData(NPhaseCore& npCore)
{
	// Can't take the NPhaseCore (scene) reference from the actors since they're already gone on scene release

	if (mReportData != NULL)
	{
		npCore.releaseActorPairContactReportData(mReportData);
		mReportData = NULL;
	}
}


}

#endif
