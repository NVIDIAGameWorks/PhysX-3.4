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
// Copyright (c) 2008-2016 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "NpShapeManager.h"
#include "NpFactory.h"
#include "ScbRigidObject.h"
#include "NpActor.h"
#include "SqSceneQueryManager.h"
#include "SqPruningStructure.h"
#include "NpScene.h"
#include "NpPtrTableStorageManager.h"
#include "GuBounds.h"
#include "CmUtils.h"

using namespace physx;
using namespace Sq;

namespace physx
{
	extern bool gUnifiedHeightfieldCollision;
}

static PX_FORCE_INLINE bool isSceneQuery(const NpShape& shape) { return shape.getFlagsFast() & PxShapeFlag::eSCENE_QUERY_SHAPE; }

NpShapeManager::NpShapeManager()
	: mPruningStructure(NULL)
{
}

// PX_SERIALIZATION
NpShapeManager::NpShapeManager(const PxEMPTY) :
	mShapes			(PxEmpty),
	mSceneQueryData	(PxEmpty) 
{	
}

NpShapeManager::~NpShapeManager() 
{ 
	PX_ASSERT(!mPruningStructure);
	Cm::PtrTableStorageManager& sm = NpFactory::getInstance().getPtrTableStorageManager();
	mShapes.clear(sm);
	mSceneQueryData.clear(sm);	
}

void NpShapeManager::exportExtraData(PxSerializationContext& stream)
{ 
	mShapes.exportExtraData(stream);							
	mSceneQueryData.exportExtraData(stream);
}

void NpShapeManager::importExtraData(PxDeserializationContext& context)
{ 
	mShapes.importExtraData(context);	
	mSceneQueryData.importExtraData(context);
}
//~PX_SERIALIZATION

void NpShapeManager::attachShape(NpShape& shape, PxRigidActor& actor)
{
	PX_ASSERT(!mPruningStructure);

	Cm::PtrTableStorageManager& sm = NpFactory::getInstance().getPtrTableStorageManager();

	const PxU32 index = getNbShapes();
	mShapes.add(&shape, sm);	
	mSceneQueryData.add(reinterpret_cast<void*>(size_t(SQ_INVALID_PRUNER_DATA)), sm);

	NpScene* scene = NpActor::getAPIScene(actor);		
	if(scene && isSceneQuery(shape))
		setupSceneQuery(scene->getSceneQueryManagerFast(), actor, index);

	Scb::RigidObject& ro = static_cast<Scb::RigidObject&>(NpActor::getScbFromPxActor(actor));
	ro.onShapeAttach(shape.getScbShape());	

	PX_ASSERT(!shape.isExclusive() || shape.getActor()==NULL);
	shape.onActorAttach(actor);
}
				 
void NpShapeManager::detachShape(NpShape& s, PxRigidActor& actor, bool wakeOnLostTouch)
{
	PX_ASSERT(!mPruningStructure);

	Cm::PtrTableStorageManager& sm = NpFactory::getInstance().getPtrTableStorageManager();

	const PxU32 index = mShapes.find(&s);
	PX_ASSERT(index!=0xffffffff);

	Scb::RigidObject& ro = static_cast<Scb::RigidObject&>(NpActor::getScbFromPxActor(actor));

	NpScene* scene = NpActor::getAPIScene(actor);
	if(scene && isSceneQuery(s))
		scene->getSceneQueryManagerFast().removePrunerShape(getPrunerData(index));

	Scb::Shape& scbShape = s.getScbShape();
	ro.onShapeDetach(scbShape, wakeOnLostTouch, (s.getRefCount() == 1));
	mShapes.replaceWithLast(index, sm);
	mSceneQueryData.replaceWithLast(index, sm);
	
	s.onActorDetach();
}

bool NpShapeManager::shapeIsAttached(NpShape& s) const
{
	return mShapes.find(&s)!=0xffffffff;
}

void NpShapeManager::detachAll(NpScene* scene)
{
	// assumes all SQ data has been released, which is currently the responsbility of the owning actor
	const PxU32 nbShapes = getNbShapes();
	NpShape*const *shapes = getShapes();

	if(scene)
		teardownAllSceneQuery(scene->getSceneQueryManagerFast()); 

	// actor cleanup in Scb/Sc will remove any outstanding references corresponding to sim objects, so we don't need to do that here.
	for(PxU32 i=0;i<nbShapes;i++)
		shapes[i]->onActorDetach();

	Cm::PtrTableStorageManager& sm = NpFactory::getInstance().getPtrTableStorageManager();

	mShapes.clear(sm);
	mSceneQueryData.clear(sm);
}

PxU32 NpShapeManager::getShapes(PxShape** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return Cm::getArrayOfPointers(buffer, bufferSize, startIndex, getShapes(), getNbShapes());
}

PxBounds3 NpShapeManager::getWorldBounds(const PxRigidActor& actor) const
{
	PxBounds3 bounds(PxBounds3::empty());

	const PxU32 nbShapes = getNbShapes();
	PxTransform actorPose = actor.getGlobalPose();
	NpShape*const* PX_RESTRICT shapes = getShapes();

	for(PxU32 i=0;i<nbShapes;i++)
		bounds.include(Gu::computeBounds(shapes[i]->getScbShape().getGeometry(), actorPose * shapes[i]->getLocalPoseFast(), !physx::gUnifiedHeightfieldCollision));
		
	return bounds;
}

