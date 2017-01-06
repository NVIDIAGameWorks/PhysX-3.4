/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_IOFX_LEGACY_H
#define MODULE_IOFX_LEGACY_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

class IofxRenderCallback;

PX_PUSH_PACK_DEFAULT

/**
\brief ModuleIofx interface used in APEXSDK before v. 1.4

This interface depricated in APEX 1.4
*/
class ModuleIofxLegacy : public Module
{
protected:
	virtual ~ModuleIofxLegacy() {}

public:
	
	/**
	\brief Returns IOFX render callback
	*/
	virtual IofxRenderCallback* getIofxLegacyRenderCallback(const Scene& apexScene) = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // MODULE_IOFX_LEGACY_H
