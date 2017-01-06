/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Contains code for box pruning.
 *	\file		IceBoxPruning.h
 *	\author		Pierre Terdiman
 *	\date		January, 29, 2000
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Include Guard
#ifndef __ICEBOXPRUNING_BASE_H__
#define __ICEBOXPRUNING_BASE_H__

//#include "vector"
#include <PsArray.h>
#include "IceRevisitedRadixBase.h"
#include "PxVec3.h"
#include "PxBounds3.h"
#include <PsUserAllocated.h>

namespace nvidia
{
namespace fracture
{
namespace base
{

	struct Axes
	{
		void set(uint32_t a0, uint32_t a1, uint32_t a2) {
			Axis0 = a0; Axis1 = a1; Axis2 = a2; 
		}
		uint32_t	Axis0;
		uint32_t	Axis1;
		uint32_t	Axis2;
	};

	class BoxPruning : public UserAllocated 
	{
	public:
		// Optimized versions
		bool completeBoxPruning(const nvidia::Array<PxBounds3> &bounds, nvidia::Array<uint32_t> &pairs, const Axes& axes);
		bool bipartiteBoxPruning(const nvidia::Array<PxBounds3> &bounds0, const nvidia::Array<PxBounds3> &bounds1, nvidia::Array<uint32_t>& pairs, const Axes& axes);

		// Brute-force versions
		bool bruteForceCompleteBoxTest(const nvidia::Array<PxBounds3> &bounds, nvidia::Array<uint32_t> &pairs, const Axes& axes);
		bool bruteForceBipartiteBoxTest(const nvidia::Array<PxBounds3> &bounds0, const nvidia::Array<PxBounds3> &bounds1, nvidia::Array<uint32_t>& pairs, const Axes& axes);

	protected:
		nvidia::Array<float> mMinPosBounds0;
		nvidia::Array<float> mMinPosBounds1;
		nvidia::Array<float> mPosList;
		RadixSort mRS0, mRS1;	
		RadixSort mRS;
	};

}
}
}

#endif // __ICEBOXPRUNING_H__
#endif
