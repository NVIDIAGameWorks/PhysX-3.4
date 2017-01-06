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


#include "foundation/PxMemory.h"
#include "ScSimStats.h"
#include "PxvSimStats.h"

using namespace physx;

static const PxU32 sBroadphaseAddRemoveSize = sizeof(PxU32) * PxSimulationStatistics::eVOLUME_COUNT;

Sc::SimStats::SimStats()
{
	PxMemZero(&numBroadPhaseAdds, sBroadphaseAddRemoveSize);
	PxMemZero(&numBroadPhaseRemoves, sBroadphaseAddRemoveSize);

	clear();
}


void Sc::SimStats::clear()
{
#if PX_ENABLE_SIM_STATS
	PxMemZero(const_cast<void*>(reinterpret_cast<volatile void*>(&numTriggerPairs)), sizeof(TriggerPairCounts));
	PxMemZero(&numBroadPhaseAddsPending, sBroadphaseAddRemoveSize);
	PxMemZero(&numBroadPhaseRemovesPending, sBroadphaseAddRemoveSize);
#endif
}


void Sc::SimStats::simStart()
{
#if PX_ENABLE_SIM_STATS
	// pending broadphase adds/removes are now the current ones
	PxMemMove(numBroadPhaseAdds, numBroadPhaseAddsPending, sBroadphaseAddRemoveSize);
	PxMemMove(numBroadPhaseRemoves, numBroadPhaseRemovesPending, sBroadphaseAddRemoveSize);
	clear();
#endif
}


void Sc::SimStats::readOut(PxSimulationStatistics& s, const PxvSimStats& simStats) const
{
#if PX_ENABLE_SIM_STATS
	s = PxSimulationStatistics();  // clear stats

	for(PxU32 i=0; i < PxGeometryType::eCONVEXMESH+1; i++)
	{
		for(PxU32 j=0; j < PxGeometryType::eGEOMETRY_COUNT; j++)
		{
			s.nbTriggerPairs[i][j] += PxU32(numTriggerPairs[i][j]);
			if (i != j)
				s.nbTriggerPairs[j][i] += PxU32(numTriggerPairs[i][j]);
		}
	}

	for(PxU32 i=0; i < PxSimulationStatistics::eVOLUME_COUNT; i++)
	{
		s.nbBroadPhaseAdds[i] = numBroadPhaseAdds[i];
		s.nbBroadPhaseRemoves[i] = numBroadPhaseRemoves[i];
	}

	for(PxU32 i=0; i < PxGeometryType::eGEOMETRY_COUNT; i++)
	{
		s.nbDiscreteContactPairs[i][i] = simStats.mNbDiscreteContactPairs[i][i];
		s.nbModifiedContactPairs[i][i] = simStats.mNbModifiedContactPairs[i][i];
		s.nbCCDPairs[i][i] = simStats.mNbCCDPairs[i][i];

		for(PxU32 j=i+1; j < PxGeometryType::eGEOMETRY_COUNT; j++)
		{
			PxU32 c = simStats.mNbDiscreteContactPairs[i][j];
			s.nbDiscreteContactPairs[i][j] = c;
			s.nbDiscreteContactPairs[j][i] = c;

			c = simStats.mNbModifiedContactPairs[i][j];
			s.nbModifiedContactPairs[i][j] = c;
			s.nbModifiedContactPairs[j][i] = c;

			c = simStats.mNbCCDPairs[i][j];
			s.nbCCDPairs[i][j] = c;
			s.nbCCDPairs[j][i] = c;
		}
#if PX_DEBUG
		for(PxU32 j=0; j < i; j++)
		{
			// PxvSimStats should only use one half of the matrix
			PX_ASSERT(simStats.mNbDiscreteContactPairs[i][j] == 0);
			PX_ASSERT(simStats.mNbModifiedContactPairs[i][j] == 0);
			PX_ASSERT(simStats.mNbCCDPairs[i][j] == 0);
		}
#endif
	}

	s.nbDiscreteContactPairsTotal = simStats.mNbDiscreteContactPairsTotal;
	s.nbDiscreteContactPairsWithCacheHits = simStats.mNbDiscreteContactPairsWithCacheHits;
	s.nbDiscreteContactPairsWithContacts = simStats.mNbDiscreteContactPairsWithContacts;
	s.nbActiveConstraints = simStats.mNbActiveConstraints;
	s.nbActiveDynamicBodies = simStats.mNbActiveDynamicBodies;
	s.nbActiveKinematicBodies = simStats.mNbActiveKinematicBodies;

	s.nbAxisSolverConstraints = simStats.mNbAxisSolverConstraints;

	s.peakConstraintMemory = simStats.mPeakConstraintBlockAllocations * 16 * 1024;
	s.compressedContactSize = simStats.mTotalCompressedContactSize;
	s.requiredContactConstraintMemory = simStats.mTotalConstraintSize;
	s.nbNewPairs = simStats.mNbNewPairs;
	s.nbLostPairs = simStats.mNbLostPairs;
	s.nbNewTouches = simStats.mNbNewTouches;
	s.nbLostTouches = simStats.mNbLostTouches;
	s.nbPartitions = simStats.mNbPartitions;

#else
	PX_UNUSED(s);
	PX_UNUSED(simStats);
#endif
}
