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


#ifndef PX_MIDPHASE_DESC_H
#define PX_MIDPHASE_DESC_H
/** \addtogroup cooking
@{
*/

#include "geometry/PxTriangleMesh.h"
#include "cooking/PxBVH33MidphaseDesc.h"
#include "cooking/PxBVH34MidphaseDesc.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/**

\brief Structure describing parameters affecting midphase mesh structure.

@see PxCookingParams, PxBVH33MidphaseDesc, PxBVH34MidphaseDesc
*/
class PxMidphaseDesc
{
public:
	PX_FORCE_INLINE PxMidphaseDesc(): mType(PxMeshMidPhase::eINVALID) { }

	/**
	\brief	Returns type of midphase mesh structure.
	\return	PxMeshMidPhase::Enum 

	@see PxMeshMidPhase::Enum
	*/
	PX_FORCE_INLINE PxMeshMidPhase::Enum getType() const { return mType; }

	/**
	\brief	Midphase descriptors union

	@see PxBV33MidphaseDesc, PxBV34MidphaseDesc
	*/
	union {		
		PxBVH33MidphaseDesc  mBVH33Desc;
		PxBVH34MidphaseDesc  mBVH34Desc;
    };

	/**
	\brief	Initialize the midphase mesh structure descriptor
	\param[in] type Midphase mesh structure descriptor

	@see PxBV33MidphaseDesc, PxBV34MidphaseDesc
	*/
	void setToDefault(PxMeshMidPhase::Enum type)
	{
		mType = type;
		if(type==PxMeshMidPhase::eBVH33)
			mBVH33Desc.setToDefault();
		else if(type==PxMeshMidPhase::eBVH34)
			mBVH34Desc.setToDefault();
	}

	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid.
	*/
	bool isValid() const
	{		
		if(mType==PxMeshMidPhase::eBVH33)
			return mBVH33Desc.isValid();
		else if(mType==PxMeshMidPhase::eBVH34)
			return mBVH34Desc.isValid();
		return true;
	}

	PX_FORCE_INLINE PxMidphaseDesc&		operator=(PxMeshMidPhase::Enum descType) 
	{ 
		setToDefault(descType);
		return *this; 
	}

protected:	
	PxMeshMidPhase::Enum	mType;
};

#if !PX_DOXYGEN
} // namespace physx
#endif


  /** @} */
#endif // PX_MIDPHASE_DESC_UNION_H
