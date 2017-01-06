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

#include "SampleCCTActor.h"
#include "characterkinematic/PxBoxController.h"
#include "characterkinematic/PxCapsuleController.h"
#include "characterkinematic/PxControllerManager.h"
#include "RenderBoxActor.h"
#include "RenderCapsuleActor.h"
#include "PhysXSample.h"

using namespace physx;
using namespace SampleRenderer;

///////////////////////////////////////////////////////////////////////////////

ControlledActorDesc::ControlledActorDesc() :
	mType				(PxControllerShapeType::eFORCE_DWORD),
	mPosition			(PxExtendedVec3(0,0,0)),
	mSlopeLimit			(0.0f),
	mContactOffset		(0.0f),
	mStepOffset			(0.0f),
	mInvisibleWallHeight(0.0f),
	mMaxJumpHeight		(0.0f),
	mRadius				(0.0f),
	mHeight				(0.0f),
	mCrouchHeight		(0.0f),
	mProxyDensity		(10.0f),
//	mProxyScale			(0.8f)
	mProxyScale			(0.9f),
	mVolumeGrowth		(1.5f),
	mReportCallback		(NULL),
	mBehaviorCallback	(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////

ControlledActor::ControlledActor(PhysXSample& owner) :
	mOwner					(owner),
	mType					(PxControllerShapeType::eFORCE_DWORD),
	mController				(NULL),
	mRenderActorStanding	(NULL),
	mRenderActorCrouching	(NULL),
	mStandingSize			(0.0f),
	mCrouchingSize			(0.0f),
	mControllerRadius		(0.0f),
	mDoStandup				(false),
	mIsCrouching			(false)
{
	mInitialPosition = PxExtendedVec3(0,0,0);
	mDelta = PxVec3(0);
	mTransferMomentum = false;
}

ControlledActor::~ControlledActor()
{
}

void ControlledActor::reset()
{
	PxSceneWriteLock scopedLock(mOwner.getActiveScene());
	mController->setPosition(mInitialPosition);
}

void ControlledActor::teleport(const PxVec3& pos)
{
	PxSceneWriteLock scopedLock(mOwner.getActiveScene());
	mController->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
	mTransferMomentum = false;
	mDelta = PxVec3(0);
}

PxExtendedVec3 ControlledActor::getFootPosition() const
{
	return mController->getFootPosition();
}

void ControlledActor::sync()
{
	if(mDoStandup)
		tryStandup();

	if(mRenderActorStanding)
		mRenderActorStanding->setRendering(!mIsCrouching);
	if(mRenderActorCrouching)
		mRenderActorCrouching->setRendering(mIsCrouching);

	const PxExtendedVec3& newPos = mController->getPosition();

	const PxTransform tr(toVec3(newPos));

//	shdfnd::printFormatted("%f %f %f\n", tr.p.x, tr.p.y, tr.p.z);

	if(mRenderActorStanding)
		mRenderActorStanding->setTransform(tr);
	if(mRenderActorCrouching)
		mRenderActorCrouching->setTransform(tr);
}

PxController* ControlledActor::init(const ControlledActorDesc& desc, PxControllerManager* manager)
{
	const float radius	= desc.mRadius;
	float height		= desc.mHeight;
	float crouchHeight	= desc.mCrouchHeight;

	PxControllerDesc* cDesc;
	PxBoxControllerDesc boxDesc;
	PxCapsuleControllerDesc capsuleDesc;

	if(desc.mType==PxControllerShapeType::eBOX)
	{
		height *= 0.5f;
		height += radius;
		crouchHeight *= 0.5f;
		crouchHeight += radius;
		boxDesc.halfHeight			= height;
		boxDesc.halfSideExtent		= radius;
		boxDesc.halfForwardExtent	= radius;
		cDesc = &boxDesc;
	}
	else 
	{
		PX_ASSERT(desc.mType==PxControllerShapeType::eCAPSULE);
		capsuleDesc.height = height;
		capsuleDesc.radius = radius;
		capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		cDesc = &capsuleDesc;
	}

	cDesc->density				= desc.mProxyDensity;
	cDesc->scaleCoeff			= desc.mProxyScale;
	cDesc->material				= &mOwner.getDefaultMaterial();
	cDesc->position				= desc.mPosition;
	cDesc->slopeLimit			= desc.mSlopeLimit;
	cDesc->contactOffset		= desc.mContactOffset;
	cDesc->stepOffset			= desc.mStepOffset;
	cDesc->invisibleWallHeight	= desc.mInvisibleWallHeight;
	cDesc->maxJumpHeight		= desc.mMaxJumpHeight;
//	cDesc->nonWalkableMode		= PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	cDesc->reportCallback		= desc.mReportCallback;
	cDesc->behaviorCallback		= desc.mBehaviorCallback;
	cDesc->volumeGrowth			= desc.mVolumeGrowth;

	mType						= desc.mType;
	mInitialPosition			= desc.mPosition;
	mStandingSize				= height;
	mCrouchingSize				= crouchHeight;
	mControllerRadius			= radius;

	PxController* ctrl = static_cast<PxBoxController*>(manager->createController(*cDesc));
	PX_ASSERT(ctrl);

	// remove controller shape from scene query for standup overlap test
	PxRigidDynamic* actor = ctrl->getActor();
	if(actor)
	{
		if(actor->getNbShapes())
		{
			PxShape* ctrlShape;
			actor->getShapes(&ctrlShape,1);
			ctrlShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);

			Renderer* renderer = mOwner.getRenderer();

			if(desc.mType==PxControllerShapeType::eBOX)
			{
				const PxVec3 standingExtents(radius, height, radius);
				const PxVec3 crouchingExtents(radius, crouchHeight, radius);

				mRenderActorStanding = SAMPLE_NEW(RenderBoxActor)(*renderer, standingExtents);
				mRenderActorCrouching = SAMPLE_NEW(RenderBoxActor)(*renderer, crouchingExtents);
			}
			else if(desc.mType==PxControllerShapeType::eCAPSULE)
			{
				mRenderActorStanding = SAMPLE_NEW(RenderCapsuleActor)(*renderer, radius, height*0.5f);
				mRenderActorCrouching = SAMPLE_NEW(RenderCapsuleActor)(*renderer, radius, crouchHeight*0.5f);
			}
		}
	}

	mController = ctrl;
	return ctrl;
}

