/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef HEADER_DebugColorParamsListener_h
#define HEADER_DebugColorParamsListener_h

#include "NvParameters.h"
#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParameterizedTraits.h"

#include "DebugColorParams.h"
#include "ApexSDKIntl.h"

namespace nvidia
{
namespace apex
{

#define MAX_COLOR_NAME_LENGTH 32

class DebugColorParamsEx : public DebugColorParams
{
public:
	DebugColorParamsEx(NvParameterized::Traits* traits, ApexSDKIntl* mSdk) :
		DebugColorParams(traits),
		mApexSdk(mSdk) {}

	~DebugColorParamsEx()
	{
	}

	void destroy()
	{
		this->~DebugColorParamsEx();
		this->DebugColorParams::destroy();
	}

	NvParameterized::ErrorType setParamU32(const NvParameterized::Handle& handle, uint32_t val)
	{
		NvParameterized::ErrorType err = NvParameterized::NvParameters::setParamU32(handle, val);

		NvParameterized::Handle& h = const_cast<NvParameterized::Handle&>(handle);
		char color[MAX_COLOR_NAME_LENGTH];
		h.getLongName(color, MAX_COLOR_NAME_LENGTH);
		mApexSdk->updateDebugColorParams(color, val);

		return err;
	}

private:
	ApexSDKIntl* mApexSdk;
};

}
} // namespace nvidia::apex::

#endif