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


#include "ScBodySim.h"
#include "ScStaticSim.h"
#include "ScScene.h"
#include "ScRbElementInteraction.h"
#include "ScParticleBodyInteraction.h"
#include "ScShapeInteraction.h"
#include "ScTriggerInteraction.h"
#include "ScSimStats.h"
#include "ScObjectIDTracker.h"
#include "GuHeightFieldUtil.h"
#include "GuTriangleMesh.h"
#include "GuConvexMeshData.h"
#include "GuHeightField.h"
#include "PxsContext.h"
#include "BpSimpleAABBManager.h"
#include "PxsTransformCache.h"
#include "CmTransformUtils.h"
#include "GuBounds.h"
#include "PxsRigidBody.h"
#include "ScSqBoundsManager.h"
#include "PxsSimulationController.h"

#if PX_USE_PARTICLE_SYSTEM_API
#include "PtContext.h"
#endif

#if PX_USE_PARTICLE_SYSTEM_API && PX_SUPPORT_GPU_PHYSX
#include "PxSceneGpu.h"
#endif

using namespace physx;
using namespace Gu;

// PT: keep local functions in cpp, no need to pollute the header. Don't force conversions to bool if not necessary.
static PX_FORCE_INLINE PxU32 hasTriggerFlags(PxShapeFlags flags)	{ return PxU32(flags) & PxU32(PxShapeFlag::eTRIGGER_SHAPE);									}
static PX_FORCE_INLINE PxU32 isBroadPhase(PxShapeFlags flags)		{ return PxU32(flags) & PxU32(PxShapeFlag::eTRIGGER_SHAPE|PxShapeFlag::eSIMULATION_SHAPE);	}

namespace physx
{
	extern bool gUnifiedHeightfieldCollision;
}

static PX_FORCE_INLINE void resetElementID(Sc::Scene& scene, Sc::ShapeSim& shapeSim)
{
	PX_ASSERT(!shapeSim.isInBroadPhase());

	scene.getDirtyShapeSimMap().reset(shapeSim.getElementID());

	if(shapeSim.getSqBoundsId() != PX_INVALID_U32)
		shapeSim.destroySqBounds();
}

void Sc::ShapeSim::initSubsystemsDependingOnElementID()
{
	Sc::Scene& scScene = getScene();

	Bp::BoundsArray& boundsArray = scScene.getBoundsArray();
	const PxU32 index = getElementID();

	PX_ALIGN(16, PxTransform absPos);
	getAbsPoseAligned(&absPos);

	PxsTransformCache& cache = scScene.getLowLevelContext()->getTransformCache();
	cache.initEntry(index);
	cache.setTransformCache(absPos, 0, index);

	boundsArray.updateBounds(absPos, mCore.getGeometryUnion(), index, !gUnifiedHeightfieldCollision);
	
	{
		PX_PROFILE_ZONE("API.simAddShapeToBroadPhase", scScene.getContextId());
		if(isBroadPhase(mCore.getFlags()))
		{
			internalAddToBroadPhase();			
		}
		else
		{
			scScene.getAABBManager()->reserveSpaceForBounds(index);
		}
		scScene.updateContactDistance(index, getContactOffset());
	}

	if(scScene.getDirtyShapeSimMap().size() <= index)
		scScene.getDirtyShapeSimMap().resize(PxMax(index+1, (scScene.getDirtyShapeSimMap().size()+1) * 2u));

	RigidSim& owner = getRbSim();
	if(owner.isDynamicRigid() && static_cast<BodySim&>(owner).isActive())
		createSqBounds();

	// Init LL shape
	{
		mLLShape.mElementIndex = index;
		mLLShape.mShapeCore = const_cast<PxsShapeCore*>(&mCore.getCore());
	
		if(owner.getActorType()==PxActorType::eRIGID_STATIC)
		{
			mLLShape.mBodySimIndex = 0xffffffff;
		}
		else
		{
			BodySim& bodySim = static_cast<BodySim&>(getActor());
			const PxU32 nodeIndex = bodySim.getNodeIndex().index();
			mLLShape.mBodySimIndex = nodeIndex;
			//mLLShape.mLocalBound = computeBounds(mCore.getGeometry(), PxTransform(PxIdentity));
		}
	}
}

Sc::ShapeSim::ShapeSim(RigidSim& owner, const ShapeCore& core) :
	ElementSim	(owner, ElementType::eSHAPE),
	mCore		(core),
	mSqBoundsId	(PX_INVALID_U32)
{
	// sizeof(ShapeSim) = 32 bytes
	Sc::Scene& scScene = getScene();

	mId = scScene.getShapeIDTracker().createID();

	initSubsystemsDependingOnElementID();
}

Sc::ShapeSim::~ShapeSim()
{
	Sc::Scene& scScene = getScene();
	resetElementID(scScene, *this);
	scScene.getShapeIDTracker().releaseID(mId);
}

