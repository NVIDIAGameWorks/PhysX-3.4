/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SAMPLE_APEX_RESOURCE_CALLBACK_H
#define SAMPLE_APEX_RESOURCE_CALLBACK_H

#include <ApexDefs.h>
#include <ApexSDK.h>
#include <PxFiltering.h>
#include <ResourceCallback.h>
#include <PxFileBuf.h>
#include "Find.h"
#include <SampleAsset.h>
#include <vector>

#pragma warning(push)
#pragma warning(disable:4512)

class FilterBits; // forward reference the filter bits class

namespace nvidia
{
namespace apex
{
class ApexSDK;
#if APEX_USE_PARTICLES
class ModuleParticles;
#endif
}
}

namespace SampleRenderer
{
class Renderer;
}

namespace SampleFramework
{
class SampleAssetManager;
}

// TODO: DISABLE ME!!!
#define WORK_AROUND_BROKEN_ASSET_PATHS 1

enum SampleAssetFileType
{
	XML_ASSET,
	BIN_ASSET,
	ANY_ASSET,
};

class SampleApexResourceCallback : public nvidia::apex::ResourceCallback
{
public:
	SampleApexResourceCallback(SampleRenderer::Renderer& renderer, SampleFramework::SampleAssetManager& assetManager);
	virtual				   ~SampleApexResourceCallback(void);

	void					addResourceSearchPath(const char* path);
	void					removeResourceSearchPath(const char* path);
	void					clearResourceSearchPaths();

	void					registerSimulationFilterData(const char* name, const physx::PxFilterData& simulationFilterData);
	void					registerPhysicalMaterial(const char* name, physx::PxMaterialTableIndex physicalMaterial);

	void					registerGroupsMask64(const char* name, nvidia::apex::GroupsMask64& groupsMask);

	void					setApexSupport(nvidia::apex::ApexSDK& apexSDK);

	physx::PxFileBuf*   	findApexAsset(const char* assetName);
	void					findFiles(const char* dir, nvidia::apex::FileHandler& handler);

	void					setAssetPreference(SampleAssetFileType pref)
	{
		m_assetPreference = pref;
	}

	static bool				xmlFileExtension(const char* assetName);
	static const char*		getFileExtension(const char* assetName);

private:
	SampleFramework::SampleAsset*	findSampleAsset(const char* assetName, SampleFramework::SampleAsset::Type type);

#if WORK_AROUND_BROKEN_ASSET_PATHS
	const char*				mapHackyPath(const char* path);
#endif

public:
	virtual void*			requestResource(const char* nameSpace, const char* name);
	virtual void			releaseResource(const char* nameSpace, const char* name, void* resource);

	bool					doesFileExist(const char* filename, const char* ext);
	bool					doesFileExist(const char* filename);
	bool					isFileReadable(const char* fullPath);

protected:
	SampleRenderer::Renderer&				m_renderer;
	SampleFramework::SampleAssetManager&	m_assetManager;
	std::vector<char*>						m_searchPaths;
	std::vector<physx::PxFilterData>		m_FilterDatas;
	FilterBits								*m_FilterBits;

	std::vector<nvidia::apex::GroupsMask64>	m_nxGroupsMask64s;
#if APEX_USE_PARTICLES
	nvidia::apex::ModuleParticles*			mModuleParticles;
#endif
	nvidia::apex::ApexSDK*					m_apexSDK;
	uint32_t							m_numGets;
	SampleAssetFileType						m_assetPreference;
};

#pragma warning(pop)

#endif // SAMPLE_APEX_RESOURCE_CALLBACK_H
