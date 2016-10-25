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

#ifndef PX_PSCOOKING_H
#define PX_PSCOOKING_H

#include "foundation/PxMemory.h"
#include "PxCooking.h"
#include "PsUserAllocated.h"

namespace physx
{
class TriangleMeshBuilder;
class ConvexMeshBuilder;
class ConvexHullLib;

class Cooking: public PxCooking, public Ps::UserAllocated
{
public:
									Cooking(const PxCookingParams& params): mParams(params) {}

	virtual void					release();
	virtual void					setParams(const PxCookingParams& params);
	virtual const PxCookingParams&	getParams();
	virtual bool					platformMismatch();
	virtual bool					cookTriangleMesh(const PxTriangleMeshDesc& desc, PxOutputStream& stream, PxTriangleMeshCookingResult::Enum* condition = NULL);
	virtual PxTriangleMesh*			createTriangleMesh(const PxTriangleMeshDesc& desc, PxPhysicsInsertionCallback& insertionCallback);
	virtual bool					validateTriangleMesh(const PxTriangleMeshDesc& desc);

	virtual bool					cookConvexMesh(const PxConvexMeshDesc& desc, PxOutputStream& stream, PxConvexMeshCookingResult::Enum* condition);
	virtual PxConvexMesh*			createConvexMesh(const PxConvexMeshDesc& desc, PxPhysicsInsertionCallback& insertionCallback);
	virtual bool					validateConvexMesh(const PxConvexMeshDesc& desc);
	virtual bool					computeHullPolygons(const PxSimpleTriangleMesh& mesh, PxAllocatorCallback& inCallback,PxU32& nbVerts, PxVec3*& vertices,
											PxU32& nbIndices, PxU32*& indices, PxU32& nbPolygons, PxHullPolygon*& hullPolygons);
	virtual bool					cookHeightField(const PxHeightFieldDesc& desc, PxOutputStream& stream);
	virtual PxHeightField*			createHeightField(const PxHeightFieldDesc& desc, PxPhysicsInsertionCallback& insertionCallback);

	PX_FORCE_INLINE static void		gatherStrided(const void* src, void* dst, PxU32 nbElem, PxU32 elemSize, PxU32 stride)
	{
		const PxU8* s = reinterpret_cast<const PxU8*>(src);
		PxU8* d = reinterpret_cast<PxU8*>(dst);
		while(nbElem--)
		{
			PxMemCopy(d, s, elemSize);
			d += elemSize;
			s += stride;
		}
	}

private:
	bool							cookConvexMeshInternal(const PxConvexMeshDesc& desc, ConvexMeshBuilder& meshBuilder, ConvexHullLib* hullLib, PxConvexMeshCookingResult::Enum* condition);

private:
			PxCookingParams			mParams;

			bool					cookTriangleMesh(TriangleMeshBuilder& builder, const PxTriangleMeshDesc& desc, PxOutputStream& stream, PxTriangleMeshCookingResult::Enum* condition);
			PxTriangleMesh*			createTriangleMesh(TriangleMeshBuilder& builder, const PxTriangleMeshDesc& desc, PxPhysicsInsertionCallback& insertionCallback);
};

}
#endif //#define PX_PSCOOKING_H
