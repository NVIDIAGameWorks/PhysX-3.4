/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "../SnippetCommon/SnippetCommon.h"

#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParamUtils.h"

#include "particles/ModuleParticles.h"
#include "emitter/EmitterAsset.h"
#include "iofx/IofxAsset.h"
#include "basicios/BasicIosAsset.h"
#include "RenderMeshAsset.h"

#include <legacy/ModuleLegacy.h>

#include "PsString.h"

#include <sys/stat.h>
#include "Shlwapi.h"

using namespace nvidia::apex;

class MyResourceCallback;

ApexSDK*					gApexSDK = NULL;
DummyRenderResourceManager* gDummyRenderResourceManager = NULL;
MyResourceCallback*			gMyResourceCallback = NULL;

LPTSTR gMediaPath;

class MyMaterial
{
public:
	MyMaterial(const char*)
	{
	}
};

class MyResourceCallback : public ResourceCallback
{
public:
	virtual void* requestResource(const char* nameSpace, const char* name)
	{
		void* resource = NULL;

		if (
			!physx::shdfnd::strcmp(nameSpace, EMITTER_AUTHORING_TYPE_NAME)
			|| !physx::shdfnd::strcmp(nameSpace, IOFX_AUTHORING_TYPE_NAME)
			|| !physx::shdfnd::strcmp(nameSpace, BASIC_IOS_AUTHORING_TYPE_NAME)
			|| !physx::shdfnd::strcmp(nameSpace, RENDER_MESH_AUTHORING_TYPE_NAME)
			)
		{
			Asset* asset = 0;

			const char* path = name;

			// does file exists?
			if (isFileExist(path))
			{
				PxFileBuf* stream = gApexSDK->createStream(path, PxFileBuf::OPEN_READ_ONLY);

				if (stream)
				{
					NvParameterized::Serializer::SerializeType serType = gApexSDK->getSerializeType(*stream);
					NvParameterized::Serializer::ErrorType serError;
					NvParameterized::Serializer*  ser = gApexSDK->createSerializer(serType);
					PX_ASSERT(ser);

					NvParameterized::Serializer::DeserializedData data;
					serError = ser->deserialize(*stream, data);

					if (serError == NvParameterized::Serializer::ERROR_NONE && data.size() == 1)
					{
						NvParameterized::Interface* params = data[0];
						asset = gApexSDK->createAsset(params, name);
						PX_ASSERT(asset && "ERROR Creating NvParameterized Asset");
					}
					else
					{
						PX_ASSERT(0 && "ERROR Deserializing NvParameterized Asset");
					}

					stream->release();
					ser->release();
				}
			}
			else
			{
				shdfnd::printFormatted("Can't find file: %s", path);
				return NULL;
			}

			resource = asset;
		}
		else if (!shdfnd::strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
		{
			MyMaterial* material = new MyMaterial(name);
			resource = material;
		}
		else
		{
			PX_ASSERT(0 && "Namespace not implemented.");
		}

		PX_ASSERT(resource);
		return resource;
	}

	virtual void  releaseResource(const char* nameSpace, const char*, void* resource)
	{
		if (!shdfnd::strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
		{
			MyMaterial* material = (MyMaterial*)resource;
			delete material;
		}
		else
		{
			Asset* asset = (Asset*)resource;
			gApexSDK->releaseAsset(*asset);
		}
	}
};


void initApex()
{
	// Fill out the Apex SDKriptor
	ApexSDKDesc apexDesc;

	// Apex needs an allocator and error stream.  By default it uses those of the PhysX SDK.

	// Let Apex know about our PhysX SDK and cooking library
	apexDesc.physXSDK = gPhysics;
	apexDesc.cooking = gCooking;
	apexDesc.pvd = gPvd;

	// Our custom dummy render resource manager
	gDummyRenderResourceManager = new DummyRenderResourceManager();
	apexDesc.renderResourceManager = gDummyRenderResourceManager;

	// Our custom named resource handler
	gMyResourceCallback = new MyResourceCallback();
	apexDesc.resourceCallback = gMyResourceCallback;
	apexDesc.foundation = gFoundation;

	// Finally, create the Apex SDK
	ApexCreateError error;
	gApexSDK = CreateApexSDK(apexDesc, &error);
	PX_ASSERT(gApexSDK);

#if APEX_MODULES_STATIC_LINK
	nvidia::apex::instantiateModuleParticles();
	nvidia::apex::instantiateModuleLegacy();
#endif

	// Initialize particles module
	ModuleParticles* apexParticlesModule = static_cast<ModuleParticles*>(gApexSDK->createModule("Particles"));
	PX_ASSERT(apexParticlesModule);
	apexParticlesModule->init(*apexParticlesModule->getDefaultModuleDesc());
}

Asset* loadApexAsset(const char* nameSpace, const char* path)
{
	Asset* asset = static_cast<Asset*>(gApexSDK->getNamedResourceProvider()->getResource(nameSpace, path));
	return asset;
}

void releaseAPEX()
{
	gApexSDK->release();
	delete gDummyRenderResourceManager;
	delete gMyResourceCallback;
}

void snippetMain(const char* rootPath)
{
	initPhysX();
	initApex();

	Asset* asset;

	std::string path;
	path.append(rootPath);
	path.append("/snippets/SnippetCommon/");

	std::string file0 = path + "testMeshEmitter4BasicIos6.apx";
	asset = loadApexAsset(EMITTER_AUTHORING_TYPE_NAME, file0.c_str());
	asset->forceLoadAssets();
	gApexSDK->forceLoadAssets();
	asset->release();

	std::string file1 = path + "testMeshEmitter4BasicIos6.apx";
	asset = loadApexAsset(EMITTER_AUTHORING_TYPE_NAME, file1.c_str());
	asset->forceLoadAssets();
	gApexSDK->forceLoadAssets();
	asset->release();

	releaseAPEX();
	releasePhysX();
}
