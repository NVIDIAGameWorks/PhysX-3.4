/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_CAPSULE_ACTOR_H__
#define __SAMPLE_CAPSULE_ACTOR_H__

#include "SampleShapeActor.h"

#include "PxRigidDynamic.h"
#include "geometry/PxCapsuleGeometry.h"
#include "extensions/PxExtensionsAPI.h"
namespace physx
{
class PxMaterial;
}

#include <Renderer.h>
#include <RendererMeshContext.h>

class SampleCapsuleActor : public SampleShapeActor
{
public:
	SampleCapsuleActor(SampleRenderer::Renderer* renderer,
	                   SampleFramework::SampleMaterialAsset& material,
	                   physx::PxScene& physxScene,
	                   const physx::PxVec3& pos,
	                   const physx::PxVec3& vel,
	                   float height,
					   float radius,
	                   float density,
	                   physx::PxMaterial* PxMaterial,
	                   bool useGroupsMask,
	                   nvidia::apex::RenderDebugInterface* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mRendererCapsuleShape(NULL)
		, mRadius(radius)
		, mHeight(height)
	{
		mRenderer = renderer;
		if (!PxMaterial)
			physxScene.getPhysics().getMaterials(&PxMaterial, 1);
		createActor(physxScene, pos, vel, radius, height, density, PxMaterial, useGroupsMask);

		mRendererCapsuleShape = new SampleRenderer::RendererCapsuleShape(*mRenderer, mHeight / 2, mRadius);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererCapsuleShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = RENDER_DEBUG_IFACE(rdebug)->beginDrawGroup(mTransform);
			RENDER_DEBUG_IFACE(rdebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::SolidShaded);
			static uint32_t ccount /* = 0 */;
			RENDER_DEBUG_IFACE(rdebug)->setCurrentColor(0xFFFFFF);
			RENDER_DEBUG_IFACE(rdebug)->setCurrentTextScale(0.5f);
			RENDER_DEBUG_IFACE(rdebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::CenterText);
			RENDER_DEBUG_IFACE(rdebug)->addToCurrentState(RENDER_DEBUG::DebugRenderState::CameraFacing);
			RENDER_DEBUG_IFACE(rdebug)->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Sphere:%d", ccount++);
			RENDER_DEBUG_IFACE(rdebug)->endDrawGroup();
		}
	}

	virtual ~SampleCapsuleActor()
	{
		if (mRendererCapsuleShape)
		{
			delete mRendererCapsuleShape;
			mRendererCapsuleShape = NULL;
		}
	}

private:
	void createActor(physx::PxScene& physxScene,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& vel,
	                 float radius,
	                 float height,
	                 float density,
	                 physx::PxMaterial* PxMaterial,
	                 bool useGroupsMask)
	{
		mTransform = physx::PxMat44(physx::PxIdentity);
		mTransform.setPosition(pos);

		physx::PxRigidDynamic* actor = physxScene.getPhysics().createRigidDynamic(physx::PxTransform(mTransform));
		PX_ASSERT(actor);
		actor->setAngularDamping(0.5f);
		actor->setLinearVelocity(vel);

		physx::PxCapsuleGeometry capsuleGeom(radius, height * 0.5f);
		physx::PxShape* shape = actor->createShape(capsuleGeom, *PxMaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1, 0, ~0u, 0));
			shape->setQueryFilterData(physx::PxFilterData(1, 0, ~0u, 0));
		}

		if (density > 0)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*actor, density);
		}
		else
		{
			actor->setMass(1.0f);
		}

		SCOPED_PHYSX_LOCK_WRITE(&physxScene);
		physxScene.addActor(*actor);
		mPhysxActor = actor;
	}

	/*virtual*/ physx::PxMat44		convertToGraphicalCoordinates(const physx::PxTransform & physicsPose) const
	{
		static const physx::PxTransform rotCcwRhZ = physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));
		return physx::PxMat44(physicsPose * rotCcwRhZ);
	}
	
	/*virtual*/ physx::PxTransform	convertToPhysicalCoordinates(const physx::PxMat44 & graphicsPose) const
	{
		static const physx::PxTransform rotCwRhZ = physx::PxTransform(physx::PxQuat(-1.0f * physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));
		return physx::PxTransform(graphicsPose * rotCwRhZ);
	}

private:
	SampleRenderer::RendererCapsuleShape*	mRendererCapsuleShape;
	float							 mRadius, mHeight;
};

#endif
