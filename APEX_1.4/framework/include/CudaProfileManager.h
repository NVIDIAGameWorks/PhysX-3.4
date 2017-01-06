/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CUDA_PROFILE_MANAGER_H
#define CUDA_PROFILE_MANAGER_H

/*!
\file
\brief classes CudaProfileManager
*/

#include <ApexDefs.h>
#include <PxSimpleTypes.h>

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Interface for options of ApexCudaProfileManager
 */
class CudaProfileManager
{
public:
	/**
	 * Normalized time unit for profile data
	 */
	enum TimeFormat
	{
		MILLISECOND = 1,
		MICROSECOND = 1000,
		NANOSECOND = 1000000
	};

	 /**
	\brief Set path for writing results
	*/
	virtual void setPath(const char* path) = 0;
	 /**
	\brief Set kernel for profile
	*/
	virtual void setKernel(const char* functionName, const char* moduleName) = 0;
	/**
	\brief Set normailized time unit
	*/
	virtual void setTimeFormat(TimeFormat tf) = 0;
	/**
	\brief Set state (on/off) for profile manager
	*/
	virtual void enable(bool state) = 0;
	/**
	\brief Get state (on/off) of profile manager
	*/
	virtual bool isEnabled() const = 0;
};

PX_POP_PACK

}
}

#endif // CUDA_PROFILE_MANAGER_H

