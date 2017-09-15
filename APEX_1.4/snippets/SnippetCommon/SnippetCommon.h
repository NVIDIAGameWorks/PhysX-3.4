/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef APEX_SNIPPET_COMMON_H
#define APEX_SNIPPET_COMMON_H


#include "PxPhysicsAPI.h"
#include "Apex.h"
#include <cstring>
#include <string>



using namespace physx;
using namespace nvidia;


PxDefaultAllocator		 gAllocator;
PxDefaultErrorCallback	 gErrorCallback;

PxFoundation*			 gFoundation = NULL;
PxPhysics*				 gPhysics = NULL;
PxCooking*				 gCooking = NULL;

PxDefaultCpuDispatcher*	 gDispatcher = NULL;
PxScene*				 gPhysicsScene = NULL;

PxMaterial*				 gMaterial = NULL;

PxPvd*           gPvd = NULL;

#define PVD_TO_FILE 0

void initPhysX()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);

	PxTolerancesScale scale;

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale, true, gPvd);

	PxCookingParams cookingParams(scale);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, gPhysics->getFoundation(), cookingParams);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gPhysicsScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gPhysicsScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}


void releasePhysX()
{
	gPhysicsScene->release();
	gDispatcher->release();
	gPhysics->release();
	gPvd->release();
	gCooking->release();
	gFoundation->release();
}


class DummyRenderResourceManager : public UserRenderResourceManager
{
public:
	virtual UserRenderVertexBuffer*   createVertexBuffer(const UserRenderVertexBufferDesc&) { return NULL;  }
	virtual void                        releaseVertexBuffer(UserRenderVertexBuffer&) {}

	virtual UserRenderIndexBuffer*    createIndexBuffer(const UserRenderIndexBufferDesc&) { return NULL; }
	virtual void                        releaseIndexBuffer(UserRenderIndexBuffer&) {}

	virtual UserRenderBoneBuffer*     createBoneBuffer(const UserRenderBoneBufferDesc&) { return NULL; }
	virtual void                        releaseBoneBuffer(UserRenderBoneBuffer&) {}

	virtual UserRenderInstanceBuffer* createInstanceBuffer(const UserRenderInstanceBufferDesc&) { return NULL; }
	virtual void                        releaseInstanceBuffer(UserRenderInstanceBuffer&) {}

	virtual UserRenderSpriteBuffer*   createSpriteBuffer(const UserRenderSpriteBufferDesc&) { return NULL; }
	virtual void                        releaseSpriteBuffer(UserRenderSpriteBuffer&) {}

	virtual UserRenderSurfaceBuffer*  createSurfaceBuffer(const UserRenderSurfaceBufferDesc&) { return NULL; }
	virtual void                        releaseSurfaceBuffer(UserRenderSurfaceBuffer&) {}

	virtual UserRenderResource*       createResource(const UserRenderResourceDesc&) { return NULL; }
	virtual void                        releaseResource(UserRenderResource&) {}

	virtual uint32_t                       getMaxBonesForMaterial(void*) { return 0; }

	virtual bool getSpriteLayoutData(uint32_t, uint32_t, UserRenderSpriteBufferDesc*)  { return false; }

	virtual bool getInstanceLayoutData(uint32_t, uint32_t, UserRenderInstanceBufferDesc*) { return false; }
};

#if PX_SWITCH
#include <nn/fs.h>
#else
#include <sys/stat.h>
#endif


// Check that path exist
bool isFileExist(const char* path)
{
#if PX_SWITCH 
	nn::fs::DirectoryEntryType t;
	nn::Result r = nn::fs::GetEntryType(&t, path);
	if (r.IsFailure() || t != nn::fs::DirectoryEntryType_File)
		return false;
#else 
	struct stat tmp;
	if (0 != stat(path, &tmp))
		return false;
#endif
	return true;
}


void snippetMain(const char*);

#if PX_WINDOWS_FAMILY
#include "Shlwapi.h"

int main(int, char**)
{
	LPTSTR cmd = GetCommandLine();
	PathRemoveFileSpec(cmd);

	char buf[256];
	strcpy(buf, cmd + 1);
	strcat(buf, "/../../");
	snippetMain(buf);

	return 0;
}

#elif PX_SWITCH

#include <nn/os.h>
#include <nn/fs.h>

static void* alloc(size_t size)			{ return malloc(size); }
static void dealloc(void* ptr, size_t)	{ free(ptr); }

extern "C" void nnMain()
{
	nn::fs::SetAllocator(alloc, dealloc);
	nn::Result res = nn::fs::MountHostRoot();
	if (res.IsSuccess())
	{
		int argc = nn::os::GetHostArgc();
		char** argv = nn::os::GetHostArgv();

		std::string path(argv[0]);
		path = path.substr(0, path.find_last_of("/\\"));
		path.append(("/.."));
		path.append(("/../"));
		std::replace(path.begin(), path.end(), '\\', '/');

		res = nn::fs::MountHost("host", path.c_str());
		if (res.IsSuccess())
		{
			snippetMain("host:/");
			nn::fs::Unmount("host");
		}

		nn::fs::UnmountHostRoot();
	}
}

#endif

#endif