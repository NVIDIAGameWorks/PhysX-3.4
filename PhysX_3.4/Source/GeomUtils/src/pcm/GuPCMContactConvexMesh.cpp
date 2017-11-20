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


#include "GuVecBox.h"
#include "GuVecConvexHull.h"
#include "GuVecConvexHullNoScale.h"
#include "GuVecTriangle.h"
#include "GuGeometryUnion.h"

#include "GuContactMethodImpl.h"
#include "GuPCMShapeConvex.h"
#include "GuConvexUtilsInternal.h"
#include "PxTriangleMesh.h"
#include "GuContactBuffer.h"
#include "GuPCMContactConvexCommon.h"
#include "GuPCMContactMeshCallback.h"
#include "GuIntersectionTriangleBox.h"
#include "GuBox.h"

using namespace physx;
using namespace Gu;
using namespace physx::shdfnd::aos;

namespace physx
{

struct PCMConvexVsMeshContactGenerationCallback : PCMMeshContactGenerationCallback<PCMConvexVsMeshContactGenerationCallback>
{
	PCMConvexVsMeshContactGenerationCallback& operator=(const PCMConvexVsMeshContactGenerationCallback&);
public:
	PCMConvexVsMeshContactGeneration	mGeneration;
	const BoxPadded&					mBox;

	PCMConvexVsMeshContactGenerationCallback(
		const Ps::aos::FloatVArg					contactDistance,
		const Ps::aos::FloatVArg					replaceBreakingThreshold,
		const PsTransformV&							convexTransform, 
		const PsTransformV&							meshTransform,
		MultiplePersistentContactManifold&			multiManifold,
		ContactBuffer&								contactBuffer,
		const PolygonalData&						polyData,
		SupportLocal*								polyMap,
		Ps::InlineArray<PxU32,LOCAL_CONTACTS_SIZE>*	delayedContacts,
		const Cm::FastVertex2ShapeScaling&			convexScaling,
		bool										idtConvexScale,
		const Cm::FastVertex2ShapeScaling&			meshScaling,
		const PxU8*									extraTriData,
		bool										idtMeshScale,
		const BoxPadded&							box,
		Cm::RenderOutput*							renderOutput = NULL
		
	) :
		PCMMeshContactGenerationCallback<PCMConvexVsMeshContactGenerationCallback>(meshScaling, extraTriData, idtMeshScale),
		mGeneration(contactDistance, replaceBreakingThreshold, convexTransform, meshTransform, multiManifold, contactBuffer, polyData, polyMap, delayedContacts, convexScaling, idtConvexScale, renderOutput),
		mBox(box)
	{
	}

	PX_FORCE_INLINE Ps::IntBool doTest(const PxVec3& v0, const PxVec3& v1, const PxVec3& v2)
	{
		// PT: this one is safe because midphase vertices are directly passed to the function
		return intersectTriangleBox(mBox, v0, v1, v2);
	}

