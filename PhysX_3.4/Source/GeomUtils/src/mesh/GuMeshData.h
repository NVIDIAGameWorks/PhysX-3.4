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

#ifndef GU_MESH_DATA_H
#define GU_MESH_DATA_H

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxVec4.h"
#include "foundation/PxBounds3.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PsAllocator.h"
#include "PxTriangleMesh.h"
#include "GuRTree.h"
#include "GuBV4.h"
#include "GuBV32.h"

namespace physx
{

#define RTREE_COOK_VERSION 1

namespace Gu {
	
// 1: support stackless collision trees for non-recursive collision queries
// 2: height field functionality not supported anymore
// 3: mass struct removed
// 4: bounding sphere removed
// 5: RTree added, opcode tree still in the binary image, physx 3.0
// 6: opcode tree removed from binary image
// 7: convex decomposition is out
// 8: adjacency information added
// 9: removed leaf triangles and most of opcode data, changed rtree layout
// 10: float rtrees
// 11: new build, isLeaf added to page
// 12: isLeaf is now the lowest bit in ptrs
// 13: TA30159 removed deprecated convexEdgeThreshold and bumped version
// 14: added midphase ID
// 15: GPU data simplification

#define PX_MESH_VERSION 15

// these flags are used to indicate/validate the contents of a cooked mesh file
enum InternalMeshSerialFlag
{
	IMSF_MATERIALS		=	(1<<0),	//!< if set, the cooked mesh file contains per-triangle material indices
	IMSF_FACE_REMAP		=	(1<<1),	//!< if set, the cooked mesh file contains a remap table
	IMSF_8BIT_INDICES	=	(1<<2),	//!< if set, the cooked mesh file contains 8bit indices (topology)
	IMSF_16BIT_INDICES	=	(1<<3),	//!< if set, the cooked mesh file contains 16bit indices (topology)
	IMSF_ADJACENCIES	=	(1<<4),	//!< if set, the cooked mesh file contains adjacency structures
	IMSF_GRB_DATA		=	(1<<5)	//!< if set, the cooked mesh file contains GRB data structures
};



#if PX_VC
#pragma warning(push)
#pragma warning(disable: 4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif

	class TriangleMeshData : public Ps::UserAllocated
	{
		public:
		PxMeshMidPhase::Enum	mType;

		PxU32					mNbVertices;
		PxU32					mNbTriangles;
		PxVec3*					mVertices;
		void*					mTriangles;

		PxBounds3				mAABB;
		PxU8*					mExtraTrigData;
		PxReal					mGeomEpsilon;

		PxU8					mFlags;
		PxU16*					mMaterialIndices;
		PxU32*					mFaceRemap;
		PxU32*					mAdjacencies;

		// GRB data -------------------------
		void *					mGRB_triIndices;				//!< GRB: GPU-friendly tri indices(uint3)

		// TODO avoroshilov: adjacency info - duplicated, remove it and use 'mAdjacencies' and 'mExtraTrigData' see GuTriangleMesh.cpp:325
		void *					mGRB_triAdjacencies;			//!< GRB: adjacency data, with BOUNDARY and NONCONVEX flags (flags replace adj indices where applicable) [uin4]
		PxU32*					mGRB_faceRemap;					//!< GRB: this remap the GPU triangle indices to CPU triangle indices

		void*					mGRB_BV32Tree;
		// End of GRB data ------------------

		TriangleMeshData() :
			mNbVertices			(0),
			mNbTriangles		(0),
			mVertices			(NULL),
			mTriangles			(NULL),
			mAABB				(PxBounds3::empty()),
			mExtraTrigData		(NULL),
			mGeomEpsilon		(0.0f),
			mFlags				(0),
			mMaterialIndices	(NULL),
			mFaceRemap			(NULL),
			mAdjacencies		(NULL),

			mGRB_triIndices					(NULL),
			mGRB_triAdjacencies				(NULL),
			mGRB_faceRemap					(NULL),
			mGRB_BV32Tree					(NULL)
			
		{
		}

		virtual ~TriangleMeshData()
		{
			if(mVertices) 
				PX_FREE(mVertices);
			if(mTriangles) 
				PX_FREE(mTriangles);
			if(mMaterialIndices)
				PX_DELETE_POD(mMaterialIndices);
			if(mFaceRemap)
				PX_DELETE_POD(mFaceRemap);
			if(mAdjacencies)
				PX_DELETE_POD(mAdjacencies);
			if(mExtraTrigData)
				PX_DELETE_POD(mExtraTrigData);


			if (mGRB_triIndices)
				PX_FREE(mGRB_triIndices);
			if (mGRB_triAdjacencies)
				PX_DELETE_POD(mGRB_triAdjacencies);

			if (mGRB_faceRemap)
				PX_DELETE_POD(mGRB_faceRemap);

			if (mGRB_BV32Tree)
			{
				Gu::BV32Tree* bv32Tree = reinterpret_cast<BV32Tree*>(mGRB_BV32Tree);
				PX_DELETE(bv32Tree);
				mGRB_BV32Tree = NULL;
			}

			
		}


