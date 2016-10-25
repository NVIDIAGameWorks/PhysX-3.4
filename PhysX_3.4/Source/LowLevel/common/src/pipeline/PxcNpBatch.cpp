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
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PxcNpBatch.h"
#include "PxcNpWorkUnit.h"
#include "PxsContactManager.h"
#include "GuGeometryUnion.h"
#include "PxcContactCache.h"
#include "PxcMaterialMethodImpl.h"
#include "PxcNpContactPrepShared.h"
#include "PxvDynamics.h"			// for PxsBodyCore
#include "PxvGeometry.h"			// for PxsShapeCore
#include "CmFlushPool.h"
#include "CmTask.h"
#include "PxTriangleMesh.h"
#include "PxsMaterialManager.h"
#include "PxsTransformCache.h"
#include "GuPersistentContactManifold.h"
#include "PxsContactManagerState.h"
#include "PsFoundation.h"

using namespace physx;
using namespace Gu;


static void startContacts(PxsContactManagerOutput& output, PxcNpThreadContext& context)
{
	context.mContactBuffer.reset();

	output.contactForces = NULL;
	output.contactPatches = NULL;
	output.contactPoints = NULL;
	output.nbContacts = 0;
	output.nbPatches = 0;
	output.statusFlag = 0;	
}

static void flipContacts(PxcNpThreadContext& threadContext, PxsMaterialInfo* PX_RESTRICT materialInfo)
{
	ContactBuffer& buffer = threadContext.mContactBuffer;
	for(PxU32 i=0; i<buffer.count; ++i)
	{
		Gu::ContactPoint& contactPoint = buffer.contacts[i];
		contactPoint.normal = -contactPoint.normal;
		Ps::swap(materialInfo[i].mMaterialIndex0, materialInfo[i].mMaterialIndex1);
	}
}

static PX_FORCE_INLINE void updateDiscreteContactStats(PxcNpThreadContext& context, PxGeometryType::Enum type0, PxGeometryType::Enum type1)
{
#if PX_ENABLE_SIM_STATS
	PX_ASSERT(type0<=type1);
	context.mDiscreteContactPairs[type0][type1]++;
#endif
}

