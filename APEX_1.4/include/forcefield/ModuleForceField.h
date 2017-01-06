/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_FORCE_FIELD_H
#define MODULE_FORCE_FIELD_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class ForceFieldAsset;
class ForceFieldAssetAuthoring;

/**
\brief ForceField Module
*/
class ModuleForceField : public Module
{
public:
	/**
	\brief The module ID value of the force field.
	*/
	virtual uint32_t getModuleValue() const = 0;

protected:
	/**
	\brief Force field module default destructor.
	*/
	virtual ~ModuleForceField() {}
};



PX_POP_PACK

} // namespace apex
} // namespace nvidia

#endif // MODULE_FORCE_FIELD_H
