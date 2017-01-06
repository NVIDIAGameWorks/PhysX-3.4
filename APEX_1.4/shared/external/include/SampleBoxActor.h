/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_BOX_ACTOR_H__
#define __SAMPLE_BOX_ACTOR_H__

#include "SampleShapeActor.h"
#include "RendererBoxShape.h"

#include "PxPhysics.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "geometry/PxBoxGeometry.h"
#include "extensions/PxExtensionsAPI.h"
namespace physx
{
class PxMaterial;
}

#include "RenderDebugInterface.h"
#include <Renderer.h>
#include <RendererMeshContext.h>

class SampleBoxActor : public SampleShapeActor
{
public:
	SampleBoxActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               physx::PxScene& physxScene,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               const physx::PxVec3& extents,
	               float density,
	               physx::PxMaterial* PxMaterial,
	               bool useGroupsMask,
	               nvidia::apex::RenderDebugInterface* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererBoxShape(NULL)
	{
		mRenderer = renderer;
		if (!PxMaterial)
			physxScene.getPhysics().getMaterials(&PxMaterial, 1);
		createActor(physxScene, pos, vel, extents, density, PxMaterial, useGroupsMask);

		mRendererBoxShape = new SampleRenderer::RendererBoxShape(*mRenderer, extents);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererBoxShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = RENDER_DEBUG_IFACE(rdebug)->beginDrawGroup(mTransform);
			RENDER_DEBUG_IFACE(rdebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::SolidShaded);
			static uint32_t bcount /* = 0 */;
			RENDER_DEBUG_IFACE(rdebug)->setCurrentColor(0xFFFFFF);
			RENDER_DEBUG_IFACE(rdebug)->setCurrentTextScale(0.5f);
			RENDER_DEBUG_IFACE(rdebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::CenterText);
			RENDER_DEBUG_IFACE(rdebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::CameraFacing);
			RENDER_DEBUG_IFACE(rdebug)->debugText(physx::PxVec3(0, extents.y + 0.01f, 0), "Sample Box:%d", bcount++);
			RENDER_DEBUG_IFACE(rdebug)->endDrawGroup();
		}
	}

	virtual ~SampleBoxActor()
	{
		if (mRendererBoxShape)
		{
			delete mRendererBoxShape;
			mRendererBoxShape = NULL;
		}
	}

private:
	void createActor(physx::PxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& vel,
	                 const physx::PxVec3& extents,
	                 float density,
	                 physx::PxMaterial* PxMaterial,
	                 bool useGroupsMask)
	{
		mTransform = physx::PxMat44(physx::PxIdentity);
		mTransform.setPosition(pos);

		physx::PxRigidActor* actor = NULL;
		if (density > 0)
		{
			actor = physxScene.getPhysics().createRigidDynamic(physx::PxTransform(mTransform));
			((physx::PxRigidDynamic*)actor)->setAngularDamping(0.5f);
			((physx::PxRigidDynamic*)actor)->setLinearVelocity(vel);
		}
		else
		{
			actor = physxScene.getPhysics().createRigidStatic(physx::PxTransform(mTransform));
		}
		
		PX_ASSERT(actor);

		physx::PxBoxGeometry boxGeom(extents);
		physx::PxShape* shape = actor->createShape(boxGeom, *PxMaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1u, 0u, ~0u, 0u));
			shape->setQueryFilterData(physx::PxFilterData(1u, 0u, ~0u, 0u));
		}

		if (density > 0)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*((physx::PxRigidDynamic*)actor), density);
		}
		SCOPED_PHYSX_LOCK_WRITE(&physxScene);
		physxScene.addActor(*actor);
		mPhysxActor = actor;
	}

private:
	SampleRenderer::RendererBoxShape* mRendererBoxShape;
};

#endif