		PxVec3* allocateVertices(PxU32 nbVertices)
		{
			PX_ASSERT(!mVertices);
			// PT: we allocate one more vertex to make sure it's safe to V4Load the last one
			const PxU32 nbAllocatedVerts = nbVertices + 1;
			mVertices = reinterpret_cast<PxVec3*>(PX_ALLOC(nbAllocatedVerts * sizeof(PxVec3), "PxVec3"));
			mNbVertices = nbVertices;
			return mVertices;
		}

		void* allocateTriangles(PxU32 nbTriangles, bool force32Bit, PxU32 allocateGPUData = 0)
		{
			PX_ASSERT(mNbVertices);
			PX_ASSERT(!mTriangles);

			bool index16 = mNbVertices <= 0xffff && !force32Bit;
			if(index16)
				mFlags |= PxTriangleMeshFlag::e16_BIT_INDICES;

			mTriangles = PX_ALLOC(nbTriangles * (index16 ? sizeof(PxU16) : sizeof(PxU32)) * 3, "mTriangles");
			if (allocateGPUData)
				mGRB_triIndices = PX_ALLOC(nbTriangles * (index16 ? sizeof(PxU16) : sizeof(PxU32)) * 3, "mGRB_triIndices");
			mNbTriangles = nbTriangles;
			return mTriangles;
		}

		PxU16* allocateMaterials()
		{
			PX_ASSERT(mNbTriangles);
			PX_ASSERT(!mMaterialIndices);
			mMaterialIndices = PX_NEW(PxU16)[mNbTriangles];
			return mMaterialIndices;
		}

		PxU32* allocateAdjacencies()
		{
			PX_ASSERT(mNbTriangles);
			PX_ASSERT(!mAdjacencies);
			mAdjacencies = PX_NEW(PxU32)[mNbTriangles*3];
			mFlags |= PxTriangleMeshFlag::eADJACENCY_INFO;
			return mAdjacencies;
		}

		PxU32* allocateFaceRemap()
		{
			PX_ASSERT(mNbTriangles);
			PX_ASSERT(!mFaceRemap);
			mFaceRemap = PX_NEW(PxU32)[mNbTriangles];
			return mFaceRemap;
		}

		PxU8* allocateExtraTrigData()
		{
			PX_ASSERT(mNbTriangles);
			PX_ASSERT(!mExtraTrigData);
			mExtraTrigData = PX_NEW(PxU8)[mNbTriangles];
			return mExtraTrigData;
		}

		PX_FORCE_INLINE void	setTriangleAdjacency(PxU32 triangleIndex, PxU32 adjacency, PxU32 offset)
		{
			PX_ASSERT(mAdjacencies); 
			mAdjacencies[triangleIndex*3 + offset] = adjacency; 
		}

		PX_FORCE_INLINE	bool	has16BitIndices()	const	
		{ 
			return (mFlags & PxTriangleMeshFlag::e16_BIT_INDICES) ? true : false; 
		}
	};

	class RTreeTriangleData : public TriangleMeshData
	{
		public:
								RTreeTriangleData()		{ mType = PxMeshMidPhase::eBVH33; }
		virtual					~RTreeTriangleData()	{}

				Gu::RTree		mRTree;
	};

	class BV4TriangleData : public TriangleMeshData
	{
		public:
								BV4TriangleData()	{ mType = PxMeshMidPhase::eBVH34;	}
		virtual					~BV4TriangleData()	{}

				Gu::SourceMesh	mMeshInterface;
				Gu::BV4Tree		mBV4Tree;
	};


	class BV32TriangleData : public TriangleMeshData
	{
	public:
		//using the same type as BV4 
		BV32TriangleData()	{ mType = PxMeshMidPhase::eBVH34; }
		virtual					~BV32TriangleData()	{}

		Gu::SourceMesh	mMeshInterface;
		Gu::BV32Tree		mBV32Tree;
	};

#if PX_VC
#pragma warning(pop)
#endif


} // namespace Gu

}

#endif // #ifdef GU_MESH_DATA_H
