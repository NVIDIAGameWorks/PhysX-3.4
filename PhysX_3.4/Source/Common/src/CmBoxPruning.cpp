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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "CmBoxPruning.h"
#include "CmRadixSortBuffered.h"
#include "PsAllocator.h"

using namespace physx;
using namespace Gu;
using namespace Cm;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Bipartite box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to a different set.
 *	\param		nb0		[in] number of boxes in the first set
 *	\param		bounds0	[in] list of boxes for the first set
 *	\param		nb1		[in] number of boxes in the second set
 *	\param		bounds1	[in] list of boxes for the second set
 *	\param		pairs	[out] list of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Cm::BipartiteBoxPruning(const PxBounds3* bounds0, PxU32 nb0, const PxBounds3* bounds1, PxU32 nb1, Ps::Array<PxU32>& pairs, const Axes& axes)
{
	pairs.clear();
	// Checkings
	if(nb0 == 0 || nb1 == 0)
		return false;

	// Catch axes
	PxU32 Axis0 = axes.mAxis0;
	PxU32 Axis1 = axes.mAxis1;
	PxU32 Axis2 = axes.mAxis2;

	PX_UNUSED(Axis1);
	PX_UNUSED(Axis2);

	// Allocate some temporary data
	float* MinPosBounds0 = reinterpret_cast<float*>(PX_ALLOC_TEMP(sizeof(float)*nb0, "Gu::BipartiteBoxPruning"));
	float* MinPosBounds1 = reinterpret_cast<float*>(PX_ALLOC_TEMP(sizeof(float)*nb1, "Gu::BipartiteBoxPruning"));

	// 1) Build main lists using the primary axis
	for(PxU32 i=0;i<nb0;i++)	MinPosBounds0[i] = bounds0[i].minimum[Axis0];
	for(PxU32 i=0;i<nb1;i++)	MinPosBounds1[i] = bounds1[i].minimum[Axis0];

	// 2) Sort the lists
	//static RadixSort RS0, RS1;	// Static for coherence. Crashes on exit
	RadixSortBuffered RS0, RS1;	// Static for coherence.

	const PxU32* Sorted0 = RS0.Sort(MinPosBounds0, nb0).GetRanks();
	const PxU32* Sorted1 = RS1.Sort(MinPosBounds1, nb1).GetRanks();

	// 3) Prune the lists
	PxU32 Index0, Index1;

	const PxU32* const LastSorted0 = &Sorted0[nb0];
	const PxU32* const LastSorted1 = &Sorted1[nb1];
	const PxU32* RunningAddress0 = Sorted0;
	const PxU32* RunningAddress1 = Sorted1;

	while(RunningAddress1<LastSorted1 && Sorted0<LastSorted0)
	{
		Index0 = *Sorted0++;

		while(RunningAddress1<LastSorted1 && MinPosBounds1[*RunningAddress1]<MinPosBounds0[Index0])	RunningAddress1++;

		const PxU32* RunningAddress2_1 = RunningAddress1;

		while(RunningAddress2_1<LastSorted1 && MinPosBounds1[Index1 = *RunningAddress2_1++]<=bounds0[Index0].maximum[Axis0])
		{
			if(bounds0[Index0].intersects(bounds1[Index1]))
			{
				pairs.pushBack(Index0);
				pairs.pushBack(Index1);
			}
		}
	}

	////

	while(RunningAddress0<LastSorted0 && Sorted1<LastSorted1)
	{
		Index0 = *Sorted1++;

		while(RunningAddress0<LastSorted0 && MinPosBounds0[*RunningAddress0]<=MinPosBounds1[Index0])	RunningAddress0++;

		const PxU32* RunningAddress2_0 = RunningAddress0;

		while(RunningAddress2_0<LastSorted0 && MinPosBounds0[Index1 = *RunningAddress2_0++]<=bounds1[Index0].maximum[Axis0])
		{
			if(bounds0[Index1].intersects(bounds1[Index0]))
			{
				pairs.pushBack(Index1);
				pairs.pushBack(Index0);
			}
		}
	}

	PX_FREE(MinPosBounds1);
	PX_FREE(MinPosBounds0);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Complete box pruning. Returns a list of overlapping pairs of boxes, each box of the pair belongs to the same set.
 *	\param		bounds	[in] list of boxes
 *	\param		nb		[in] number of boxes
 *	\param		pairs	[out] list of overlapping pairs
 *	\param		axes	[in] projection order (0,2,1 is often best)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Cm::CompleteBoxPruning(const PxBounds3* bounds, PxU32 nb, Ps::Array<PxU32>& pairs, const Axes& axes)
{
	pairs.clear();

	// Checkings
	if(!nb)
		return false;

	// Catch axes
	const PxU32 Axis0 = axes.mAxis0;
	const PxU32 Axis1 = axes.mAxis1;
	const PxU32 Axis2 = axes.mAxis2;

	PX_UNUSED(Axis1);
	PX_UNUSED(Axis2);

	// Allocate some temporary data
	float* PosList = reinterpret_cast<float*>(PX_ALLOC_TEMP(sizeof(float)*nb, "Cm::CompleteBoxPruning"));

	// 1) Build main list using the primary axis
	for(PxU32 i=0;i<nb;i++)	PosList[i] = bounds[i].minimum[Axis0];

	// 2) Sort the list
	/*static*/ RadixSortBuffered RS;	// Static for coherence
	const PxU32* Sorted = RS.Sort(PosList, nb).GetRanks();

	// 3) Prune the list
	const PxU32* const LastSorted = &Sorted[nb];
	const PxU32* RunningAddress = Sorted;
	PxU32 Index0, Index1;
	while(RunningAddress<LastSorted && Sorted<LastSorted)
	{
		Index0 = *Sorted++;

		while(RunningAddress<LastSorted && PosList[*RunningAddress++]<PosList[Index0]);

		const PxU32* RunningAddress2 = RunningAddress;

		while(RunningAddress2<LastSorted && PosList[Index1 = *RunningAddress2++]<=bounds[Index0].maximum[Axis0])
		{
			if(Index0!=Index1)
			{
				if(bounds[Index0].intersects(bounds[Index1]))
				{
					pairs.pushBack(Index0);
					pairs.pushBack(Index1);
				}
			}
		}
	}

	PX_FREE(PosList);

	return true;
}

