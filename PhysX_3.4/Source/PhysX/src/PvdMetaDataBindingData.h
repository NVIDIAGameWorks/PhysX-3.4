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
#ifndef PX_META_DATA_PVD_BINDING_DATA_H
#define PX_META_DATA_PVD_BINDING_DATA_H
#if PX_SUPPORT_PVD
#include "foundation/PxSimpleTypes.h"
#include "PsArray.h"
#include "PsHashSet.h"
#include "PsHashMap.h"

namespace physx
{
namespace Vd
{
using namespace physx::shdfnd;

typedef HashSet<const PxRigidActor*> OwnerActorsValueType;
typedef HashMap<const PxShape*, OwnerActorsValueType*> OwnerActorsMap;

struct PvdMetaDataBindingData : public UserAllocated
{
	Array<PxU8> mTempU8Array;
	Array<PxActor*> mActors;
	Array<PxArticulation*> mArticulations;
	Array<PxArticulationLink*> mArticulationLinks;
	HashSet<PxActor*> mSleepingActors;
	OwnerActorsMap mOwnerActorsMap;

	PvdMetaDataBindingData()
	: mTempU8Array(PX_DEBUG_EXP("TempU8Array"))
	, mActors(PX_DEBUG_EXP("PxActor"))
	, mArticulations(PX_DEBUG_EXP("Articulations"))
	, mArticulationLinks(PX_DEBUG_EXP("ArticulationLinks"))
	, mSleepingActors(PX_DEBUG_EXP("SleepingActors"))
	{
	}

	template <typename TDataType>
	TDataType* allocateTemp(PxU32 numItems)
	{
		mTempU8Array.resize(numItems * sizeof(TDataType));
		if(numItems)
			return reinterpret_cast<TDataType*>(mTempU8Array.begin());
		else
			return NULL;
	}

	DataRef<const PxU8> tempToRef()
	{
		return DataRef<const PxU8>(mTempU8Array.begin(), mTempU8Array.size());
	}
};
}
}
#endif // PX_SUPPORT_PVD
#endif
