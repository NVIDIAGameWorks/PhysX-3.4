/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_IOFX_INTL_H
#define MODULE_IOFX_INTL_H

#include "ModuleIntl.h"

namespace nvidia
{
namespace apex
{

class IofxManagerDescIntl;
class IofxManagerIntl;
class Scene;

class ModuleIofxIntl : public ModuleIntl
{
public:
	virtual IofxManagerIntl*	createActorManager(const Scene& scene, const nvidia::apex::IofxAsset& asset, const IofxManagerDescIntl& desc) = 0;
};

}
} // end namespace nvidia::apex

#endif // MODULE_IOFX_INTL_H
