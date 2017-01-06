/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NV_PARAMETERIZED_CONVERSION_TEMPLATE_H
#define NV_PARAMETERIZED_CONVERSION_TEMPLATE_H

#include <NvTraitsInternal.h>
#include <nvparameterized/NvParameterizedTraits.h>

namespace NvParameterized
{
/**
\brief Class to handle all the redundant part of version upgrades.

It verifies class names and versions, and it runs the default converter. The user may overload
convert(), getPreferredVersions() and release() methods.
*/
template<typename Told, typename Tnew, uint32_t oldVersion, uint32_t newVersion>
class ParamConversionTemplate : public NvParameterized::Conversion
{
public:
	typedef Told TOldClass;
	typedef Tnew TNewClass;

	bool operator()(NvParameterized::Interface& legacyObj, NvParameterized::Interface& obj)
	{
		if (!mDefaultConversion)
		{
			mDefaultConversion = NvParameterized::internalCreateDefaultConversion(mTraits, getPreferredVersions());
		}

		// verify class names
		if (physx::shdfnd::strcmp(legacyObj.className(), Told::staticClassName()) != 0)
		{
			return false;
		}
		if (physx::shdfnd::strcmp(obj.className(), Tnew::staticClassName()) != 0)
		{
			return false;
		}

		// verify version
		if (legacyObj.version() != oldVersion)
		{
			return false;
		}
		if (obj.version() != newVersion)
		{
			return false;
		}

		//Copy unchanged fields
		if (!(*mDefaultConversion)(legacyObj, obj))
		{
			return false;
		}

		mLegacyData = static_cast<Told*>(&legacyObj);
		mNewData = static_cast<Tnew*>(&obj);

		if (!convert())
		{
			return false;
		}

		NvParameterized::Handle invalidHandle(mNewData);
		if (!mNewData->areParamsOK(&invalidHandle, 1))
		{
			if (invalidHandle.isValid())
			{
				char buf[256];				
				physx::shdfnd::strlcpy(buf, 256, "First invalid item: ");
				invalidHandle.getLongName(buf + strlen("First invalid item: "), 256UL - static_cast<uint32_t>(strlen("First invalid item: ")));
				mTraits->traitsWarn(buf);
			}
			return false;
		}

		return true;
	}

	/// User code, frees itself with the traits, and also calls destroy() on the ParamConversionTemplate object
	virtual void release()
	{
		destroy();
		mTraits->free(this);
	}

protected:
	ParamConversionTemplate(NvParameterized::Traits* traits)
		: mTraits(traits), mDefaultConversion(0), mLegacyData(0), mNewData(0)
	{
		// Virtual method getPreferredVersions() can not be called in constructors
		// so we defer construction of mDefaultConversion
	}

	~ParamConversionTemplate()
	{
		destroy();
	}

	/// User code, return list of preferred versions.
	virtual const NvParameterized::PrefVer* getPreferredVersions() const
	{
		return 0;
	}

	/// User code, return true if conversion is successful.
	virtual bool convert()
	{
		return true;
	}

	void destroy()
	{
		if (mDefaultConversion)
		{
			mDefaultConversion->release();
		}
	}


	NvParameterized::Traits*			mTraits;
	NvParameterized::Conversion*		mDefaultConversion;

	Told*							mLegacyData;
	Tnew*							mNewData;
};
}

// Force inclusion of files with initParamRef before we redefine it below
#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParamUtils.h"
#include "NvParameters.h"
// Do not call initParamRef in converter - prefer Traits::createNvParameterized with explicit version
// (see wiki for more details)
#define initParamRef DO_NOT_USE_ME

#endif // PARAM_CONVERSION_TEMPLATE_H
