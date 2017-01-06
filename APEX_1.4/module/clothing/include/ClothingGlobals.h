/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_GLOBALS_H
#define CLOTHING_GLOBALS_H

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace clothing
{


struct ClothingConstants
{
	enum Enum
	{
		ImmediateClothingInSkinFlag =		0x80000000, // only highest (sign?) bit. The rest is the index in the clothSkinMapB
		ImmediateClothingInvertNormal =		0x40000000, // if second highest bit is set, invert the normal from Cloth
		ImmediateClothingBadNormal =		0x20000000, // the normal is neither correct nor inverted, just different, use mesh-mesh skinning from neighboring triangles
		ImmediateClothingInvalidValue =		0x1fffffff, // the lowest bit is set, all others are maxed out
		ImmediateClothingReadMask =			0x0fffffff, // read mask, use this to read the number (two flags can still be put there so far)
	};
};

}
} // namespace nvidia


#endif // CLOTHING_GLOBALS_H
