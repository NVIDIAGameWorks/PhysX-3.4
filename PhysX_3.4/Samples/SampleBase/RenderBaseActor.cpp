// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "SamplePreprocessor.h"
#include "RenderBaseActor.h"
#include "RenderMaterial.h"
#include "RendererShape.h"
#include "Renderer.h"
#include "RendererMemoryMacros.h"
#include "extensions/PxShapeExt.h"
#include "RaycastCCD.h"
#include "RenderPhysX3Debug.h"
#include "PxRigidDynamic.h"
#include "PxArticulationLink.h"

#include "geometry/PxSphereGeometry.h"
#include "geometry/PxBoxGeometry.h"
#include "RenderSphereActor.h"
#include "RenderCapsuleActor.h"

using namespace physx;
using namespace SampleRenderer;

static const bool gRaycastCCD = true;

RenderBaseActor::RenderBaseActor() :
	mRendererShape		(NULL),
	mPhysicsShape		(NULL),
	mDynamicActor		(NULL),
	mArticulationLink	(NULL),
	mMaterial			(NULL),
	mEnableCCD			(false),
	mEnableRender		(true),
	mEnableDebugRender	(true),
	mEnableCameraCull	(false)
{
	mTransform			= PxTransform(PxIdentity);
	mWorldBounds		= PxBounds3(PxVec3(0),PxVec3(0));

	mCCDWitness			= PxVec3(0);
	mCCDWitnessOffset	= PxVec3(0);

	mPhysicsToGraphicsRot = PxQuat(PxIdentity);

	updateScale();
}

RenderBaseActor::RenderBaseActor(const RenderBaseActor& src) : RenderBaseObject(src)
{
	mEnableCCD			= src.mEnableCCD;
	mEnableRender		= src.mEnableRender;
	mEnableDebugRender	= src.mEnableDebugRender;
	mEnableCameraCull	= src.mEnableCameraCull;

	mTransform			= src.getTransform();
	mWorldBounds		= PxBounds3(PxVec3(0.0f), PxVec3(0.0f));
	mPhysicsShape		= NULL;	// PT: the physics shape is not cloned for now
	mDynamicActor		= NULL;	// PT: the physics actor is not cloned for now
	mArticulationLink	= NULL;	// PT: the articulation link is not cloned for now
	mMaterial			= src.getRenderMaterial();
	mRendererShape		= NULL;
	setRenderShape(src.getRenderShape());

	mPhysicsToGraphicsRot	= src.mPhysicsToGraphicsRot;
	mCCDWitness				= src.mCCDWitness;
	mCCDWitnessOffset		= src.mCCDWitnessOffset;

	mScaling				= src.mScaling;

	updateScale();
}

RenderBaseActor::~RenderBaseActor()
{
	deleteRenderShape();
	mMaterial		= NULL;	// PT: we don't own this
	mPhysicsShape	= NULL;	// PT: we don't own this
	mDynamicActor	= NULL;	// PT: we don't own this
}

void RenderBaseActor::update(float deltaTime)
{
	// Setup render transform from physics transform, if physics object exists
	if(mPhysicsShape)
	{
		if(!mArticulationLink && ( !mDynamicActor || mDynamicActor->isSleeping()))
			return;

		PxTransform newPose = PxShapeExt::getGlobalPose(*mPhysicsShape, *mPhysicsActor);
		PxVec3 newShapeCenter = getShapeCenter(mPhysicsActor, mPhysicsShape, mCCDWitnessOffset);

		bool updateCCDWitness = true;
		if(gRaycastCCD && mEnableCCD)
			updateCCDWitness = doRaycastCCD(mPhysicsActor, mPhysicsShape, newPose, newShapeCenter, mCCDWitness, mCCDWitnessOffset);

		// Copy physics pose to graphics pose
		setTransform(PxTransform(newPose.p, newPose.q * mPhysicsToGraphicsRot));

		if(updateCCDWitness)
			mCCDWitness = newShapeCenter;
		
		setWorldBounds(PxShapeExt::getWorldBounds(*mPhysicsShape, *mPhysicsActor));
	}
}

void RenderBaseActor::render(Renderer& renderer, RenderMaterial* material, bool wireFrame)
{
	if(!mEnableRender)
		return;

	RenderMaterial* m = mMaterial ? mMaterial : material;
	PX_ASSERT(m);

	mRendererMeshContext.cullMode			= m->mDoubleSided ? RendererMeshContext::NONE : RendererMeshContext::CLOCKWISE;
	mRendererMeshContext.fillMode			= wireFrame ? RendererMeshContext::LINE : RendererMeshContext::SOLID;
	mRendererMeshContext.material			= m->mRenderMaterial;
	mRendererMeshContext.materialInstance	= m->mRenderMaterialInstance;
	mRendererMeshContext.mesh				= mRendererShape->getMesh();
	mRendererMeshContext.transform			= &mScaledTransform;

	renderer.queueMeshForRender(mRendererMeshContext);
}

