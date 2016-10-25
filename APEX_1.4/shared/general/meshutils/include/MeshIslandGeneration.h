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

#ifndef MESH_ISLAND_GENERATION_H

#define MESH_ISLAND_GENERATION_H

#include "Ps.h"
#include "foundation/PxSimpleTypes.h"
#include "PsUserAllocated.h"

namespace physx
{
	namespace general_meshutils2
	{

class MeshIslandGeneration
{
public:

  virtual PxU32 islandGenerate(PxU32 tcount,const PxU32 *indices,const PxF32 *vertices) = 0;
  virtual PxU32 islandGenerate(PxU32 tcount,const PxU32 *indices,const PxF64 *vertices) = 0;

  // sometimes island generation can produce co-planar islands.  Slivers if you will.  If you are passing these islands into a geometric system
  // that wants to turn them into physical objects, they may not be acceptable.  In this case it may be preferable to merge the co-planar islands with
  // other islands that it 'touches'.
  virtual PxU32 mergeCoplanarIslands(const PxF32 *vertices) = 0;
  virtual PxU32 mergeCoplanarIslands(const PxF64 *vertices) = 0;

  virtual PxU32 mergeTouchingIslands(const PxF32 *vertices) = 0;
  virtual PxU32 mergeTouchingIslands(const PxF64 *vertices) = 0;

  virtual PxU32 *   getIsland(PxU32 index,PxU32 &tcount) = 0;


};

MeshIslandGeneration * createMeshIslandGeneration(void);
void                   releaseMeshIslandGeneration(MeshIslandGeneration *cm);

}; // end of namespace
	using namespace general_meshutils2;
};

#endif
