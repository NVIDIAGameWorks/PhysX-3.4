/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexAssetAuthoring.h"

#include "P4Info.h"
#include "PsString.h"

#include "PhysXSDKVersion.h"
#include "ApexSDKIntl.h"



namespace nvidia
{
namespace apex
{


void ApexAssetAuthoring::setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
{
#ifdef WITHOUT_APEX_AUTHORING
	PX_UNUSED(toolName);
	PX_UNUSED(toolVersion);
	PX_UNUSED(toolChangelist);
#else
	const uint32_t buflen = 256;
	char buf[buflen];
	nvidia::strlcpy(buf, buflen, toolName);
	nvidia::strlcat(buf, buflen, " ");

	if (toolVersion != NULL)
	{
		nvidia::strlcat(buf, buflen, toolVersion);
		nvidia::strlcat(buf, buflen, ":");
	}

	if (toolChangelist == 0)
	{
		toolChangelist = P4_TOOLS_CHANGELIST;
	}

	{
		char buf2[14];
		shdfnd::snprintf(buf2, 14, "CL %d", toolChangelist);
		nvidia::strlcat(buf, buflen, buf2);
		nvidia::strlcat(buf, buflen, " ");
	}
	
	{
#ifdef WIN64
		nvidia::strlcat(buf, buflen, "Win64 ");
#elif defined(WIN32)
		nvidia::strlcat(buf, buflen, "Win32 ");
#endif
	}

	{
		nvidia::strlcat(buf, buflen, "(Apex ");
		nvidia::strlcat(buf, buflen, P4_APEX_VERSION_STRING);
		char buf2[20];
		shdfnd::snprintf(buf2, 20, ", CL %d, ", P4_CHANGELIST);
		nvidia::strlcat(buf, buflen, buf2);
#ifdef _DEBUG
		nvidia::strlcat(buf, buflen, "DEBUG ");
#elif defined(PHYSX_PROFILE_SDK)
		nvidia::strlcat(buf, buflen, "PROFILE ");
#endif
		nvidia::strlcat(buf, buflen, P4_APEX_BRANCH);
		nvidia::strlcat(buf, buflen, ") ");
	}

	{
		nvidia::strlcat(buf, buflen, "(PhysX ");

		char buf2[10] = { 0 };
#if PX_PHYSICS_VERSION_MAJOR == 0
		shdfnd::snprintf(buf2, 10, "No) ");
#elif PX_PHYSICS_VERSION_MAJOR == 3
		shdfnd::snprintf(buf2, 10, "%d.%d) ", PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR);
#endif
		nvidia::strlcat(buf, buflen, buf2);
	}


	nvidia::strlcat(buf, buflen, "Apex Build Time: ");
	nvidia::strlcat(buf, buflen, P4_BUILD_TIME);
	
	nvidia::strlcat(buf, buflen, "Distribution author: ");
	nvidia::strlcat(buf, buflen, AUTHOR_DISTRO);
	
	nvidia::strlcat(buf, buflen, "The reason for the creation of the distribution: ");
	nvidia::strlcat(buf, buflen, REASON_DISTRO);

	//uint32_t len = strlen(buf);
	//len = len;

	//"<toolName> <toolVersion>:<toolCL> <platform> (Apex <apexVersion>, CL <apexCL> <apexConfiguration> <apexBranch>) (PhysX <physxVersion>) <toolBuildDate>"

	setToolString(buf);
#endif
}



void ApexAssetAuthoring::setToolString(const char* /*toolString*/)
{
	PX_ALWAYS_ASSERT();
	APEX_INVALID_OPERATION("Not Implemented.");
}

} // namespace apex
} // namespace nvidia