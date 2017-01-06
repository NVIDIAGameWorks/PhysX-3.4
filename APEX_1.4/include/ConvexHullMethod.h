/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVEX_HULL_METHOD_H
#define CONVEX_HULL_METHOD_H

/*!
\file
\brief Misc utility classes
*/

#include "Module.h"
#include "foundation/PxMath.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT


/**
\brief Method by which chunk mesh collision hulls are generated.
*/
struct ConvexHullMethod
{
	/**
	\brief Enum of methods by which chunk mesh collision hulls are generated.
	*/
	enum Enum
	{
		USE_6_DOP,
		USE_10_DOP_X,
		USE_10_DOP_Y,
		USE_10_DOP_Z,
		USE_14_DOP_XY,
		USE_14_DOP_YZ,
		USE_14_DOP_ZX,
		USE_18_DOP,
		USE_26_DOP,
		WRAP_GRAPHICS_MESH,
		CONVEX_DECOMPOSITION,

		COUNT
	};
};



PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // CONVEX_HULL_METHOD_H
