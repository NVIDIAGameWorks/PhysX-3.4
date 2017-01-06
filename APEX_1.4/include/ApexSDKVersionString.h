/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_SDKVERSION_STRING_H
#define APEX_SDKVERSION_STRING_H

/*!
\file
\brief APEX SDK versioning definitions
*/

#include "foundation/PxPreprocessor.h"
#include "ApexDefs.h"

//! \brief physx namespace
namespace nvidia
{
//! \brief apex namespace
namespace apex
{

/**
\brief These values are used to select version string GetApexSDKVersionString function should return
*/
enum ApexSDKVersionString
{
	/**
	\brief APEX version
	*/
	VERSION = 0,
	/**
	\brief APEX build changelist
	*/
	CHANGELIST = 1,
	/**
	\brief APEX tools build changelist
	*/
	TOOLS_CHANGELIST = 2,
	/**
	\brief APEX branch used to create build 
	*/
	BRANCH = 3,
	/**
	\brief Time at which the build was created
	*/
	BUILD_TIME = 4,
	/**
	\brief Author of the build
	*/
	AUTHOR = 5,
	/**
	\brief Reason to prepare the build
	*/
	REASON = 6
};

/**
\def APEX_API
\brief Export the function declaration from its DLL
*/

/**
\def CALL_CONV
\brief Use C calling convention, required for exported functions
*/

#ifdef CALL_CONV
#undef CALL_CONV
#endif

#if PX_WINDOWS_FAMILY
#define APEX_API extern "C" __declspec(dllexport)
#define CALL_CONV __cdecl
#else
#define APEX_API extern "C"
#define CALL_CONV /* void */
#endif

/**
\brief Returns version strings
*/
APEX_API const char*	CALL_CONV GetApexSDKVersionString(ApexSDKVersionString versionString);

}
} // end namespace nvidia::apex

#endif // APEX_SDKVERSION_STRING_H
