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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_CUBE_INDEX_H
#define GU_CUBE_INDEX_H

#include "foundation/PxVec3.h"
#include "CmPhysXCommon.h"
#include "PsFPU.h"

namespace physx
{

	enum CubeIndex
	{
		CUBE_RIGHT,
		CUBE_LEFT,
		CUBE_TOP,
		CUBE_BOTTOM,
		CUBE_FRONT,
		CUBE_BACK,

		CUBE_FORCE_DWORD	= 0x7fffffff
	};

	/*
		It's pretty straightforwards in concept (though the execution in hardware is
		a bit crufty and complex). You use a 3D texture coord to look up a texel in
		a cube map. First you find which of the axis has the largest value (i.e.
		X,Y,Z), and then the sign of that axis decides which face you are going to
		use. Which is why the faces are called +X, -X, +Y, -Y, +Z, -Z - after their
		principle axis. Then you scale the vector so that the largest value is +/-1.
		Then use the other two as 2D coords to look up your texel (with a 0.5 scale
		& offset).

		For example, vector (0.4, -0.2, -0.5). Largest value is the Z axis, and it's
		-ve, so we're reading from the -Z map. Scale so that this Z axis is +/-1,
		and you get the vector (0.8, -0.4, -1.0). So now use the other two values to
		look up your texel. So we look up texel (0.8, -0.4). The scale & offset move
		the -1->+1 range into the usual 0->1 UV range, so we actually look up texel
		(0.9, 0.3). The filtering is extremely complex, especially where three maps
		meet, but that's a hardware problem :-)
	*/

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	 *	Cubemap lookup function.
	 *
	 *	To transform returned uvs into mapping coordinates :
	 *	u += 1.0f;	u *= 0.5f;
	 *	v += 1.0f;	v *= 0.5f;
	 *
	 *	\fn			CubemapLookup(const PxVec3& direction, float& u, float& v)
	 *	\param		direction	[in] a direction vector
	 *	\param		u			[out] impact coordinate on the unit cube, in [-1,1]
	 *	\param		v			[out] impact coordinate on the unit cube, in [-1,1]
	 *	\return		cubemap texture index
	 */
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PX_INLINE CubeIndex		CubemapLookup(const PxVec3& direction, float& u, float& v);

	PX_INLINE PxU32 ComputeCubemapOffset(const PxVec3& dir, PxU32 subdiv)
	{
		float u,v;
		const CubeIndex CI = CubemapLookup(dir, u, v);

		// Remap to [0, subdiv[
		const float Coeff = 0.5f * float(subdiv-1);
		u += 1.0f;	u *= Coeff;
		v += 1.0f;	v *= Coeff;

		// Compute offset
		return PxU32(CI)*(subdiv*subdiv) + PxU32(u)*subdiv + PxU32(v);
	}


	PX_INLINE PxU32 ComputeCubemapNearestOffset(const PxVec3& dir, PxU32 subdiv)
	{
		float u,v;
		const CubeIndex CI = CubemapLookup(dir, u, v);

		// Remap to [0, subdiv]
		const float Coeff = 0.5f * float(subdiv-1);
		u += 1.0f;	u *= Coeff;
		v += 1.0f;	v *= Coeff;

		// Compute offset
		return PxU32(CI)*(subdiv*subdiv) + PxU32(u + 0.5f)*subdiv + PxU32(v + 0.5f);
	}


	PX_INLINE CubeIndex CubemapLookup(const PxVec3& direction, float& u, float& v)
	{
		const PxU32* binary = reinterpret_cast<const PxU32*>(&direction.x);

		const PxU32 absPx = binary[0] & ~PX_SIGN_BITMASK;
		const PxU32 absNy = binary[1] & ~PX_SIGN_BITMASK;
		const PxU32 absNz = binary[2] & ~PX_SIGN_BITMASK;

		PxU32 Index1 = 0;	//x biggest axis
		PxU32 Index2 = 1;
		PxU32 Index3 = 2;
		if( (absNy > absPx) & (absNy > absNz))
		{
			//y biggest
			Index2 = 2;
			Index3 = 0;
			Index1 = 1;
		}
		else if(absNz > absPx)
		{
			//z biggest
			Index2 = 0;
			Index3 = 1;
			Index1 = 2;
		}

		const PxF32* data = &direction.x;
		const float Coeff = 1.0f / fabsf(data[Index1]);
		u = data[Index2] * Coeff;
		v = data[Index3] * Coeff;

		const PxU32 Sign = binary[Index1]>>31;
		return CubeIndex(Sign|(Index1+Index1));
	}

}

#endif
