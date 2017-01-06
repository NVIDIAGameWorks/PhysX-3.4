/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_SHAPE_ACTOR_H__
#define __SAMPLE_SHAPE_ACTOR_H__

#include "SampleActor.h"
#include "ApexDefs.h"
#include "PxActor.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"

#if PX_PHYSICS_VERSION_MAJOR == 3
#include "ScopedPhysXLock.h"
#endif
#include "RenderDebugInterface.h"
#include <Renderer.h>
#include <RendererMeshContext.h>


#if PX_PHYSICS_VERSION_MAJOR == 0
namespace physx
{
namespace apex
{
/**
\brief A scoped object for acquiring/releasing read/write lock of a PhysX scene
*/
class ScopedPhysXLockRead
{
public:
	/**
	\brief Ctor
	*/
	ScopedPhysXLockRead(PxScene *scene,const char *fileName,int lineno) : mScene(scene)
	{
		if ( mScene )
		{
			mScene->lockRead(fileName, (physx::PxU32)lineno);
		}
	}
	~ScopedPhysXLockRead()
	{
		if ( mScene )
		{
			mScene->unlockRead();
		}
	}
private:
	PxScene* mScene;
};

/**
\brief A scoped object for acquiring/releasing read/write lock of a PhysX scene
*/
class ScopedPhysXLockWrite
{
public:
	/**
	\brief Ctor
	*/
	ScopedPhysXLockWrite(PxScene *scene,const char *fileName,int lineno) : mScene(scene)
	{
		if ( mScene ) 
		{
			mScene->lockWrite(fileName, (physx::PxU32)lineno);
		}
	}
	~ScopedPhysXLockWrite()
	{
		if ( mScene )
		{
			mScene->unlockWrite();
		}
	}
private:
	PxScene* mScene;
};

}; // end apx namespace
}; // end physx namespace


#if defined(_DEBUG) || defined(PX_CHECKED)
#define SCOPED_PHYSX_LOCK_WRITE(x) physx::apex::ScopedPhysXLockWrite _wlock(x,__FILE__,__LINE__);
#else
#define SCOPED_PHYSX_LOCK_WRITE(x) physx::apex::ScopedPhysXLockWrite _wlock(x,"",__LINE__);
#endif

#if defined(_DEBUG) || defined(PX_CHECKED)
#define SCOPED_PHYSX_LOCK_READ(x) physx::apex::ScopedPhysXLockRead _rlock(x,__FILE__,__LINE__);
#else
#define SCOPED_PHYSX_LOCK_READ(x) physx::apex::ScopedPhysXLockRead _rlock(x,"",__LINE__);
#endif

#endif // PX_PHYSICS_VERSION_MAJOR



class SampleShapeActor : public SampleFramework::SampleActor
{
public:
	SampleShapeActor(nvidia::apex::RenderDebugInterface* rdebug)
		: mBlockId(-1)
		, mApexRenderDebug(rdebug)
		, mRenderer(NULL)
		, mPhysxActor(NULL)
	{
	}

	virtual ~SampleShapeActor(void)
	{
		if (mApexRenderDebug != NULL)
		{
			RENDER_DEBUG_IFACE(mApexRenderDebug)->reset(mBlockId);
		}

		if (mPhysxActor)
		{
			SCOPED_PHYSX_LOCK_WRITE(mPhysxActor->getScene());
			mPhysxActor->release();
		}
	}

	physx::PxTransform getPose() const
	{
		return physx::PxTransform(mTransform);
	}

	void setPose(const physx::PxTransform& pose)
	{
		mTransform = physx::PxMat44(pose);
		if (mPhysxActor)
		{
			SCOPED_PHYSX_LOCK_WRITE(mPhysxActor->getScene());
			if (physx::PxRigidDynamic* rd = mPhysxActor->is<physx::PxRigidDynamic>())
			{
				rd->setGlobalPose(this->convertToPhysicalCoordinates(mTransform));
			}
		}
		if (mApexRenderDebug != NULL)
		{
			RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mBlockId, mTransform);
		}
	}

	virtual void tick(float dtime, bool rewriteBuffers = false)
	{
		if (mPhysxActor)
		{
			physx::PxRigidDynamic* rd = mPhysxActor->is<physx::PxRigidDynamic>();
			SCOPED_PHYSX_LOCK_READ(mPhysxActor->getScene());
			if (rd && !rd->isSleeping())
			{
				mTransform = this->convertToGraphicalCoordinates(rd->getGlobalPose());
				if (mApexRenderDebug != NULL)
				{
					RENDER_DEBUG_IFACE(mApexRenderDebug)->setDrawGroupPose(mBlockId, mTransform);
				}
			}
		}
	}

	physx::PxActor* getPhysXActor()
	{
		return mPhysxActor;
	}

	virtual void render(bool /*rewriteBuffers*/ = false)
	{
		if (mRenderer)
		{
			mRenderer->queueMeshForRender(mRendererMeshContext);
		}
	}

protected:
	int32_t								mBlockId;
	nvidia::apex::RenderDebugInterface*			mApexRenderDebug;
	SampleRenderer::Renderer*			mRenderer;
	SampleRenderer::RendererMeshContext	mRendererMeshContext;
	physx::PxMat44						mTransform; 
	physx::PxActor*						mPhysxActor;

private:
	virtual physx::PxMat44		convertToGraphicalCoordinates(const physx::PxTransform & physicsPose) const
	{
		return physx::PxMat44(physicsPose);
	}

	virtual physx::PxTransform	convertToPhysicalCoordinates(const physx::PxMat44 & graphicsPose) const
	{
		return physx::PxTransform(graphicsPose);
	}
};

#endif
