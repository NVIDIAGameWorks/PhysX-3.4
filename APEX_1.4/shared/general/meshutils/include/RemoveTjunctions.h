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

#ifndef REMOVE_TJUNCTIONS_H

#define REMOVE_TJUNCTIONS_H

#include "Ps.h"
#include "foundation/PxSimpleTypes.h"

namespace physx
{
	namespace general_meshutils2
	{

class RemoveTjunctionsDesc
{
public:
  RemoveTjunctionsDesc(void)
  {
    mVcount = 0;
    mVertices = 0;
    mTcount = 0;
    mIndices = 0;
    mIds = 0;
    mTcountOut = 0;
    mIndicesOut = 0;
    mIdsOut = 0;
	mEpsilon = 0.00000001f;
  }

// input
  PxF32        mEpsilon;
  PxF32        mDistanceEpsilon;
  PxU32        mVcount;  // input vertice count.
  const PxF32 *mVertices; // input vertices as PxF32s or...
  PxU32        mTcount;    // number of input triangles.
  const PxU32 *mIndices;   // triangle indices.
  const PxU32 *mIds;       // optional triangle Id numbers.
// output..
  PxU32        mTcountOut;  // number of output triangles.
  const PxU32 *mIndicesOut; // output triangle indices
  const PxU32 *mIdsOut;     // output retained id numbers.
};

// Removes t-junctions from an input mesh.  Does not generate any new data points, but may possible produce additional triangles and new indices.
class RemoveTjunctions
{
public:

   virtual PxU32 removeTjunctions(RemoveTjunctionsDesc &desc) =0; // returns number of triangles output and the descriptor is filled with the appropriate results.


};

RemoveTjunctions * createRemoveTjunctions(void);
void               releaseRemoveTjunctions(RemoveTjunctions *tj);

	};
	using namespace general_meshutils2;
};

#endif
