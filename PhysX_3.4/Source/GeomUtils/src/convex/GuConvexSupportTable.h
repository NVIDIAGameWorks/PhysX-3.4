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

#ifndef GU_CONVEX_SUPPORT_TABLE_H
#define GU_CONVEX_SUPPORT_TABLE_H

#include "GuVecConvex.h"
#include "PsVecTransform.h"
#include "PxPhysXCommonConfig.h"

namespace physx
{
namespace Gu  
{

	class TriangleV; 
	class CapsuleV;
	class BoxV;
	class ConvexHullV;
	class ConvexHullNoScaleV;

	
#if PX_VC 
    #pragma warning(push)   
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif
	class SupportLocal
	{
	public:		
		Ps::aos::Vec3V shapeSpaceCenterOfMass;
		const Ps::aos::PsTransformV& transform;
		const Ps::aos::Mat33V& vertex2Shape;
		const Ps::aos::Mat33V& shape2Vertex;
		const bool isIdentityScale;	

		SupportLocal(const Ps::aos::PsTransformV& _transform, const Ps::aos::Mat33V& _vertex2Shape, const Ps::aos::Mat33V& _shape2Vertex, const bool _isIdentityScale = true): transform(_transform), 
			vertex2Shape(_vertex2Shape), shape2Vertex(_shape2Vertex), isIdentityScale(_isIdentityScale)
		{
		}

		PX_FORCE_INLINE void setShapeSpaceCenterofMass(const Ps::aos::Vec3VArg _shapeSpaceCenterOfMass)
		{
			shapeSpaceCenterOfMass = _shapeSpaceCenterOfMass;
		}
        virtual ~SupportLocal() {}
		virtual Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const = 0;
		virtual void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max) const = 0;
		virtual void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts)const = 0;
	
	protected:
		SupportLocal& operator=(const SupportLocal&);
	};
#if PX_VC 
     #pragma warning(pop) 
#endif

	template <typename Convex>
	class SupportLocalImpl : public SupportLocal
	{
		
	public:
		const Convex& conv;
		SupportLocalImpl(const Convex& _conv, const Ps::aos::PsTransformV& _transform, const Ps::aos::Mat33V& _vertex2Shape, const Ps::aos::Mat33V& _shape2Vertex, const bool _isIdentityScale = true) : 
		SupportLocal(_transform, _vertex2Shape, _shape2Vertex, _isIdentityScale), conv(_conv)
		{
		}

		Ps::aos::Vec3V doSupport(const Ps::aos::Vec3VArg dir) const
		{
			//return conv.supportVertsLocal(dir);
			return conv.supportLocal(dir);
		}

		void doSupport(const Ps::aos::Vec3VArg dir, Ps::aos::FloatV& min, Ps::aos::FloatV& max) const
		{
			return conv.supportLocal(dir, min, max);
		}

		void populateVerts(const PxU8* inds, PxU32 numInds, const PxVec3* originalVerts, Ps::aos::Vec3V* verts) const 
		{
			conv.populateVerts(inds, numInds, originalVerts, verts);
		}

	protected:
		SupportLocalImpl& operator=(const SupportLocalImpl&);

	};
}

}

#endif
