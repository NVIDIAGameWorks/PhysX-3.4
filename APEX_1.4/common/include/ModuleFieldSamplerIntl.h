/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_FIELD_SAMPLER_INTL_H
#define MODULE_FIELD_SAMPLER_INTL_H

#include "ModuleIntl.h"

namespace nvidia
{
namespace apex
{

class FieldSamplerManagerIntl;
class Scene;

class ModuleFieldSamplerIntl : public ModuleIntl
{
public:
	virtual	FieldSamplerManagerIntl*	getInternalFieldSamplerManager(const Scene& apexScene) = 0;
};

}
} // end namespace nvidia::apex

#endif // MODULE_FIELD_SAMPLER_INTL_H
