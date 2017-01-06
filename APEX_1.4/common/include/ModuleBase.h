/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_BASE_H
#define MODULE_BASE_H

#include "ApexResource.h"
#include "ApexString.h"
#include "ApexSDKIntl.h"
#include "PsArray.h"

namespace nvidia
{
namespace apex
{

class ModuleBase : public UserAllocated
{
public:
	ModuleBase();
	void release();

	const char* getName() const;

	/* Framework internal ModuleIntl class methods */
	void destroy();

	ApexSDKIntl* mSdk;

protected:
	ApexSimpleString mName;
	Module* mApiProxy;
};

}
} // end namespace nvidia::apex

#endif // MODULE_BASE_H
