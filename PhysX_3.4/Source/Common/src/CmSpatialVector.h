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


#ifndef PX_PHYSICS_COMMON_VECTOR
#define PX_PHYSICS_COMMON_VECTOR

#include "foundation/PxVec3.h"
#include "CmPhysXCommon.h"
#include "PsVecMath.h"

/*!
Combination of two R3 vectors.
*/

namespace physx
{
namespace Cm
{
PX_ALIGN_PREFIX(16)
class SpatialVector
{
public:
	//! Default constructor
	PX_FORCE_INLINE SpatialVector()
	{}

	//! Construct from two PxcVectors
	PX_FORCE_INLINE SpatialVector(const PxVec3& lin, const PxVec3& ang)
		: linear(lin), pad0(0.0f), angular(ang), pad1(0.0f)
	{
	}

	PX_FORCE_INLINE ~SpatialVector()
	{}



	// PT: this one is very important. Without it, the Xbox compiler generates weird "float-to-int" and "int-to-float" LHS
	// each time we copy a SpatialVector (see for example PIX on "solveSimpleGroupA" without this operator).
	PX_FORCE_INLINE	void	operator = (const SpatialVector& v)
	{
		linear = v.linear;
		pad0 = 0.0f;
		angular = v.angular;
		pad1 = 0.0f;
	}


	static PX_FORCE_INLINE SpatialVector zero() {	return SpatialVector(PxVec3(0),PxVec3(0)); }

	PX_FORCE_INLINE SpatialVector operator+(const SpatialVector& v) const
	{
		return SpatialVector(linear+v.linear,angular+v.angular);
	}

	PX_FORCE_INLINE SpatialVector operator-(const SpatialVector& v) const
	{
		return SpatialVector(linear-v.linear,angular-v.angular);
	}

	PX_FORCE_INLINE SpatialVector operator-() const
	{
		return SpatialVector(-linear,-angular);
	}


	PX_FORCE_INLINE SpatialVector operator *(PxReal s) const
	{	
		return SpatialVector(linear*s,angular*s);	
	}
		
	PX_FORCE_INLINE void operator+=(const SpatialVector& v)
	{
		linear+=v.linear;
		angular+=v.angular;
	}

	PX_FORCE_INLINE void operator-=(const SpatialVector& v)
	{
		linear-=v.linear;
		angular-=v.angular;
	}

	PX_FORCE_INLINE PxReal magnitude()	const
	{
		return angular.magnitude() + linear.magnitude();
	}

	PX_FORCE_INLINE PxReal dot(const SpatialVector& v) const
	{
		return linear.dot(v.linear) + angular.dot(v.angular);
	}
		
	PX_FORCE_INLINE bool isFinite() const
	{
		return linear.isFinite() && angular.isFinite();
	}

	PX_FORCE_INLINE Cm::SpatialVector scale(PxReal l, PxReal a) const
	{
		return Cm::SpatialVector(linear*l, angular*a);
	}

	PxVec3 linear;
	PxReal pad0;
	PxVec3 angular;
	PxReal pad1;
}
PX_ALIGN_SUFFIX(16);


PX_ALIGN_PREFIX(16)
struct SpatialVectorV
{
	Ps::aos::Vec3V linear;
	Ps::aos::Vec3V angular;

	PX_FORCE_INLINE SpatialVectorV() {}
	PX_FORCE_INLINE SpatialVectorV(PxZERO): linear(Ps::aos::V3Zero()), angular(Ps::aos::V3Zero()) {}
	PX_FORCE_INLINE SpatialVectorV(const Cm::SpatialVector& v): linear(Ps::aos::V3LoadU(v.linear)), angular(Ps::aos::V3LoadU(v.angular)) {}
	PX_FORCE_INLINE SpatialVectorV(const Ps::aos::Vec3VArg l, const Ps::aos::Vec3VArg a): linear(l), angular(a) {}
	PX_FORCE_INLINE SpatialVectorV(const SpatialVectorV& other): linear(other.linear), angular(other.angular) {}
	PX_FORCE_INLINE SpatialVectorV& operator=(const SpatialVectorV& other) { linear = other.linear; angular = other.angular; return *this; }

	PX_FORCE_INLINE SpatialVectorV operator+(const SpatialVectorV& other) const { return SpatialVectorV(Ps::aos::V3Add(linear,other.linear),
																								  Ps::aos::V3Add(angular, other.angular)); }
	
	PX_FORCE_INLINE SpatialVectorV& operator+=(const SpatialVectorV& other) { linear = Ps::aos::V3Add(linear,other.linear); 
																			  angular = Ps::aos::V3Add(angular, other.angular);
																			  return *this;
																			}
																								    
	PX_FORCE_INLINE SpatialVectorV operator-(const SpatialVectorV& other) const { return SpatialVectorV(Ps::aos::V3Sub(linear,other.linear),
																								  Ps::aos::V3Sub(angular, other.angular)); }
	
	PX_FORCE_INLINE SpatialVectorV operator-() const { return SpatialVectorV(Ps::aos::V3Neg(linear), Ps::aos::V3Neg(angular)); }

	PX_FORCE_INLINE SpatialVectorV operator*(Ps::aos::FloatVArg r) const { return SpatialVectorV(Ps::aos::V3Scale(linear,r), Ps::aos::V3Scale(angular,r)); }

	PX_FORCE_INLINE SpatialVectorV& operator-=(const SpatialVectorV& other) { linear = Ps::aos::V3Sub(linear,other.linear); 
																			  angular = Ps::aos::V3Sub(angular, other.angular);
																			  return *this;
																			}

	PX_FORCE_INLINE Ps::aos::FloatV dot(const SpatialVectorV& other) const { return Ps::aos::FAdd(Ps::aos::V3Dot(linear, other.linear), Ps::aos::V3Dot(angular, other.angular)); }

	
}PX_ALIGN_SUFFIX(16);

} // namespace Cm

PX_COMPILE_TIME_ASSERT(sizeof(Cm::SpatialVector) == 32);
PX_COMPILE_TIME_ASSERT(sizeof(Cm::SpatialVectorV) == 32);

}

#endif