static bool copyBuffers(PxsContactManagerOutput& cmOutput, Gu::Cache& cache, PxcNpThreadContext& context, const bool useContactCache, const bool isMeshType)
{
	bool ret = false;
	//Copy the contact stream from previous buffer to current buffer...
	PxU32 oldSize = sizeof(PxContact) * cmOutput.nbContacts + sizeof(PxContactPatch)*cmOutput.nbPatches;
	if(oldSize)
	{
		ret = true;
		PxU8* oldPatches = cmOutput.contactPatches;
		PxU8* oldContacts = cmOutput.contactPoints;

		PxU32 forceSize = cmOutput.nbContacts * sizeof(PxReal);

		PxU8* PX_RESTRICT contactPatches = NULL;
		PxU8* PX_RESTRICT contactPoints = NULL;

		PxReal* forceBuffer = NULL;

		bool isOverflown = false;

		//ML: if we are using contactStreamPool, which means we are running the GPU codepath
		if(context.mContactStreamPool)
		{
			const PxU32 patchSize = cmOutput.nbPatches * sizeof(PxContactPatch);
			const PxU32 contactSize = cmOutput.nbContacts * sizeof(PxContact);

			PxU32 index = PxU32(Ps::atomicAdd(&context.mContactStreamPool->mSharedDataIndex, PxI32(contactSize)));
			
			if(context.mContactStreamPool->isOverflown())
			{
				PX_WARN_ONCE("Contact buffer overflow detected, please increase its size in the scene desc!\n");
				isOverflown = true;
			}
			contactPoints = context.mContactStreamPool->mDataStream + context.mContactStreamPool->mDataStreamSize - index;

			const PxU32 patchIndex = PxU32(Ps::atomicAdd(&context.mPatchStreamPool->mSharedDataIndex, PxI32(patchSize)));
			
			if(context.mPatchStreamPool->isOverflown())
			{
				PX_WARN_ONCE("Patch buffer overflow detected, please increase its size in the scene desc!\n");
				isOverflown = true;
			}
			contactPatches = context.mPatchStreamPool->mDataStream + context.mPatchStreamPool->mDataStreamSize - patchIndex;

			if(forceSize)
			{
				forceSize = isMeshType ? (forceSize * 2) : forceSize;

				index = PxU32(Ps::atomicAdd(&context.mForceAndIndiceStreamPool->mSharedDataIndex, PxI32(forceSize)));
			
				if(context.mForceAndIndiceStreamPool->isOverflown())
				{
					PX_WARN_ONCE("Force buffer overflow detected, please increase its size in the scene desc!\n");
					isOverflown = true;
				}
				forceBuffer = reinterpret_cast<PxReal*>(context.mForceAndIndiceStreamPool->mDataStream + context.mForceAndIndiceStreamPool->mDataStreamSize - index);
			}

			if(isOverflown)
			{
				contactPatches = NULL;
				contactPoints = NULL;
				forceBuffer = NULL;
				cmOutput.nbContacts = cmOutput.nbPatches = 0;
			}
			else
			{
				PxMemCopy(contactPatches, oldPatches, patchSize);
				PxMemCopy(contactPoints, oldContacts, contactSize);
			}
		}
		else
		{
			const PxU32 alignedOldSize = (oldSize + 0xf) & 0xfffffff0;

			PxU8* data = context.mContactBlockStream.reserve(alignedOldSize + forceSize);
			if(forceSize)
				forceBuffer = reinterpret_cast<PxReal*>(data + alignedOldSize);

			contactPatches = data;
			contactPoints = data + cmOutput.nbPatches * sizeof(PxContactPatch);

			PxMemCopy(data, oldPatches, oldSize);
		}

		if(forceSize)
		{
			PxMemZero(forceBuffer, forceSize);
		}
		
		cmOutput.contactPatches= contactPatches;
		cmOutput.contactPoints = contactPoints;
		cmOutput.contactForces = forceBuffer;
	}

	if(cache.mCachedSize)
	{
		if(cache.isMultiManifold())
		{
			PX_ASSERT((cache.mCachedSize & 0xF) == 0);
			PxU8* newData = context.mNpCacheStreamPair.reserve(cache.mCachedSize);
			PX_ASSERT((reinterpret_cast<uintptr_t>(newData)& 0xF) == 0);
			PxMemCopy(newData, & cache.getMultipleManifold(), cache.mCachedSize);
			cache.setMultiManifold(newData);
		}
		else if(useContactCache)
		{
			//Copy cache information as well...
			const PxU8* cachedData = cache.mCachedData;
			PxU8* newData = context.mNpCacheStreamPair.reserve(PxU32(cache.mCachedSize + 0xf) & 0xfff0);
			PxMemCopy(newData, cachedData, cache.mCachedSize);
			cache.mCachedData = newData;
		}
	}
	return ret;
}

