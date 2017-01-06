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
#include "DyThresholdTable.h"
#include "PsHash.h"
#include "PsUtilities.h"
#include "PsAllocator.h"

namespace physx
{
	namespace Dy
	{
		bool ThresholdTable::check(const ThresholdStream& stream, const PxU32 nodeIndexA, const PxU32 nodeIndexB, PxReal dt)
		{
			PxU32* PX_RESTRICT hashes = mHash;
			PxU32* PX_RESTRICT nextIndices = mNexts;
			Pair* PX_RESTRICT pairs = mPairs;

			/*const PxsRigidBody* b0 = PxMin(body0, body1);
			const PxsRigidBody* b1 = PxMax(body0, body1);*/

			const PxU32 nA = PxMin(nodeIndexA, nodeIndexB);
			const PxU32 nB = PxMax(nodeIndexA, nodeIndexB);

			PxU32 hashKey = computeHashKey(nodeIndexA, nodeIndexB, mHashSize);

			PxU32 pairIndex = hashes[hashKey];
			while(NO_INDEX != pairIndex)
			{
				Pair& pair = pairs[pairIndex];
				const PxU32 thresholdStreamIndex = pair.thresholdStreamIndex;
				PX_ASSERT(thresholdStreamIndex < stream.size());
				const ThresholdStreamElement& otherElement = stream[thresholdStreamIndex];
				if(otherElement.nodeIndexA==nA && otherElement.nodeIndexB==nB)
					return (pair.accumulatedForce > (otherElement.threshold * dt));
				pairIndex = nextIndices[pairIndex];
			}
			return false;
		}
	}
}
