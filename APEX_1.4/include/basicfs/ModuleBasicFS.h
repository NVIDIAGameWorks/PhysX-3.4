/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MODULE_BASIC_FS_H
#define MODULE_BASIC_FS_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class BasicFSAsset;
class BasicFSAssetAuthoring;


/**
 \brief BasicFS module class
 */
class ModuleBasicFS : public Module
{
public:

protected:
	virtual ~ModuleBasicFS() {}
};


PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // MODULE_BASIC_FS_H
