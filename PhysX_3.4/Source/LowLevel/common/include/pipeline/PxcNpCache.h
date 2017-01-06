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

#ifndef PXC_NPCACHE_H
#define PXC_NPCACHE_H

#include "foundation/PxMemory.h"

#include "PsIntrinsics.h"
#include "PxcNpCacheStreamPair.h"

#include "PsPool.h"
#include "PsFoundation.h"
#include "GuContactMethodImpl.h"
#include "PsUtilities.h"

namespace physx
{

template <typename T>
void PxcNpCacheWrite(PxcNpCacheStreamPair& streams,
					 Gu::Cache& cache,
					 const T& payload,
					 PxU32 bytes, 
					 const PxU8* data)
{
	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	cache.mCachedSize = Ps::to16((payloadSize + 4 + bytes + 0xF)&~0xF);

	PxU8* ls = streams.reserve(cache.mCachedSize);
	cache.mCachedData = ls;
	if(ls==NULL || (reinterpret_cast<PxU8*>(-1))==ls)
	{
		if(ls==NULL)
		{
			PX_WARN_ONCE(
				"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
				"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
			return;
		}
		else
		{
			PX_WARN_ONCE(
				"Attempting to allocate more than 16K of contact data for a single contact pair in narrowphase. "
				"Either accept dropped contacts or simplify collision geometry.");
			cache.mCachedData = NULL;
			ls = NULL;
			return;
		}
	}

	*reinterpret_cast<T*>(ls) = payload;
	*reinterpret_cast<PxU32*>(ls+payloadSize) = bytes;
	if(data)
		PxMemCopy(ls+payloadSize+sizeof(PxU32), data, bytes);
}


template <typename T>
PxU8* PxcNpCacheWriteInitiate(PxcNpCacheStreamPair& streams, Gu::Cache& cache, const T& payload, PxU32 bytes)
{
	PX_UNUSED(payload);

	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	cache.mCachedSize = Ps::to16((payloadSize + 4 + bytes + 0xF)&~0xF);

	PxU8* ls = streams.reserve(cache.mCachedSize);
	cache.mCachedData = ls;
	if(NULL==ls || reinterpret_cast<PxU8*>(-1)==ls)
	{
		if(NULL==ls)
		{
			PX_WARN_ONCE(
				"Reached limit set by PxSceneDesc::maxNbContactDataBlocks - ran out of buffer space for narrow phase. "
				"Either accept dropped contacts or increase buffer size allocated for narrow phase by increasing PxSceneDesc::maxNbContactDataBlocks.");
		}
		else
		{
			PX_WARN_ONCE(
				"Attempting to allocate more than 16K of contact data for a single contact pair in narrowphase. "
				"Either accept dropped contacts or simplify collision geometry.");
			cache.mCachedData = NULL;
			ls = NULL;
		}
	}
	return ls;
}

template <typename T>
PX_FORCE_INLINE void PxcNpCacheWriteFinalize(PxU8* ls, const T& payload, PxU32 bytes, const PxU8* data)
{
	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	*reinterpret_cast<T*>(ls) = payload;
	*reinterpret_cast<PxU32*>(ls+payloadSize) = bytes;
	if(data)
		PxMemCopy(ls+payloadSize+sizeof(PxU32), data, bytes);
}


template <typename T>
PX_FORCE_INLINE PxU8* PxcNpCacheRead(Gu::Cache& cache, T*& payload)
{
	PxU8* ls = cache.mCachedData;
	payload = reinterpret_cast<T*>(ls);
	const PxU32 payloadSize = (sizeof(T)+3)&~3;
	return reinterpret_cast<PxU8*>(ls+payloadSize+sizeof(PxU32));
}

template <typename T>
const PxU8* PxcNpCacheRead2(Gu::Cache& cache, T& payload, PxU32& bytes)
{
	const PxU8* ls = cache.mCachedData;
	if(ls==NULL)
	{
		bytes = 0;
		return NULL;
	}

	const PxU32 payloadSize = (sizeof(payload)+3)&~3;
	payload = *reinterpret_cast<const T*>(ls);
	bytes = *reinterpret_cast<const PxU32*>(ls+payloadSize);
	PX_ASSERT(cache.mCachedSize == ((payloadSize + 4 + bytes+0xF)&~0xF));
	return reinterpret_cast<const PxU8*>(ls+payloadSize+sizeof(PxU32));
}

}

#endif // #ifndef PXC_NPCACHE_H