PxBounds3 RenderBaseActor::getWorldBounds() const
{
	return mWorldBounds;
}

void RenderBaseActor::setWorldBounds(const PxBounds3& bounds)
{
	mWorldBounds = bounds;
}

void RenderBaseActor::updateScale()
{
	if (!mScaling.isIdentity())
	{
		PxMat33 q = PxMat33(mTransform.q) * mScaling.toMat33();
		mScaledTransform = PxMat44(q, mTransform.p);
		mRendererMeshContext.negativeScale = mScaling.hasNegativeDeterminant();
	}
	else
	{
		mScaledTransform = PxMat44(mTransform);
	}
}

void RenderBaseActor::setPhysicsShape(PxShape* shape, PxRigidActor* actor)
{
	mPhysicsShape = shape;
	mPhysicsActor = actor;

	if(shape)
	{
		mCCDWitness = getShapeCenter(actor, shape, mCCDWitnessOffset);

		const PxTransform newPose = PxShapeExt::getGlobalPose(*shape, *actor);
		setTransform(PxTransform(newPose.p, newPose.q * mPhysicsToGraphicsRot));

		PxRigidActor& ra = *actor;
		mDynamicActor = ra.is<PxRigidDynamic>();
		mArticulationLink = ra.is<PxArticulationLink>();
		mWorldBounds = PxShapeExt::getWorldBounds(*mPhysicsShape, *mPhysicsActor);
	}
	else
	{
		mDynamicActor = NULL;
	}
}

void RenderBaseActor::setRenderMaterial(RenderMaterial* material)
{
	mMaterial = material;
}

void RenderBaseActor::setRenderShape(RendererShape* shape)
{
	PX_ASSERT(!mRendererShape);
	mRendererShape = shape;

	if(shape)
	{
		// PT: we use the user-data as a ref-counter
		size_t refCount = size_t(mRendererShape->m_userData);
		refCount++;
		mRendererShape->m_userData = (void*)refCount;
	}
}

void RenderBaseActor::deleteRenderShape()
{
	if(mRendererShape)
	{
		// PT: we use the user-data as a ref-counter
		size_t refCount = size_t(mRendererShape->m_userData);
		refCount--;
		if(!refCount)
		{
			DELETESINGLE(mRendererShape);
		}
		else
		{
			mRendererShape->m_userData = (void*)refCount;
			mRendererShape = NULL;
		}
	}
}

void RenderBaseActor::drawDebug(RenderPhysX3Debug* debug)
{
//	return;

	if(!mPhysicsShape)
		return;

	if(0 && mEnableCCD)
	{
		const PxBounds3 bounds = PxShapeExt::getWorldBounds(*mPhysicsShape, *mPhysicsActor);
		const PxReal scale = (bounds.maximum - bounds.minimum).magnitude();

		const PxTransform pose = PxShapeExt::getGlobalPose(*mPhysicsShape, *mPhysicsActor);
		debug->addLine(pose.p, pose.p+PxVec3(scale, 0.0f, 0.0f), RendererColor(0, 255, 0));
		debug->addLine(pose.p, pose.p+PxVec3(0.0f, scale, 0.0f), RendererColor(0, 255, 0));
		debug->addLine(pose.p, pose.p+PxVec3(0.0f, 0.0f, scale), RendererColor(0, 255, 0));

		const PxVec3 shapeCenter = getShapeCenter(mPhysicsActor, mPhysicsShape, mCCDWitnessOffset);
//shdfnd::printFormatted("Render: %f | %f | %f\n", shapeCenter.x, shapeCenter.y, shapeCenter.z);

		debug->addLine(shapeCenter, shapeCenter+PxVec3(scale, 0.0f, 0.0f), RendererColor(255, 0, 0));
		debug->addLine(shapeCenter, shapeCenter+PxVec3(0.0f, scale, 0.0f), RendererColor(255, 0, 0));
		debug->addLine(shapeCenter, shapeCenter+PxVec3(0.0f, 0.0f, scale), RendererColor(255, 0, 0));
		return;
	}

/*
	BasicRandom rnd(42);

	const PxTransform globalShapePose = PxShapeExt::getGlobalPose(*mPhysicsShape);

	const RendererColor colorPurple(255, 0, 255);
	for(PxU32 i=0;i<50;i++)
	{
		PxVec3 dir;
		rnd.unitRandomPt(dir);

		PxVec3 localCenter;
		const PxReal internalRadius = computeInternalRadius(mPhysicsShape, dir, localCenter, mCCDWitnessOffset);

		const PxVec3 center = globalShapePose.transform(localCenter);

		debug->addLine(center, center+dir*internalRadius, colorPurple);
	}*/
}
