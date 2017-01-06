/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef APEX_RESOURCE_CALLBACK_H
#define APEX_RESOURCE_CALLBACK_H

#include "Apex.h"
#include "ModuleParticles.h"

#pragma warning(push)
#pragma warning(disable : 4350)
#include <vector>
#include <string>
#include <map>
#pragma warning(pop)

using namespace nvidia::apex;

class physx::PxFileBuf;

class ApexResourceCallback : public ResourceCallback
{
  public:
	ApexResourceCallback();
	~ApexResourceCallback();

	void setApexSDK(ApexSDK* apexSDK)
	{
		mApexSDK = apexSDK;
	}

	void setModuleParticles(ModuleParticles* moduleParticles)
	{
		mModuleParticles = moduleParticles;
	}

	// ResourceCallback API:
	virtual void* requestResource(const char* nameSpace, const char* name);
	virtual void releaseResource(const char* nameSpace, const char*, void* resource);

	// Internal samples API:
	void addSearchDir(const char* dir, bool recursive = true);
	NvParameterized::Interface* deserializeFromFile(const char* path);

	enum CustomResourceNameSpace
	{
		eSHADER_FILE_PATH,
		eTEXTURE_RESOURCE
	};

	void* requestResourceCustom(CustomResourceNameSpace customNameSpace, const char* name);

  private:
	ApexSDK* mApexSDK;
	ModuleParticles* mModuleParticles;

	bool findFile(const char* fileName, std::vector<const char*> exts, char* foundPath);
	bool findFileInDir(const char* fileNameFull, const char* path, bool recursive, char* foundPath);

	struct SearchDir
	{
		char* path;
		bool recursive;
	};

	std::vector<SearchDir> mSearchDirs;
	std::map<std::pair<CustomResourceNameSpace, std::string>, void*> mCustomResources;
};

#endif