void NpShapeManager::clearShapesOnRelease(Scb::Scene& s, PxRigidActor& r)
{
	PX_ASSERT(static_cast<Scb::RigidObject&>(NpActor::getScbFromPxActor(r)).isSimDisabledInternally());
	
	const PxU32 nbShapes = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes();

	for(PxU32 i=0;i<nbShapes;i++)
	{
		Scb::Shape& scbShape = shapes[i]->getScbShape();
		scbShape.checkUpdateOnRemove<false>(&s);
#if PX_SUPPORT_PVD
		s.getScenePvdClient().releasePvdInstance(&scbShape, r);
#else
		PX_UNUSED(r);
#endif
	}
}

void NpShapeManager::releaseExclusiveUserReferences()
{
	// when the factory is torn down, release any shape owner refs that are still outstanding
	const PxU32 nbShapes = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes();
	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(shapes[i]->isExclusiveFast() && shapes[i]->getRefCount()>1)
			shapes[i]->release();
	}
}

void NpShapeManager::setupSceneQuery(SceneQueryManager& sqManager, const PxRigidActor& actor, const NpShape& shape)
{ 
	PX_ASSERT(shape.getFlags() & PxShapeFlag::eSCENE_QUERY_SHAPE);
	const PxU32 index = mShapes.find(&shape);
	PX_ASSERT(index!=0xffffffff);
	setupSceneQuery(sqManager, actor, index);
}

void NpShapeManager::teardownSceneQuery(SceneQueryManager& sqManager, const NpShape& shape)
{
	const PxU32 index = mShapes.find(&shape);
	PX_ASSERT(index!=0xffffffff);
	teardownSceneQuery(sqManager, index);
}

void NpShapeManager::setupAllSceneQuery(NpScene* scene, const PxRigidActor& actor, bool hasPrunerStructure, const PxBounds3* bounds)
{ 
	PX_ASSERT(scene);		// shouldn't get here unless we're in a scene
	SceneQueryManager& sqManager = scene->getSceneQueryManagerFast();

	const PxU32 nbShapes = getNbShapes();
	NpShape*const *shapes = getShapes();

	const PxType actorType = actor.getConcreteType();
	const bool isDynamic = actorType == PxConcreteType::eRIGID_DYNAMIC || actorType == PxConcreteType::eARTICULATION_LINK;

	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(isSceneQuery(*shapes[i]))
			setPrunerData(i, sqManager.addPrunerShape(*shapes[i], actor, isDynamic, bounds ? bounds + i : NULL, hasPrunerStructure));
	}
}

void NpShapeManager::teardownAllSceneQuery(SceneQueryManager& sqManager)
{
	NpShape*const *shapes = getShapes();
	const PxU32 nbShapes = getNbShapes();

	for(PxU32 i=0;i<nbShapes;i++)
	{
		if(isSceneQuery(*shapes[i]))
			sqManager.removePrunerShape(getPrunerData(i));

		setPrunerData(i, SQ_INVALID_PRUNER_DATA);
	}
}

void NpShapeManager::markAllSceneQueryForUpdate(SceneQueryManager& sqManager)
{
	const PxU32 nbShapes = getNbShapes();

	for(PxU32 i=0;i<nbShapes;i++)
	{
		const PrunerData data = getPrunerData(i);
		if(data!=SQ_INVALID_PRUNER_DATA)
			sqManager.markForUpdate(data);
	}
}

Sq::PrunerData NpShapeManager::findSceneQueryData(const NpShape& shape) const
{
	const PxU32 index = mShapes.find(&shape);
	PX_ASSERT(index!=0xffffffff);

	return getPrunerData(index);
}

//
// internal methods
// 

void NpShapeManager::setupSceneQuery(SceneQueryManager& sqManager, const PxRigidActor& actor, PxU32 index)
{ 
	const PxType actorType = actor.getConcreteType();
	const bool isDynamic = actorType == PxConcreteType::eRIGID_DYNAMIC || actorType == PxConcreteType::eARTICULATION_LINK;
	setPrunerData(index, sqManager.addPrunerShape(*(getShapes()[index]), actor, isDynamic));
}

void NpShapeManager::teardownSceneQuery(SceneQueryManager& sqManager, PxU32 index)
{
	sqManager.removePrunerShape(getPrunerData(index));
	setPrunerData(index, SQ_INVALID_PRUNER_DATA);
}

#if PX_ENABLE_DEBUG_VISUALIZATION
void NpShapeManager::visualize(Cm::RenderOutput& out, NpScene* scene, const PxRigidActor& actor)
{
	const PxU32 nbShapes = getNbShapes();
	NpShape*const* PX_RESTRICT shapes = getShapes();
	PxTransform actorPose = actor.getGlobalPose();

	const bool visualizeCompounds = (nbShapes>1) && scene->getVisualizationParameter(PxVisualizationParameter::eCOLLISION_COMPOUNDS)!=0.0f;

	PxBounds3 compoundBounds(PxBounds3::empty());
	for(PxU32 i=0;i<nbShapes;i++)
	{
		Scb::Shape& shape = shapes[i]->getScbShape();
		if(shape.getFlags() & PxShapeFlag::eVISUALIZATION)
		{
			shapes[i]->visualize(out, actor);
			if(visualizeCompounds)
				compoundBounds.include(Gu::computeBounds(shape.getGeometry(), actorPose*shapes[i]->getLocalPose(), !physx::gUnifiedHeightfieldCollision));
		}
	}
	if(visualizeCompounds && !compoundBounds.isEmpty())
		out << PxU32(PxDebugColor::eARGB_MAGENTA) << PxMat44(PxIdentity) << Cm::DebugBox(compoundBounds);
}
#endif  // PX_ENABLE_DEBUG_VISUALIZATION