PX_FORCE_INLINE void Sc::ShapeSim::internalAddToBroadPhase()
{
	PX_ASSERT(!isInBroadPhase());
	addToAABBMgr(mCore.getContactOffset(), getRbSim().getBroadphaseGroupId(), !!(mCore.getCore().mShapeFlags & PxShapeFlag::eTRIGGER_SHAPE));
}

PX_FORCE_INLINE void Sc::ShapeSim::internalRemoveFromBroadPhase()
{
	PX_ASSERT(isInBroadPhase());
	removeFromAABBMgr();

	Sc::Scene& scene = getScene();
	PxsContactManagerOutputIterator outputs = scene.getLowLevelContext()->getNphaseImplementationContext()->getContactManagerOutputs();
	scene.getNPhaseCore()->onVolumeRemoved(this, PxU32(PairReleaseFlag::eWAKE_ON_LOST_TOUCH), outputs, scene.getPublicFlags() & PxSceneFlag::eADAPTIVE_FORCE);
}

void Sc::ShapeSim::removeFromBroadPhase(bool wakeOnLostTouch)
{
	if(isInBroadPhase())
	{
		// PT: TODO: refactor with internalRemoveFromBroadPhase()
		removeFromAABBMgr();
		Sc::Scene& scene = getScene();
		PxsContactManagerOutputIterator outputs = scene.getLowLevelContext()->getNphaseImplementationContext()->getContactManagerOutputs();
		scene.getNPhaseCore()->onVolumeRemoved(this, wakeOnLostTouch ? PxU32(PairReleaseFlag::eWAKE_ON_LOST_TOUCH) : 0, outputs, scene.getPublicFlags() & PxSceneFlag::eADAPTIVE_FORCE);
	}
}

void Sc::ShapeSim::reinsertBroadPhase()
{
	internalRemoveFromBroadPhase();
//	internalAddToBroadPhase();

	Sc::Scene& scene = getScene();

	// Sc::Scene::removeShape
	{
		//unregisterShapeFromNphase(shape.getCore());

		// PT: "getID" is const but the addShape call used LLShape, which uses elementID, so....
		scene.getSimulationController()->removeShape(getID());
	}

	// Call ShapeSim dtor
	{
		resetElementID(scene, *this);
	}

	// Call ElementSim dtor
	{
		releaseID();
	}

	// Call ElementSim ctor
	{
		initID();
	}

	// Call ShapeSim ctor
	{
		initSubsystemsDependingOnElementID();
	}

	// Sc::Scene::addShape
	{
		scene.getSimulationController()->addShape(&getLLShapeSim(), getID());
		// PT: TODO: anything else needed here?
		//registerShapeInNphase(shapeCore);
	}
}

void Sc::ShapeSim::onFilterDataChange()
{
	setElementInteractionsDirty(InteractionDirtyFlag::eFILTER_STATE, InteractionFlag::eFILTERABLE);
}

void Sc::ShapeSim::onResetFiltering()
{
	if(isInBroadPhase())
		reinsertBroadPhase();
}

void Sc::ShapeSim::onMaterialChange()
{
	setElementInteractionsDirty(InteractionDirtyFlag::eMATERIAL, InteractionFlag::eRB_ELEMENT);
}

void Sc::ShapeSim::onRestOffsetChange()
{
	setElementInteractionsDirty(InteractionDirtyFlag::eREST_OFFSET, InteractionFlag::eRB_ELEMENT);
}

void Sc::ShapeSim::onContactOffsetChange()
{
	if(isInBroadPhase())
		getScene().getAABBManager()->setContactOffset(getElementID(), mCore.getContactOffset());
}

void Sc::ShapeSim::onFlagChange(PxShapeFlags oldFlags)
{
	PxShapeFlags newFlags = mCore.getFlags();

	const PxSceneFlags sceneFlags = getScene().getPublicFlags();

	const bool oldBp = isBroadPhase(oldFlags)!=0;
	const bool newBp = isBroadPhase(newFlags)!=0;

	// Change of collision shape flags requires removal/add to broadphase
	if(oldBp != newBp)
	{
		if(!oldBp && newBp)
		{
			PX_ASSERT(!isInBroadPhase());
			internalAddToBroadPhase();
		}
		else
		{
			internalRemoveFromBroadPhase();
		}
	}
	else
	{
		const bool wasTrigger = hasTriggerFlags(oldFlags)!=0;
		const bool isTrigger = hasTriggerFlags(newFlags)!=0;
		if(!(sceneFlags & PxSceneFlag::eDEPRECATED_TRIGGER_TRIGGER_REPORTS))
		{
			if (wasTrigger != isTrigger)
				reinsertBroadPhase();  // re-insertion is necessary because trigger-trigger pairs get killed
		}
		else
		{
			getScene().getAABBManager()->setVolumeType(this->getElementID(), PxU8((isTrigger ? Sc::ElementType::eTRIGGER : getElementType())));
			if (wasTrigger != isTrigger)
				setElementInteractionsDirty(InteractionDirtyFlag::eFILTER_STATE, InteractionFlag::eFILTERABLE);
		}
	}

	PxShapeFlags hadSq = oldFlags&PxShapeFlag::eSCENE_QUERY_SHAPE, hasSq = newFlags&PxShapeFlag::eSCENE_QUERY_SHAPE;
	if(hasSq && !hadSq)
	{
		if(getBodySim() &&  getBodySim()->isActive())
			createSqBounds();
	}
	else if(hadSq && !hasSq)
		destroySqBounds();
}

