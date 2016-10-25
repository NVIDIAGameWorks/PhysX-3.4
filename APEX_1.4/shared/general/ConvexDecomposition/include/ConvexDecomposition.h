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
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

#ifndef CONVEX_DECOMPOSITION_H

#define CONVEX_DECOMPOSITION_H

#include "foundation/PxSimpleTypes.h"

namespace CONVEX_DECOMPOSITION
{

class TriangleMesh
{
public:
	TriangleMesh(void)
	{
		mVcount		= 0;
		mVertices	= NULL;
		mTriCount	= 0;
		mIndices	= NULL;
	}

	physx::PxU32	mVcount;
	physx::PxF32	*mVertices;
	physx::PxU32	mTriCount;
	physx::PxU32	*mIndices;
};

class ConvexResult
{
public:
  ConvexResult(void)
  {
    mHullVcount = 0;
    mHullVertices = 0;
    mHullTcount = 0;
    mHullIndices = 0;
  }

// the convex hull.result
  physx::PxU32		   	mHullVcount;			// Number of vertices in this convex hull.
  physx::PxF32 			*mHullVertices;			// The array of vertices (x,y,z)(x,y,z)...
  physx::PxU32       	mHullTcount;			// The number of triangles int he convex hull
  physx::PxU32			*mHullIndices;			// The triangle indices (0,1,2)(3,4,5)...
  physx::PxF32           mHullVolume;		    // the volume of the convex hull.

};

// just to avoid passing a zillion parameters to the method the
// options are packed into this descriptor.
class DecompDesc
{
public:
	DecompDesc(void)
	{
		mVcount = 0;
		mVertices = 0;
		mTcount   = 0;
		mIndices  = 0;
		mDepth    = 10;
		mCpercent = 4;
		mPpercent = 4;
		mVpercent = 0.2f;
		mMaxVertices = 32;
		mSkinWidth   = 0;
		mRemoveTjunctions = false;
		mInitialIslandGeneration = false;
		mUseIslandGeneration = false;
		mClosedSplit = false;
		mUseHACD = false;
		mConcavityHACD = 100;
		mMinClusterSizeHACD = 10;
		mConnectionDistanceHACD = 0;
  }

// describes the input triangle.
	physx::PxU32		mVcount;   // the number of vertices in the source mesh.
	const physx::PxF32  *mVertices; // start of the vertex position array.  Assumes a stride of 3 doubles.
	physx::PxU32		mTcount;   // the number of triangles in the source mesh.
	const physx::PxU32	*mIndices;  // the indexed triangle list array (zero index based)
// options
	physx::PxU32		mDepth;    // depth to split, a maximum of 10, generally not over 7.
	physx::PxF32		mCpercent; // the concavity threshold percentage.  0=20 is reasonable.
	physx::PxF32		mPpercent; // the percentage volume conservation threshold to collapse hulls. 0-30 is reasonable.
	physx::PxF32		mVpercent; // the pecentage of total mesh volume before we stop splitting.

	bool				mInitialIslandGeneration; // true if the initial mesh should be subjected to island generation.
	bool				mUseIslandGeneration; // true if the decomposition code should break the input mesh and split meshes into discrete 'islands/pieces'
	bool				mRemoveTjunctions; // whether or not to initially remove tjunctions found in the original mesh.
	bool				mClosedSplit;		// true if the hulls should be closed as they are split
	bool				mUseHACD;			// True if using the hierarchical approximate convex decomposition algorithm; recommended 
	physx::PxF32		mConcavityHACD;		// The concavity value to use for HACD (not the same as concavity percentage used with ACD. 
	physx::PxU32		mMinClusterSizeHACD;	// Minimum number of clusters to consider.
	physx::PxF32		mConnectionDistanceHACD; // The connection distance for HACD

// hull output limits.
	physx::PxU32		mMaxVertices; // maximum number of vertices in the output hull. Recommended 32 or less.
	physx::PxF32		mSkinWidth;   // a skin width to apply to the output hulls.
};

class ConvexDecomposition
{
public:
	virtual physx::PxU32 performConvexDecomposition(const DecompDesc &desc) = 0; // returns the number of hulls produced.
	virtual void release(void) = 0;
	virtual ConvexResult * getConvexResult(physx::PxU32 index) = 0;


	virtual void splitMesh(const physx::PxF32 *planeEquation,const TriangleMesh &input,TriangleMesh &left,TriangleMesh &right,bool closedMesh) = 0;
	virtual void releaseTriangleMeshMemory(TriangleMesh &mesh) = 0;

protected:
	virtual ~ConvexDecomposition(void) { };
};

ConvexDecomposition * createConvexDecomposition(void);

};

#endif
