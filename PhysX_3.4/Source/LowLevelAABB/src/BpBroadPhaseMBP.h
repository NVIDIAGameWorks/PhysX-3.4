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

#ifndef BP_BROADPHASE_MBP_H
#define BP_BROADPHASE_MBP_H

#include "CmPhysXCommon.h"
#include "BpBroadPhase.h"
#include "BpBroadPhaseMBPCommon.h"
#include "PsUserAllocated.h"
#include "BpMBPTasks.h"

	class MBP;
	

namespace physx
{

namespace Bp
{
	class BPDefaultMemoryAllocator;

	class BroadPhaseMBP : public BroadPhase, public Ps::UserAllocated
	{
											PX_NOCOPY(BroadPhaseMBP)
		public:
											BroadPhaseMBP(PxU32 maxNbRegions,
															PxU32 maxNbBroadPhaseOverlaps,
															PxU32 maxNbStaticShapes,
															PxU32 maxNbDynamicShapes,
															PxU64 contextID
															);
		virtual								~BroadPhaseMBP();

	// BroadPhaseBase
		virtual	bool						getCaps(PxBroadPhaseCaps& caps)														const;
		virtual	PxU32						getNbRegions()																		const;
		virtual	PxU32						getRegions(PxBroadPhaseRegionInfo* userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;
		virtual	PxU32						addRegion(const PxBroadPhaseRegion& region, bool populateRegion);
		virtual	bool						removeRegion(PxU32 handle);
		virtual	PxU32						getNbOutOfBoundsObjects()	const;
		virtual	const PxU32*				getOutOfBoundsObjects()		const;
	//~BroadPhaseBase

	// BroadPhase
		virtual	PxBroadPhaseType::Enum		getType()					const	{ return PxBroadPhaseType::eMBP;	}

		virtual	void						destroy();

		virtual	void						update(const PxU32 numCpuTasks, PxcScratchAllocator* scratchAllocator, const BroadPhaseUpdateData& updateData, physx::PxBaseTask* continuation, physx::PxBaseTask* narrowPhaseUnblockTask);
		virtual void						fetchBroadPhaseResults(physx::PxBaseTask*) {}

		virtual	PxU32						getNbCreatedPairs()		const;
		virtual BroadPhasePairReport*		getCreatedPairs();
		virtual PxU32						getNbDeletedPairs()		const;
		virtual BroadPhasePairReport*		getDeletedPairs();

		virtual void						freeBuffers();

		virtual void						shiftOrigin(const PxVec3& shift);

#if PX_CHECKED
		virtual bool						isValid(const BroadPhaseUpdateData& updateData)	const;
#endif

		virtual BroadPhasePair*				getBroadPhasePairs() const  {return NULL;}  //KS - TODO - implement this!!!

		virtual void						deletePairs(){}								//KS - TODO - implement this!!!

		
	//~BroadPhase

				MBPUpdateWorkTask			mMBPUpdateWorkTask;
				MBPPostUpdateWorkTask		mMBPPostUpdateWorkTask;

				MBP*						mMBP;		// PT: TODO: aggregate

				MBP_Handle*					mMapping;
				PxU32						mCapacity;
				Ps::Array<BroadPhasePairReport>	mCreated;
				Ps::Array<BroadPhasePairReport>	mDeleted;

				const BpHandle*				mGroups;	// ### why are those 'handles'?

				void						setUpdateData(const BroadPhaseUpdateData& updateData);
				void						update(physx::PxBaseTask* continuation);
				void						postUpdate(physx::PxBaseTask* continuation);
				void						allocateMappingArray(PxU32 newCapacity);
	};

} //namespace Bp

} //namespace physx

#endif // BP_BROADPHASE_MBP_H
