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


#ifndef DY_SPATIAL_H
#define DY_SPATIAL_H

#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "PsMathUtils.h"
#include "CmSpatialVector.h"

namespace physx
{
namespace Dy
{
// translate a motion resolved at position p to the origin


// should have a 'from' frame and a 'to' frame
class SpInertia
{
public:
	SpInertia() {}

	SpInertia(const PxMat33& ll, const PxMat33& la, const PxMat33& aa): mLL(ll), mLA(la), mAA(aa)
	{
	}

	static SpInertia getZero()
	{
		return SpInertia(PxMat33(PxZero), PxMat33(PxZero), 
							     PxMat33(PxZero));
	}

	static SpInertia dyad(const Cm::SpatialVector& column, const Cm::SpatialVector& row) 
	{
		return SpInertia(dyad(column.linear, row.linear),  
						 dyad(column.linear, row.angular),  
					     dyad(column.angular, row.angular));
	}


	static SpInertia inertia(PxReal mass, const PxVec3& inertia)
	{
		return SpInertia(PxMat33::createDiagonal(PxVec3(mass,mass,mass)), PxMat33(PxZero),			 
							     PxMat33::createDiagonal(inertia));
	}


	SpInertia operator+(const SpInertia& m) const
	{
		return SpInertia(mLL+m.mLL, mLA+m.mLA, mAA+m.mAA);
	}

	SpInertia operator-(const SpInertia& m) const
	{
		return SpInertia(mLL-m.mLL, mLA-m.mLA, mAA-m.mAA);
	}

	SpInertia operator*(PxReal r) const
	{
		return SpInertia(mLL*r, mLA*r, mAA*r);
	}

	void operator+=(const SpInertia& m)
	{
		mLL+=m.mLL; 
		mLA+=m.mLA;		
		mAA+=m.mAA;
	}

	void operator-=(const SpInertia& m)
	{
		mLL-=m.mLL; 
		mLA-=m.mLA;		
		mAA-=m.mAA;
	}


	PX_FORCE_INLINE Cm::SpatialVector operator *(const Cm::SpatialVector& v) const
	{
		return Cm::SpatialVector(mLL*v.linear            +mLA*v.angular,
		 					    mLA.transformTranspose(v.linear)+mAA*v.angular);
	}

	SpInertia operator *(const SpInertia& v) const
	{
		return SpInertia(mLL*v.mLL             + mLA * v.mLA.getTranspose(), 
						 mLL*v.mLA             + mLA * v.mAA,
						 mLA.getTranspose()*v.mLA + mAA * v.mAA);
	}


	bool isFinite() const
	{
		return true;
//		return mLL.isFinite() && mLA.isFinite() && mAA.isFinite(); 
	}

	PxMat33 mLL, mLA;		// linear force from angular motion, linear force from linear motion
	PxMat33 mAA;		    // angular force from angular motion, mAL = mLA.transpose()

private:
	static PxMat33 dyad(PxVec3 col, PxVec3 row)	
	{ 
		return PxMat33(col*row.x, col*row.y, col*row.z); 
	}


};

}
}

#endif //DY_SPATIAL_H
