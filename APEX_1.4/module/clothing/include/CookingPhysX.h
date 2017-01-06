/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef COOKING_PHYS_X_H
#define COOKING_PHYS_X_H

#include "CookingAbstract.h"

namespace nvidia
{
namespace apex
{
template<class T>
class ParamArray;
}
namespace clothing
{


class CookingPhysX : public CookingAbstract
{
public:

	virtual NvParameterized::Interface* execute() { return 0;}

	static uint32_t getCookingVersion()
	{
#if PX_PHYSICS_VERSION_MAJOR == 3
		return PX_PHYSICS_VERSION;
#else
		return 0;
#endif
	}

protected:
};

}
} // namespace nvidia


#endif // COOKING_PHYS_X_H