void physx::PxcDiscreteNarrowPhase(PxcNpThreadContext& context, PxcNpWorkUnit& input, Gu::Cache& cache, PxsContactManagerOutput& output)
{
	//ML : if user doesn't raise the eDETECT_DISCRETE_CONTACT, we should not generate contacts
	if(!(input.flags & PxcNpWorkUnitFlag::eDETECT_DISCRETE_CONTACT))
		return;

	PxGeometryType::Enum type0 = static_cast<PxGeometryType::Enum>(input.geomType0);
	PxGeometryType::Enum type1 = static_cast<PxGeometryType::Enum>(input.geomType1);

	const bool flip = (type1<type0);

	const PxsCachedTransform* cachedTransform0 = &context.mTransformCache->getTransformCache(input.mTransformCache0);
	const PxsCachedTransform* cachedTransform1 = &context.mTransformCache->getTransformCache(input.mTransformCache1);

	if(!(output.statusFlag & PxcNpWorkUnitStatusFlag::eDIRTY_MANAGER) && !(input.flags & PxcNpWorkUnitFlag::eMODIFIABLE_CONTACT))
	{
		const PxU32 body0Dynamic = PxU32(input.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0);
		const PxU32 body1Dynamic = PxU32(input.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1);

		const PxU32 active0 = PxU32(body0Dynamic && !cachedTransform0->isFrozen());
		const PxU32 active1 = PxU32(body1Dynamic && !cachedTransform1->isFrozen());

		if(!(active0 || active1))
		{
			if(flip)
				Ps::swap(type0, type1);

			const bool useContactCache = context.mContactCache && g_CanUseContactCache[type0][type1];
			
#if PX_ENABLE_SIM_STATS
			if(output.nbContacts)
				context.mNbDiscreteContactPairsWithContacts++;
#endif
			const bool isMeshType = type1 > PxGeometryType::eCONVEXMESH;
			copyBuffers(output, cache, context, useContactCache, isMeshType);
			return;
		}
	}

	output.statusFlag &= (~PxcNpWorkUnitStatusFlag::eDIRTY_MANAGER);

	PxsShapeCore* shape0 = const_cast<PxsShapeCore*>(input.shapeCore0);
	PxsShapeCore* shape1 = const_cast<PxsShapeCore*>(input.shapeCore1);

	if(flip)
	{
		Ps::swap(type0, type1);
		Ps::swap(shape0, shape1);
		Ps::swap(cachedTransform0, cachedTransform1);
	}

	// PT: many cache misses here...
	// PT: TODO: refactor this change with runNpBatchPPU
	
	Ps::prefetchLine(shape1, 0);	// PT: at least get rid of L2s for shape1

	const PxTransform* tm0 = &cachedTransform0->transform;
	const PxTransform* tm1 = &cachedTransform1->transform;
	PX_ASSERT(tm0->isSane() && tm1->isSane());

	updateDiscreteContactStats(context, type0, type1);

	const PxReal contactDist0 = context.mContactDistance[input.mTransformCache0];
	const PxReal contactDist1 = context.mContactDistance[input.mTransformCache1];
	//context.mNarrowPhaseParams.mContactDistance = shape0->contactOffset + shape1->contactOffset;
	context.mNarrowPhaseParams.mContactDistance = contactDist0 + contactDist1;

	startContacts(output, context);

	const PxcContactMethod conMethod = g_ContactMethodTable[type0][type1];
	PX_ASSERT(conMethod);

	const bool useContactCache = context.mContactCache && g_CanUseContactCache[type0][type1];
	if(useContactCache)
	{
#if PX_ENABLE_SIM_STATS
		if(PxcCacheLocalContacts(context, cache, *tm0, *tm1, conMethod, shape0->geometry, shape1->geometry))
			context.mNbDiscreteContactPairsWithCacheHits++;
#else
		PxcCacheLocalContacts(context, n.pairCache, *tm0, *tm1, conMethod, shape0->geometry, shape1->geometry);
#endif
	}
	else
	{
		conMethod(shape0->geometry, shape1->geometry, *tm0, *tm1, context.mNarrowPhaseParams, cache, context.mContactBuffer, &context.mRenderOutput);
	}

	PxsMaterialInfo materialInfo[ContactBuffer::MAX_CONTACTS];

	const PxcGetMaterialMethod materialMethod = g_GetMaterialMethodTable[type0][type1];
	PX_ASSERT(materialMethod);

	materialMethod(shape0, shape1, context, materialInfo);

	if(flip)
		flipContacts(context, materialInfo);

	const bool isMeshType = type1 > PxGeometryType::eCONVEXMESH; 
	finishContacts(input, output, context, materialInfo, isMeshType);
}

