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

#ifndef GU_MTD_H
#define GU_MTD_H

#include "PxGeometry.h"

namespace physx
{
namespace Gu
{
	// PT: we use a define to be able to quickly change the signature of all MTD functions.
	// (this also ensures they all use consistent names for passed parameters).
	// \param[out]	mtd		computed depenetration dir
	// \param[out]	depth	computed depenetration depth
	// \param[in]	geom0	first geometry object
	// \param[in]	pose0	pose of first geometry object
	// \param[in]	geom1	second geometry object
	// \param[in]	pose1	pose of second geometry object
	// \param[in]	cache	optional cached data for triggers
	#define GU_MTD_FUNC_PARAMS	PxVec3& mtd, PxF32& depth,							\
								const PxGeometry& geom0, const PxTransform& pose0,	\
								const PxGeometry& geom1, const PxTransform& pose1

	// PT: function pointer for Geom-indexed MTD functions
	// See GU_MTD_FUNC_PARAMS for function parameters details.
	// \return		true if an overlap was found, false otherwise
	// \note		depenetration vector D is equal to mtd * depth. It should be applied to the 1st object, to get out of the 2nd object.
	typedef bool (*GeomMTDFunc)	(GU_MTD_FUNC_PARAMS);
}
}

#endif
