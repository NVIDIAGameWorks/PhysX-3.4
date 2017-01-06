/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_BASIC_IOS_H
#define MODULE_BASIC_IOS_H

#include "Apex.h"
#include "TestBase.h"
#include <limits.h>

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class Scene;
class BasicIosAsset;
class BasicIosActor;
class BasicIosAssetAuthoring;

/**
\brief BasicIOS Module
*/
class ModuleBasicIos : public Module
{
protected:
	virtual										~ModuleBasicIos() {}

public:
	/// Get BasicIOS authoring type name
	virtual const char*							getBasicIosTypeName() = 0;

	/// Get TestBase implementation of BasicIos scene
	virtual const TestBase*						getTestBase(Scene* apexScene) const = 0;

};


PX_POP_PACK

}
} // namespace nvidia

#endif // MODULE_BASIC_IOS_H