	template<PxU32 CacheSize>
	void processTriangleCache(TriangleCache<CacheSize>& cache)
	{
		mGeneration.processTriangleCache<CacheSize, PCMConvexVsMeshContactGeneration>(cache);
	}
	
};


bool Gu::PCMContactConvexMesh(const PolygonalData& polyData, SupportLocal* polyMap, const Ps::aos::FloatVArg minMargin, const PxBounds3& hullAABB, const PxTriangleMeshGeometryLL& shapeMesh,
						const PxTransform& transform0, const PxTransform& transform1,
						PxReal contactDistance, ContactBuffer& contactBuffer,
						const Cm::FastVertex2ShapeScaling& convexScaling, const Cm::FastVertex2ShapeScaling& meshScaling,
						bool idtConvexScale, bool idtMeshScale, Gu::MultiplePersistentContactManifold& multiManifold,
						Cm::RenderOutput* renderOutput)

{
	using namespace Ps::aos;

	const QuatV q0 = QuatVLoadA(&transform0.q.x);
	const Vec3V p0 = V3LoadA(&transform0.p.x);

	const QuatV q1 = QuatVLoadA(&transform1.q.x);
	const Vec3V p1 = V3LoadA(&transform1.p.x);

	const FloatV contactDist = FLoad(contactDistance);
	//Transfer A into the local space of B
	const PsTransformV convexTransform(p0, q0);//box
	const PsTransformV meshTransform(p1, q1);//triangleMesh  
	const PsTransformV curTransform = meshTransform.transformInv(convexTransform);
	
	
	if(multiManifold.invalidate(curTransform, minMargin))
	{
		const FloatV replaceBreakingThreshold = FMul(minMargin, FLoad(0.05f));
		multiManifold.mNumManifolds = 0;
		multiManifold.setRelativeTransform(curTransform); 

	
		////////////////////
		const TriangleMesh* PX_RESTRICT meshData = shapeMesh.meshData;

		const Cm::Matrix34 world0(transform0);
		const Cm::Matrix34 world1(transform1);
		BoxPadded hullOBB;
		computeHullOBB(hullOBB, hullAABB, contactDistance, world0, world1, meshScaling, idtMeshScale);

		// Setup the collider

		Ps::InlineArray<PxU32,LOCAL_CONTACTS_SIZE> delayedContacts;
		
		const PxU8* PX_RESTRICT extraData = meshData->getExtraTrigData();
		PCMConvexVsMeshContactGenerationCallback blockCallback(
			contactDist, replaceBreakingThreshold, convexTransform, meshTransform, multiManifold, contactBuffer,
			polyData, polyMap, &delayedContacts, convexScaling, idtConvexScale, meshScaling, extraData, idtMeshScale, 
			hullOBB, renderOutput);

		Midphase::intersectOBB(meshData, hullOBB, blockCallback, true);

		PX_ASSERT(multiManifold.mNumManifolds <= GU_MAX_MANIFOLD_SIZE);

		blockCallback.flushCache();
		//This is very important
		blockCallback.mGeneration.generateLastContacts();
		blockCallback.mGeneration.processContacts(GU_SINGLE_MANIFOLD_CACHE_SIZE, false);

#if PCM_LOW_LEVEL_DEBUG
		multiManifold.drawManifold(*renderOutput, transform0, transform1);
#endif
	}
	else
	{
		const PsMatTransformV aToB(curTransform);
		const FloatV projectBreakingThreshold = FMul(minMargin, FLoad(0.8f));
		multiManifold.refreshManifold(aToB, projectBreakingThreshold, contactDist);
	}

	return multiManifold.addManifoldContactsToContactBuffer(contactBuffer, meshTransform);
}

bool Gu::pcmContactConvexMesh(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	PX_UNUSED(renderOutput);

	const PxConvexMeshGeometryLL& shapeConvex = shape0.get<const PxConvexMeshGeometryLL>();
	const PxTriangleMeshGeometryLL& shapeMesh = shape1.get<const PxTriangleMeshGeometryLL>();

	const ConvexHullData* hullData = shapeConvex.hullData;
	MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();

	const PsTransformV convexTransform = loadTransformA(transform0);

	const bool idtScaleMesh = shapeMesh.scale.isIdentity();

	Cm::FastVertex2ShapeScaling meshScaling;
	if(!idtScaleMesh)
		meshScaling.init(shapeMesh.scale);

	Cm::FastVertex2ShapeScaling convexScaling;
	PxBounds3 hullAABB;
	PolygonalData polyData;
	const bool idtScaleConvex = getPCMConvexData(shape0, convexScaling, hullAABB, polyData);
	const QuatV vQuat = QuatVLoadU(&shapeConvex.scale.rotation.x);

	const Vec3V vScale = V3LoadU_SafeReadW(shapeConvex.scale.scale);	// PT: safe because 'rotation' follows 'scale' in PxMeshScale
	
	const PxReal toleranceScale = params.mToleranceLength;
	const FloatV minMargin = CalculatePCMConvexMargin(hullData, vScale, toleranceScale, GU_PCM_MESH_MANIFOLD_EPSILON);
	
	ConvexHullV convexHull(hullData, V3Zero(), vScale, vQuat, idtScaleConvex);

	if(idtScaleConvex)
	{
		SupportLocalImpl<Gu::ConvexHullNoScaleV> convexMap(static_cast<ConvexHullNoScaleV&>(convexHull), convexTransform, convexHull.vertex2Shape, convexHull.shape2Vertex, true);
		return Gu::PCMContactConvexMesh(polyData, &convexMap, minMargin, hullAABB, shapeMesh,transform0,transform1, params.mContactDistance, contactBuffer, convexScaling,  
			meshScaling, idtScaleConvex, idtScaleMesh, multiManifold, renderOutput);
	}
	else
	{
		SupportLocalImpl<Gu::ConvexHullV> convexMap(convexHull, convexTransform, convexHull.vertex2Shape, convexHull.shape2Vertex, false);
		return Gu::PCMContactConvexMesh(polyData, &convexMap, minMargin, hullAABB, shapeMesh,transform0,transform1, params.mContactDistance, contactBuffer, convexScaling,  
			meshScaling, idtScaleConvex, idtScaleMesh, multiManifold, renderOutput);
	}
}

bool Gu::pcmContactBoxMesh(GU_CONTACT_METHOD_ARGS)
{
	using namespace Ps::aos;
	PX_UNUSED(renderOutput);

	MultiplePersistentContactManifold& multiManifold = cache.getMultipleManifold();

	const PxBoxGeometry& shapeBox = shape0.get<const PxBoxGeometry>();
	const PxTriangleMeshGeometryLL& shapeMesh = shape1.get<const PxTriangleMeshGeometryLL>();

	const PxBounds3 hullAABB(-shapeBox.halfExtents, shapeBox.halfExtents);  

	const bool idtMeshScale = shapeMesh.scale.isIdentity();

	Cm::FastVertex2ShapeScaling meshScaling;
	if(!idtMeshScale)
		meshScaling.init(shapeMesh.scale);

	Cm::FastVertex2ShapeScaling idtScaling;

	const Vec3V boxExtents = V3LoadU(shapeBox.halfExtents);
	const PxReal toleranceLength = params.mToleranceLength;
	const FloatV minMargin = Gu::CalculatePCMBoxMargin(boxExtents, toleranceLength, GU_PCM_MESH_MANIFOLD_EPSILON);

	BoxV boxV(V3Zero(), boxExtents);

	const PsTransformV boxTransform = loadTransformA(transform0);//box

	PolygonalData polyData;
	PCMPolygonalBox polyBox(shapeBox.halfExtents);
	polyBox.getPolygonalData(&polyData);

	Mat33V identity =  M33Identity();
	SupportLocalImpl<BoxV> boxMap(boxV, boxTransform, identity, identity, true);

	return Gu::PCMContactConvexMesh(polyData, &boxMap, minMargin, hullAABB, shapeMesh,transform0,transform1, params.mContactDistance, contactBuffer, idtScaling,  meshScaling, 
		true, idtMeshScale, multiManifold, renderOutput);
}

}
