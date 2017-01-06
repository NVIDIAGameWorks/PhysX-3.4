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


#include "GuVecTriangle.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuConvexUtilsInternal.h"
#include "PxTriangleMesh.h"
#include "GuContactBuffer.h"
#include "GuTriangleCache.h"
#include "GuPCMContactConvexCommon.h"
#include "GuHeightFieldUtil.h"
#include "GuPCMContactMeshCallback.h"
#include "GuBox.h"

using namespace physx;
using namespace Gu;
using namespace physx::shdfnd::aos;

namespace physx
{

struct PCMSphereVsMeshContactGenerationCallback : PCMMeshContactGenerationCallback< PCMSphereVsMeshContactGenerationCallback >
{

public:
	PCMSphereVsMeshContactGeneration		mGeneration;
	

	PCMSphereVsMeshContactGenerationCallback(
		const Ps::aos::Vec3VArg		sphereCenter,
		const Ps::aos::FloatVArg	sphereRadius,
		const Ps::aos::FloatVArg	contactDist,
		const Ps::aos::FloatVArg	replaceBreakingThreshold,
		const PsTransformV& sphereTransform,
		const PsTransformV& meshTransform,
		MultiplePersistentContactManifold& multiManifold,
		ContactBuffer& contactBuffer,
		const PxU8*		extraTriData,
		const Cm::FastVertex2ShapeScaling& meshScaling,
		bool idtMeshScale,
		Cm::RenderOutput* renderOutput = NULL
	) :
		PCMMeshContactGenerationCallback<PCMSphereVsMeshContactGenerationCallback>(meshScaling, extraTriData, idtMeshScale), 
		mGeneration(sphereCenter, sphereRadius, contactDist, replaceBreakingThreshold, sphereTransform, meshTransform, multiManifold, contactBuffer, renderOutput)
	{
	}

	PX_FORCE_INLINE bool doTest(const PxVec3&, const PxVec3&, const PxVec3&) { return true; }

	template<PxU32 CacheSize>
	void processTriangleCache(TriangleCache<CacheSize>& cache)
	{
		mGeneration.processTriangleCache<CacheSize, PCMSphereVsMeshContactGeneration>(cache);
	}
	
};


bool Gu::pcmContactSphereMesh(GU_CONTACT_METHOD_ARGS)
{
	PX_UNUSED(renderOutput);

	using namespace Ps::aos;
	MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();
	const PxSphereGeometry& shapeSphere = shape0.get<const PxSphereGeometry>();
	const PxTriangleMeshGeometryLL& shapeMesh = shape1.get<const PxTriangleMeshGeometryLL>();

	//gRenderOutPut = cache.mRenderOutput;

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&transform0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV sphereRadius = FLoad(shapeSphere.radius);
	const FloatV contactDist = FLoad(params.mContactDistance);
	
	const PsTransformV sphereTransform(p0, q0);//sphere transform
	const PsTransformV meshTransform(p1, q1);//triangleMesh  
	const PsTransformV curTransform = meshTransform.transformInv(sphereTransform);
	
	// We must be in local space to use the cache
	if(multiManifold.invalidate(curTransform, sphereRadius, FLoad(0.02f)))
	{
		const FloatV replaceBreakingThreshold = FMul(sphereRadius, FLoad(0.001f));
		const PxVec3 sphereCenterShape1Space = transform1.transformInv(transform0.p);
		PxReal inflatedRadius = shapeSphere.radius + params.mContactDistance;

		const Vec3V sphereCenter = V3LoadU(sphereCenterShape1Space);

		const TriangleMesh* meshData = shapeMesh.meshData;

		Cm::FastVertex2ShapeScaling meshScaling;	// PT: TODO: get rid of default ctor :(
		const bool idtMeshScale = shapeMesh.scale.isIdentity();
		if(!idtMeshScale)
			meshScaling.init(shapeMesh.scale);

		multiManifold.mNumManifolds = 0;
		multiManifold.setRelativeTransform(curTransform); 

		const PxU8* PX_RESTRICT extraData = meshData->getExtraTrigData();
		// mesh scale is not baked into cached verts
		PCMSphereVsMeshContactGenerationCallback callback(
			sphereCenter,
			sphereRadius,
			contactDist,
			replaceBreakingThreshold,
			sphereTransform,
			meshTransform,
			multiManifold,
			contactBuffer,
			extraData,
			meshScaling,
			idtMeshScale);

		PxVec3 obbCenter = sphereCenterShape1Space;
		PxVec3 obbExtents = PxVec3(inflatedRadius);
		PxMat33 obbRot(PxIdentity);
		if(!idtMeshScale)
			meshScaling.transformQueryBounds(obbCenter, obbExtents, obbRot);
		const Box obb(obbCenter, obbExtents, obbRot);

		Midphase::intersectOBB(meshData, obb, callback, true);

		callback.flushCache();

		callback.mGeneration.processContacts(GU_SPHERE_MANIFOLD_CACHE_SIZE, false);
	}
	else
	{
		const PsMatTransformV aToB(curTransform);
		const FloatV projectBreakingThreshold = FMul(sphereRadius, FLoad(0.05f));
		const FloatV refereshDistance = FAdd(sphereRadius, contactDist);
		multiManifold.refreshManifold(aToB, projectBreakingThreshold, refereshDistance);
	}
	
	//multiManifold.drawManifold(*gRenderOutPut, sphereTransform, meshTransform);
	return multiManifold.addManifoldContactsToContactBuffer(contactBuffer, sphereTransform, meshTransform, sphereRadius);
}

}
