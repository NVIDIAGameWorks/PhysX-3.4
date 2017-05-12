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

#ifndef GU_VEC_CONVEX_H
#define GU_VEC_CONVEX_H

#include "CmPhysXCommon.h"
#include "PsVecMath.h"

#define PX_SUPPORT_INLINE PX_FORCE_INLINE
#define PX_SUPPORT_FORCE_INLINE PX_FORCE_INLINE

namespace physx
{
namespace Gu
{

	struct ConvexType
	{
		enum Type
		{
			eCONVEXHULL = 0,
			eCONVEXHULLNOSCALE = 1,
			eSPHERE = 2,
			eBOX = 3,
			eCAPSULE = 4,
			eTRIANGLE = 5
		};
	};
	
	class ConvexV
	{
	public:

		PX_FORCE_INLINE ConvexV(const ConvexType::Type type_) : type(type_), bMarginIsRadius(false)
		{
			margin = 0.f;
			minMargin = 0.f;
			sweepMargin = 0.f;
			center = Ps::aos::V3Zero();
			maxMargin = PX_MAX_F32;
		}

		PX_FORCE_INLINE ConvexV(const ConvexType::Type type_, const Ps::aos::Vec3VArg center_) : type(type_), bMarginIsRadius(false)
		{
			using namespace Ps::aos;
			center = center_;
			margin = 0.f;
			minMargin = 0.f;
			sweepMargin = 0.f;
			maxMargin = PX_MAX_F32;
		}

		//everytime when someone transform the object, they need to up
		PX_FORCE_INLINE void setCenter(const Ps::aos::Vec3VArg _center)
		{
			center = _center;
		}

		PX_FORCE_INLINE void setMargin(const Ps::aos::FloatVArg margin_)
		{
			Ps::aos::FStore(margin_, &margin);
		}

		PX_FORCE_INLINE void setMargin(const PxReal margin_)
		{
			margin = margin_;
			//compare with margin and choose the smallest one
			margin = PxMin<PxReal>(maxMargin, margin);
		}

		PX_FORCE_INLINE void setMaxMargin(const PxReal maxMargin_)
		{
			maxMargin = maxMargin_;
			//compare with margin and choose the smallest one
			margin = PxMin<PxReal>(maxMargin, margin);
		}

		PX_FORCE_INLINE void setMinMargin(const Ps::aos::FloatVArg minMargin_)
		{
			Ps::aos::FStore(minMargin_, & minMargin);
		}

		PX_FORCE_INLINE void setSweepMargin(const Ps::aos::FloatVArg sweepMargin_)
		{
			Ps::aos::FStore(sweepMargin_, &sweepMargin);
		}

		PX_FORCE_INLINE Ps::aos::Vec3V getCenter()const 
		{
			return center;
		}

		PX_FORCE_INLINE Ps::aos::FloatV getMargin() const
		{
			return Ps::aos::FLoad(margin);
		}

		PX_FORCE_INLINE Ps::aos::FloatV getMinMargin() const
		{
			return Ps::aos::FLoad(minMargin);
		}

		PX_FORCE_INLINE Ps::aos::FloatV getSweepMargin() const
		{
			return Ps::aos::FLoad(sweepMargin);
		}

		PX_FORCE_INLINE ConvexType::Type getType() const
		{
			return type;
		}

		PX_FORCE_INLINE Ps::aos::BoolV isMarginEqRadius()const
		{
			return Ps::aos::BLoad(bMarginIsRadius);
		}

		PX_FORCE_INLINE bool getMarginIsRadius() const
		{
			return bMarginIsRadius;
		}

		PX_FORCE_INLINE PxReal getMarginF() const
		{
			return margin;
		}


	protected:
		~ConvexV(){}
		Ps::aos::Vec3V center;
		PxReal margin;				//margin is the amount by which we shrunk the shape for a convex or box. If the shape are sphere/capsule, margin is the radius
		PxReal minMargin;			//minMargin is some percentage of marginBase, which is used to determine the termination condition for gjk
		PxReal sweepMargin;			//sweepMargin minMargin is some percentage of marginBase, which is used to determine the termination condition for gjkRaycast
		PxReal maxMargin;			//the shrunk amount defined by the application
		ConvexType::Type	type;
		bool bMarginIsRadius;
	};

	PX_FORCE_INLINE Ps::aos::FloatV getContactEps(const Ps::aos::FloatV& _marginA, const Ps::aos::FloatV& _marginB)
	{
		using namespace Ps::aos;

		const FloatV ratio = FLoad(0.25f);
		const FloatV minMargin = FMin(_marginA, _marginB);
		
		return FMul(minMargin, ratio);
	}

	PX_FORCE_INLINE Ps::aos::FloatV getSweepContactEps(const Ps::aos::FloatV& _marginA, const Ps::aos::FloatV& _marginB)
	{
		using namespace Ps::aos;

		const FloatV ratio = FLoad(100.f);
		const FloatV minMargin = FAdd(_marginA, _marginB);
		
		return FMul(minMargin, ratio);
	}
}

}

#endif