void ControlledActor::tryStandup()
{
	// overlap with upper part
	if(mType==PxControllerShapeType::eBOX)
	{
	}
	else if(mType==PxControllerShapeType::eCAPSULE)
	{
		PxScene* scene = mController->getScene();
		PxSceneReadLock scopedLock(*scene);
		
		PxCapsuleController* capsuleCtrl = static_cast<PxCapsuleController*>(mController);

		PxReal r = capsuleCtrl->getRadius();
		PxReal dh = mStandingSize - mCrouchingSize-2*r;
		PxCapsuleGeometry geom(r, dh*.5f);

		PxExtendedVec3 position = mController->getPosition();
		PxVec3 pos((float)position.x,(float)position.y+mStandingSize*.5f+r,(float)position.z);
		PxQuat orientation(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f));

		PxOverlapBuffer hit;
		if(scene->overlap(geom, PxTransform(pos,orientation), hit, PxQueryFilterData(PxQueryFlag::eANY_HIT|PxQueryFlag::eSTATIC|PxQueryFlag::eDYNAMIC)))
			return;
	}

	// if no hit, we can stand up
	resizeStanding();

	mDoStandup = false;
	mIsCrouching = false;
}

void ControlledActor::resizeController(PxReal height)
{
	PxSceneWriteLock scopedLock(mOwner.getActiveScene());
	mIsCrouching = true;
	mController->resize(height);
}



// PT: I'm forced to duplicate this code here for now, since otherwise "eACCELERATION" is banned

PX_INLINE void addForceAtPosInternal(PxRigidBody& body, const PxVec3& force, const PxVec3& pos, PxForceMode::Enum mode, bool wakeup)
{
/*	if(mode == PxForceMode::eACCELERATION || mode == PxForceMode::eVELOCITY_CHANGE)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
			"PxRigidBodyExt::addForce methods do not support eACCELERATION or eVELOCITY_CHANGE modes");
		return;
	}*/

	const PxTransform globalPose = body.getGlobalPose();
	const PxVec3 centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

	const PxVec3 torque = (pos - centerOfMass).cross(force);
	body.addForce(force, mode, wakeup);
	body.addTorque(torque, mode, wakeup);
}

static void addForceAtLocalPos(PxRigidBody& body, const PxVec3& force, const PxVec3& pos, PxForceMode::Enum mode, bool wakeup=true)
{
	//transform pos to world space
	const PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

	addForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

void defaultCCTInteraction(const PxControllerShapeHit& hit)
{
	PxRigidDynamic* actor = hit.shape->getActor()->is<PxRigidDynamic>();
	if(actor)
	{
		if(actor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
			return;

		if(0)
		{
			const PxVec3 p = actor->getGlobalPose().p + hit.dir * 10.0f;

			PxShape* shape;
			actor->getShapes(&shape, 1);
			PxRaycastHit newHit;
			PxU32 n = PxShapeExt::raycast(*shape, *shape->getActor(), p, -hit.dir, 20.0f, PxHitFlag::ePOSITION, 1, &newHit);
			if(n)
			{
				// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
				// useless stress on the solver. It would be possible to enable/disable vertical pushes on
				// particular objects, if the gameplay requires it.
				const PxVec3 upVector = hit.controller->getUpDirection();
				const PxF32 dp = hit.dir.dot(upVector);
		//		shdfnd::printFormatted("%f\n", fabsf(dp));
				if(fabsf(dp)<1e-3f)
		//		if(hit.dir.y==0.0f)
				{
					const PxTransform globalPose = actor->getGlobalPose();
					const PxVec3 localPos = globalPose.transformInv(newHit.position);
					::addForceAtLocalPos(*actor, hit.dir*hit.length*1000.0f, localPos, PxForceMode::eACCELERATION);
				}
			}
		}

		// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
		// useless stress on the solver. It would be possible to enable/disable vertical pushes on
		// particular objects, if the gameplay requires it.
		const PxVec3 upVector = hit.controller->getUpDirection();
		const PxF32 dp = hit.dir.dot(upVector);
//		shdfnd::printFormatted("%f\n", fabsf(dp));
		if(fabsf(dp)<1e-3f)
//		if(hit.dir.y==0.0f)
		{
			const PxTransform globalPose = actor->getGlobalPose();
			const PxVec3 localPos = globalPose.transformInv(toVec3(hit.worldPos));
			::addForceAtLocalPos(*actor, hit.dir*hit.length*1000.0f, localPos, PxForceMode::eACCELERATION);
		}
	}
}