void Sc::ShapeSim::getFilterInfo(PxFilterObjectAttributes& filterAttr, PxFilterData& filterData) const
{
	filterAttr = 0;
	const PxShapeFlags flags = mCore.getFlags();

	if (hasTriggerFlags(flags))
		filterAttr |= PxFilterObjectFlag::eTRIGGER;

	BodySim* b = getBodySim();
	if (b)
	{
		if (!b->isArticulationLink())
		{
			if (b->isKinematic())
				filterAttr |= PxFilterObjectFlag::eKINEMATIC;

			setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eRIGID_DYNAMIC);
		}
		else
			setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eARTICULATION);
	}
	else
	{
		setFilterObjectAttributeType(filterAttr, PxFilterObjectType::eRIGID_STATIC);
	}

	filterData = mCore.getSimulationFilterData();
}

void Sc::ShapeSim::getAbsPoseAligned(PxTransform* PX_RESTRICT globalPose) const
{
	const PxTransform& shape2Actor = mCore.getCore().transform;
	const PxTransform* actor2World = NULL;
	if(getActor().getActorType()==PxActorType::eRIGID_STATIC)
	{
		PxsRigidCore& core = static_cast<StaticSim&>(getActor()).getStaticCore().getCore();
		actor2World = &core.body2World;
	}
	else
	{
		PxsBodyCore& core = static_cast<BodySim&>(getActor()).getBodyCore().getCore();
		if(!core.mIdtBody2Actor)
		{
			Cm::getDynamicGlobalPoseAligned(core.body2World, shape2Actor, core.getBody2Actor(), *globalPose);
			return;
		}
		actor2World = &core.body2World;
	}
	Cm::getStaticGlobalPoseAligned(*actor2World, shape2Actor, *globalPose);
}

Sc::BodySim* Sc::ShapeSim::getBodySim() const	
{ 
	ActorSim& a = getActor();
	return a.isDynamicRigid() ? static_cast<BodySim*>(&a) : NULL;
}

PxsRigidCore& Sc::ShapeSim::getPxsRigidCore() const
{
	ActorSim& a = getActor();
	return a.isDynamicRigid() ? static_cast<BodySim&>(a).getBodyCore().getCore()
							  : static_cast<StaticSim&>(a).getStaticCore().getCore();
}

bool Sc::ShapeSim::actorIsDynamic() const
{
	return getActor().isDynamicRigid();
}

void Sc::ShapeSim::updateCached(PxU32 transformCacheFlags, Cm::BitMapPinned* shapeChangedMap)
{
	PX_ALIGN(16, PxTransform absPose);
	getAbsPoseAligned(&absPose);

	Sc::Scene& scene = getScene();
	const PxU32 index = getElementID();

	scene.getLowLevelContext()->getTransformCache().setTransformCache(absPose, transformCacheFlags, index);
	scene.getBoundsArray().updateBounds(absPose, mCore.getGeometryUnion(), index, !gUnifiedHeightfieldCollision);
	if (shapeChangedMap && isInBroadPhase())
		shapeChangedMap->growAndSet(index);
}

void Sc::ShapeSim::updateCached(PxsTransformCache& transformCache, Bp::BoundsArray& boundsArray)
{
	const PxU32 index = getElementID();

	PxsCachedTransform& ct = transformCache.getTransformCache(index);
	Ps::prefetchLine(&ct);

	getAbsPoseAligned(&ct.transform);

	ct.flags = 0;

	PxBounds3& b = boundsArray.begin()[index];
	Gu::computeBounds(b, mCore.getGeometryUnion().getGeometry(), ct.transform, 0.0f, NULL, 1.0f, !physx::gUnifiedHeightfieldCollision);
}

void Sc::ShapeSim::updateContactDistance(PxReal* contactDistance, const PxReal inflation, const PxVec3 angVel, const PxReal dt, Bp::BoundsArray& boundsArray)
{
	const PxU32 index = getElementID();

	const PxBounds3& bounds = boundsArray.getBounds(index);

	PxReal radius = bounds.getExtents().magnitude();

	//Heuristic for angular velocity...
	PxReal angularInflation = angVel.magnitude() * dt * radius;

	contactDistance[index] = getContactOffset() + inflation + angularInflation;
}

