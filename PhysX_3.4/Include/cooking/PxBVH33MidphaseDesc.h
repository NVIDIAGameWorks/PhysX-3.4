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


#ifndef PX_BVH_33_MIDPHASE_DESC_H
#define PX_BVH_33_MIDPHASE_DESC_H
/** \addtogroup cooking
@{
*/

#include "foundation/PxPreprocessor.h"
#include "foundation/PxSimpleTypes.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/** \brief Enumeration for mesh cooking hints. */
struct PxMeshCookingHint
{
	enum Enum
	{
		eSIM_PERFORMANCE = 0,		//!< Default value. Favors higher quality hierarchy with higher runtime performance over cooking speed.
		eCOOKING_PERFORMANCE = 1	//!< Enables fast cooking path at the expense of somewhat lower quality hierarchy construction.
	};
};

/**

\brief Structure describing parameters affecting BVH33 midphase mesh structure.

@see PxCookingParams, PxMidphaseDesc
*/
struct PxBVH33MidphaseDesc
{
	/**
	\brief Controls the trade-off between mesh size and runtime performance.

	Using a value of 1.0 will produce a larger cooked mesh with generally higher runtime performance,
	using 0.0 will produce a smaller cooked mesh, with generally lower runtime performance.

	Values outside of [0,1] range will be clamped and cause a warning when any mesh gets cooked.

	<b>Default value:</b> 0.55
	<b>Range:</b> [0.0f, 1.0f]
	*/
	PxF32							meshSizePerformanceTradeOff;

	/**
	\brief Mesh cooking hint. Used to specify mesh hierarchy construction preference.

	<b>Default value:</b> PxMeshCookingHint::eSIM_PERFORMANCE
	*/
	PxMeshCookingHint::Enum			meshCookingHint;

	/**
	\brief Desc initialization to default value.
	*/
    void setToDefault()
    {
	    meshSizePerformanceTradeOff = 0.55f;
		meshCookingHint = PxMeshCookingHint::eSIM_PERFORMANCE;
    }

	/**
	\brief Returns true if the descriptor is valid.
	\return true if the current settings are valid.
	*/
	bool isValid() const
	{
		if(meshSizePerformanceTradeOff < 0.0f || meshSizePerformanceTradeOff > 1.0f)
			return false;
		return true;
	}
};

#if !PX_DOXYGEN
} // namespace physx
#endif


  /** @} */
#endif // PX_BVH_33_MIDPHASE_DESC_H