void physx::PxcDiscreteNarrowPhasePCM(PxcNpThreadContext& context, PxcNpWorkUnit& cmInput, Gu::Cache& cache, PxsContactManagerOutput& output)
{
	//ML : if user doesn't raise the eDETECT_DISCRETE_CONTACT, we should not generate contacts
	if(!(cmInput.flags & PxcNpWorkUnitFlag::eDETECT_DISCRETE_CONTACT))
		return;
	
	PxGeometryType::Enum type0 = static_cast<PxGeometryType::Enum>(cmInput.geomType0);
	PxGeometryType::Enum type1 = static_cast<PxGeometryType::Enum>(cmInput.geomType1);

	const bool flip = type1<type0;

	const PxsCachedTransform* tm0 = &context.mTransformCache->getTransformCache(cmInput.mTransformCache0);
	const PxsCachedTransform* tm1 = &context.mTransformCache->getTransformCache(cmInput.mTransformCache1);

	if(!(output.statusFlag & PxcNpWorkUnitStatusFlag::eDIRTY_MANAGER) && !(cmInput.flags & PxcNpWorkUnitFlag::eMODIFIABLE_CONTACT))
	{
		const PxU32 body0Dynamic = PxU32(cmInput.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY0);
		const PxU32 body1Dynamic = PxU32(cmInput.flags & PxcNpWorkUnitFlag::eDYNAMIC_BODY1);

		const PxU32 active0 = PxU32(body0Dynamic && !(tm0->isFrozen()));
		const PxU32 active1 = PxU32(body1Dynamic && !(tm1->isFrozen()));

		if(!(active0 || active1))
		{
			if(flip)
				Ps::swap(type0, type1);

#if PX_ENABLE_SIM_STATS
			if(output.nbContacts)
				context.mNbDiscreteContactPairsWithContacts++;
#endif
			const bool isMeshType = type1 > PxGeometryType::eCONVEXMESH; 
			copyBuffers(output, cache, context, false, isMeshType);
			return;
		}
	}

	output.statusFlag &= (~PxcNpWorkUnitStatusFlag::eDIRTY_MANAGER);

	Gu::MultiplePersistentContactManifold& manifold = context.mTempManifold;
	bool isMultiManifold = false;

	if(cache.isMultiManifold())
	{
		//We are using a multi-manifold. This is cached in a reduced npCache...
		isMultiManifold = true;
		uintptr_t address = uintptr_t(&cache.getMultipleManifold());
		manifold.fromBuffer(reinterpret_cast<PxU8*>(address));
		cache.setMultiManifold(&manifold);
	}
	else if(cache.isManifold())
	{
		void* address = reinterpret_cast<void*>(&cache.getManifold());
		Ps::prefetch(address);
		Ps::prefetch(address, 128);
		Ps::prefetch(address, 256);
	}

	PxsShapeCore* shape0 = const_cast<PxsShapeCore*>(cmInput.shapeCore0);
	PxsShapeCore* shape1 = const_cast<PxsShapeCore*>(cmInput.shapeCore1);

	if(flip)
	{
		Ps::swap(tm0, tm1);
		Ps::swap(shape0, shape1);
		Ps::swap(type0, type1);
	}

	const PxReal contactDist0 = context.mContactDistance[cmInput.mTransformCache0];
	const PxReal contactDist1 = context.mContactDistance[cmInput.mTransformCache1];
//	context.mNarrowPhaseParams.mContactDistance = shape0->contactOffset + shape1->contactOffset;
	context.mNarrowPhaseParams.mContactDistance = contactDist0 + contactDist1;

	PX_ASSERT(tm0->transform.isSane() && tm1->transform.isSane());

	updateDiscreteContactStats(context, type0, type1);

	const PxcContactMethod conMethod = g_PCMContactMethodTable[type0][type1];
	PX_ASSERT(conMethod);

	startContacts(output, context);

	conMethod(shape0->geometry, shape1->geometry, tm0->transform, tm1->transform, context.mNarrowPhaseParams, cache, context.mContactBuffer, &context.mRenderOutput);
	
	PxsMaterialInfo materialInfo[ContactBuffer::MAX_CONTACTS];

	const PxcGetMaterialMethod materialMethod = g_GetMaterialMethodTable[type0][type1];
	PX_ASSERT(materialMethod);

	materialMethod(shape0, shape1, context,  materialInfo);

	if(flip)
		flipContacts(context, materialInfo);

	if(isMultiManifold)
	{
		//Store the manifold back...
		const PxU32 size = (sizeof(MultiPersistentManifoldHeader) +
			manifold.mNumManifolds * sizeof(SingleManifoldHeader) +
			manifold.mNumTotalContacts * sizeof(Gu::CachedMeshPersistentContact));

		PxU8* buffer = context.mNpCacheStreamPair.reserve(size);

		PX_ASSERT((reinterpret_cast<uintptr_t>(buffer)& 0xf) == 0);
		manifold.toBuffer(buffer);
		cache.setMultiManifold(buffer);
		cache.mCachedSize = Ps::to16(size);
	}

	const bool isMeshType = type1 > PxGeometryType::eCONVEXMESH;
	finishContacts(cmInput, output, context, materialInfo, isMeshType);
}