Ps::IntBool Sc::ShapeSim::updateSweptBounds()
{
	Vec3p endOrigin, endExtent;
	const ShapeCore& shapeCore = mCore;
	const PxTransform& endPose = getScene().getLowLevelContext()->getTransformCache().getTransformCache(getElementID()).transform;
	PxReal ccdThreshold = computeBoundsWithCCDThreshold(endOrigin, endExtent, shapeCore.getGeometry(), endPose, NULL);

	PxBounds3 bounds = PxBounds3::centerExtents(endOrigin, endExtent);

	PxcRigidBody& rigidBody = getBodySim()->getLowLevelBody();
	PxsBodyCore& bodyCore = getBodySim()->getBodyCore().getCore();
	PX_ALIGN(16, PxTransform shape2World);
	Cm::getDynamicGlobalPoseAligned(rigidBody.mLastTransform, shapeCore.getShape2Actor(), bodyCore.getBody2Actor(), shape2World);
	PxBounds3 startBounds = computeBounds(shapeCore.getGeometry(), shape2World, !physx::gUnifiedHeightfieldCollision);

	const Ps::IntBool isFastMoving = (startBounds.getCenter() - endOrigin).magnitudeSquared() >= ccdThreshold * ccdThreshold ? 1 : 0;

	if (isFastMoving)
		bounds.include(startBounds);

	PX_ASSERT(bounds.minimum.x <= bounds.maximum.x
		&&	  bounds.minimum.y <= bounds.maximum.y
		&&	  bounds.minimum.z <= bounds.maximum.z);

	getScene().getBoundsArray().setBounds(bounds, getElementID());

	return isFastMoving;	
}

void Sc::ShapeSim::onVolumeOrTransformChange(bool asPartOfActorTransformChange, bool forceBoundsUpdate)
{
	Sc::Scene& scene = getScene();

	Sc::BodySim* body = getBodySim();
	const bool isDynamic = (body != NULL);
	bool isAsleep;
	if (body)
	{
		isAsleep = !body->isActive();
		body->postShapeChange(asPartOfActorTransformChange);
	}
	else
		isAsleep = true;

	ElementSim::ElementInteractionIterator iter = getElemInteractions();
	ElementSimInteraction* i = iter.getNext();
	while(i)
	{
		if(i->getType() == InteractionType::eOVERLAP)
		{
			Sc::ShapeInteraction* si = static_cast<Sc::ShapeInteraction*>(i);
			si->resetManagerCachedState();
			
			if (isAsleep)
				si->onShapeChangeWhileSleeping(isDynamic);
		}
		else if (i->getType() == InteractionType::eTRIGGER)
			(static_cast<Sc::TriggerInteraction*>(i))->forceProcessingThisFrame(scene);  // trigger pairs need to be checked next frame
#if PX_USE_PARTICLE_SYSTEM_API
		else if (i->getType() == InteractionType::ePARTICLE_BODY)
			(static_cast<Sc::ParticleElementRbElementInteraction *>(i))->onRbShapeChange();
#endif

		i = iter.getNext();
	}


	const PxU32 elementID = getElementID();

	if (forceBoundsUpdate)
	{
		updateCached(0, &scene.getAABBManager()->getChangedAABBMgActorHandleMap());
	}
	else if (isInBroadPhase())
		scene.getDirtyShapeSimMap().growAndSet(elementID);

#if PX_USE_PARTICLE_SYSTEM_API
#if PX_SUPPORT_GPU_PHYSX
	// PT: onShapeChange currently only used for GPU physics. Inlined 'getSceneGpu' call avoids
	// extra function calls and additional work from getPxsRigidCore(), etc
	Pt::Context* context = scene.getParticleContext();
	if(context->getSceneGpuFast())
	{
		context->getSceneGpuFast()->onShapeChange(size_t(&mCore.getCore()), size_t(&getPxsRigidCore()), (body != NULL));
	}
#endif
#endif
}

void Sc::ShapeSim::createSqBounds()
{
	if(mSqBoundsId!=PX_INVALID_U32)
		return;

	Sc::BodySim* bodySim = getBodySim();

	PX_ASSERT(bodySim);

	if(bodySim->usingSqKinematicTarget() || bodySim->isFrozen() || !bodySim->isActive())
		return;

	if(mCore.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE)
		getScene().getSqBoundsManager().addShape(*this);
}

void Sc::ShapeSim::destroySqBounds()
{
	if(mSqBoundsId!=PX_INVALID_U32)
		getScene().getSqBoundsManager().removeShape(*this);
}

