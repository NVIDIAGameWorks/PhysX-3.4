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

#ifndef SPLIT_MESH_H

#define SPLIT_MESH_H

#include <stdlib.h>
#include "foundation/PxSimpleTypes.h"
#include "PsAllocator.h"

namespace SPLIT_MESH
{

class SimpleMesh
{
public:
	SimpleMesh(void)
	{
		mVcount = 0;
		mTcount = 0;
		mVertices = NULL;
		mIndices = NULL;
	}
	SimpleMesh(physx::PxU32 vcount,physx::PxU32 tcount,const physx::PxF32 *vertices,const physx::PxU32 *indices)
	{
		mVcount = 0;
		mTcount = 0;
		mVertices = NULL;
		mIndices = NULL;
		set(vcount,tcount,vertices,indices);
	}

	void set(physx::PxU32 vcount,physx::PxU32 tcount,const physx::PxF32 *vertices,const physx::PxU32 *indices)
	{
		release();
		mVcount = vcount;
		mTcount = tcount;
		mVertices = (physx::PxF32 *)PX_ALLOC(sizeof(physx::PxF32)*3*mVcount, PX_DEBUG_EXP("SimpleMesh"));
		memcpy(mVertices,vertices,sizeof(physx::PxF32)*3*mVcount);
		mIndices = (physx::PxU32 *)PX_ALLOC(sizeof(physx::PxU32)*3*mTcount, PX_DEBUG_EXP("SimpleMesh"));
		memcpy(mIndices,indices,sizeof(physx::PxU32)*3*mTcount);
	}


	~SimpleMesh(void)
	{
		release();
	}

	void release(void)
	{
		PX_FREE(mVertices);
		PX_FREE(mIndices);
		mVertices = NULL;
		mIndices = NULL;
		mVcount = 0;
		mTcount = 0;
	}


	physx::PxU32	mVcount;
	physx::PxU32	mTcount;
	physx::PxF32	*mVertices;
	physx::PxU32	*mIndices;
};


void splitMesh(const physx::PxF32 *planeEquation,const SimpleMesh &input,SimpleMesh &left,SimpleMesh &right,bool closedMesh);


};


#endif
