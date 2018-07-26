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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "NpActor.h"
#include "PxRigidActor.h"
#include "NpConstraint.h"
#include "NpFactory.h"
#include "NpShape.h"
#include "NpPhysics.h"
#include "NpAggregate.h"
#include "NpScene.h"
#include "NpRigidStatic.h"
#include "NpRigidDynamic.h"
#include "NpParticleSystem.h"
#include "NpParticleFluid.h"
#include "NpArticulationLink.h"
#include "NpCloth.h"
#include "CmTransformUtils.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

NpActor::NpActor(const char* name) :
	mName(name),
	mConnectorArray(NULL)
{
}

NpActor::~NpActor()
{
}


typedef Ps::HashMap<NpActor*, NpConnectorArray*> ConnectorMap;
struct NpActorUserData
{
	PxU32				referenceCount;
	ConnectorMap*		tmpOriginalConnectors;
};

void NpActor::exportExtraData(PxSerializationContext& stream)
{
	const PxCollection& collection = stream.getCollection();
	if(mConnectorArray)
	{		
		PxU32 connectorSize = mConnectorArray->size();	
		PxU32 missedCount = 0;
		for(PxU32 i = 0; i < connectorSize; ++i)
		{
			NpConnector& c = (*mConnectorArray)[i];
			PxBase* object = c.mObject;
			if(!collection.contains(*object))
			{
				++missedCount;
			}
		}
		
		NpConnectorArray* exportConnectorArray = mConnectorArray;

		if(missedCount > 0)
		{
			exportConnectorArray = NpFactory::getInstance().acquireConnectorArray();
			if(missedCount < connectorSize)
			{
				exportConnectorArray->reserve(connectorSize - missedCount);	
				for(PxU32 i = 0; i < connectorSize; ++i)
				{
					NpConnector& c = (*mConnectorArray)[i];
					PxBase* object = c.mObject;
					if(collection.contains(*object))
					{
						exportConnectorArray->pushBack(c);
					}
				}
			}			
		}

		stream.alignData(PX_SERIAL_ALIGN);
		stream.writeData(exportConnectorArray, sizeof(NpConnectorArray));
		Cm::exportInlineArray(*exportConnectorArray, stream);

		if(missedCount > 0)
			 NpFactory::getInstance().releaseConnectorArray(exportConnectorArray);
	}
	stream.writeName(mName);
}

void NpActor::importExtraData(PxDeserializationContext& context)
{
	if(mConnectorArray)
	{
		mConnectorArray = context.readExtraData<NpConnectorArray, PX_SERIAL_ALIGN>();
		new (mConnectorArray) NpConnectorArray(PxEmpty);

		if(mConnectorArray->size() == 0)
			mConnectorArray = NULL;
		else
			Cm::importInlineArray(*mConnectorArray, context);
	}
	context.readName(mName);
}

void NpActor::resolveReferences(PxDeserializationContext& context)
{
	// Resolve connector pointers if needed
	if(mConnectorArray)
	{
		const PxU32 nbConnectors = mConnectorArray->size();
		for(PxU32 i=0; i<nbConnectors; i++)
		{
			NpConnector& c = (*mConnectorArray)[i];
			context.translatePxBase(c.mObject);
		}
	}	
}

///////////////////////////////////////////////////////////////////////////////

void NpActor::releaseConstraints(PxRigidActor& owner)
{
	if(mConnectorArray)
	{
		PxU32 nbConnectors = mConnectorArray->size();
		PxU32 currentIndex = 0;
		while(nbConnectors--)
		{
			NpConnector& connector = (*mConnectorArray)[currentIndex];
			if (connector.mType == NpConnectorType::eConstraint)
			{
				NpConstraint* c = static_cast<NpConstraint*>(connector.mObject);
				c->actorDeleted(&owner);

				NpScene* s = c->getNpScene();
				if (s)
				{
					s->getScene().removeConstraint(c->getScbConstraint());
					s->removeFromConstraintList(*c);
				}

				removeConnector(owner, currentIndex);
			}
			else
				currentIndex++;
		}
	}
}

void NpActor::release(PxActor& owner)
{
// PX_AGGREGATE
	if (mConnectorArray)  // Need to test again because the code above might purge the connector array if no element remains
	{
		PX_ASSERT(mConnectorArray->size() == 1);  // At this point only the aggregate should remain
		PX_ASSERT((*mConnectorArray)[0].mType == NpConnectorType::eAggregate);

		NpAggregate* a = static_cast<NpAggregate*>((*mConnectorArray)[0].mObject);
		bool status = a->removeActorAndReinsert(owner, false);
		PX_ASSERT(status);
		PX_UNUSED(status);
		PX_ASSERT(!mConnectorArray);  // Remove should happen in aggregate code
	}
//~PX_AGGREGATE

	PX_ASSERT(!mConnectorArray);  // All the connector objects should have been removed at this point
}

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

PxU32 NpActor::findConnector(NpConnectorType::Enum type, PxBase* object) const
{
	if(!mConnectorArray)
		return 0xffffffff;

	for(PxU32 i=0; i < mConnectorArray->size(); i++)
	{
		NpConnector& c = (*mConnectorArray)[i];
		if (c.mType == type && c.mObject == object)
			return i;
	}

	return 0xffffffff;
}

void NpActor::addConnector(NpConnectorType::Enum type, PxBase* object, const char* errMsg)
{
	if(!mConnectorArray)
		mConnectorArray = NpFactory::getInstance().acquireConnectorArray();

	PX_CHECK_MSG(findConnector(type, object) == 0xffffffff, errMsg);
	PX_UNUSED(errMsg);
		
	if(mConnectorArray->isInUserMemory() && mConnectorArray->size() == mConnectorArray->capacity())
	{		
		NpConnectorArray* newConnectorArray = NpFactory::getInstance().acquireConnectorArray();		
		newConnectorArray->assign(mConnectorArray->begin(), mConnectorArray->end());
		mConnectorArray->~NpConnectorArray();
		mConnectorArray = newConnectorArray;
	}

	NpConnector c(type, object);
	mConnectorArray->pushBack(c);
}

void NpActor::removeConnector(PxActor& /*owner*/, PxU32 index)
{
	PX_ASSERT(mConnectorArray);
	PX_ASSERT(index < mConnectorArray->size());
	
	mConnectorArray->replaceWithLast(index);

	if(mConnectorArray->size() == 0)
	{
		if(!mConnectorArray->isInUserMemory())
			NpFactory::getInstance().releaseConnectorArray(mConnectorArray);
		else
			mConnectorArray->~NpConnectorArray();
		mConnectorArray = NULL;
	}
}

void NpActor::removeConnector(PxActor& owner, NpConnectorType::Enum type, PxBase* object, const char* errorMsg)
{
	PX_CHECK_MSG(mConnectorArray, errorMsg);
	PX_UNUSED(errorMsg);

	if(mConnectorArray)
	{
		PxU32 index = findConnector(type, object);

		PX_CHECK_MSG(index != 0xffffffff, errorMsg);

		removeConnector(owner, index);
	}
}

PxU32 NpActor::getNbConnectors(NpConnectorType::Enum type) const
{
	PxU32 nbConnectors = 0;

	if(mConnectorArray)
	{
		for(PxU32 i=0; i < mConnectorArray->size(); i++)
		{
			if ((*mConnectorArray)[i].mType == type)
				nbConnectors++;
		}
	}

	return nbConnectors;
}

///////////////////////////////////////////////////////////////////////////////

NpAggregate* NpActor::getNpAggregate(PxU32& index) const
{
	PX_ASSERT(getNbConnectors(NpConnectorType::eAggregate) <= 1);

	if(mConnectorArray)
	{
		// PT: TODO: sort by type to optimize this...
		for(PxU32 i=0; i < mConnectorArray->size(); i++)
		{
			NpConnector& c = (*mConnectorArray)[i];
			if (c.mType == NpConnectorType::eAggregate)
			{
				index = i;
				return static_cast<NpAggregate*>(c.mObject);
			}
		}
	}

	return NULL;
}

void NpActor::setAggregate(NpAggregate* np, PxActor& owner)
{
	PxU32 index = 0xffffffff;
	NpAggregate* a = getNpAggregate(index);

	if (!a)
	{
		PX_ASSERT(np);
		addConnector(NpConnectorType::eAggregate, np, "NpActor::setAggregate() failed");
	}
	else
	{
		PX_ASSERT(mConnectorArray);
		PX_ASSERT(index != 0xffffffff);
		if (!np)
			removeConnector(owner, index);
		else
			(*mConnectorArray)[index].mObject = np;
	}
}

PxAggregate* NpActor::getAggregate()	const
{
	PxU32 index = 0xffffffff;
	NpAggregate* a = getNpAggregate(index);
	return static_cast<PxAggregate*>(a);
}


///////////////////////////////////////////////////////////////////////////////

void NpActor::removeConstraintsFromScene()
{
	NpConnectorIterator iter = getConnectorIterator(NpConnectorType::eConstraint);
	while (PxBase* ser = iter.getNext())
	{
		NpConstraint* c = static_cast<NpConstraint*>(ser);
			
		NpScene* s = c->getNpScene();

		if (s)
		{
			s->removeFromConstraintList(*c);
			s->getScene().removeConstraint(c->getScbConstraint());
		}
	}
}

void NpActor::addConstraintsToSceneInternal()
{
	if(!mConnectorArray)
		return;

	NpConnectorIterator iter = getConnectorIterator(NpConnectorType::eConstraint);
	while (PxBase* ser = iter.getNext())
	{
		NpConstraint* c = static_cast<NpConstraint*>(ser);
		PX_ASSERT(c->getNpScene() == NULL);

		c->markDirty();	// PT: "temp" fix for crash when removing/re-adding jointed actor from/to a scene

		NpScene* s = c->getSceneFromActors();
		if (s)
		{
			s->addToConstraintList(*c);
			s->getScene().addConstraint(c->getScbConstraint());
		}
	}
}



///////////////////////////////////////////////////////////////////////////////

NpShapeManager* NpActor::getShapeManager(PxRigidActor& actor)
{
	// DS: if the performance here becomes an issue we can use the same kind of offset hack as below

	const PxType actorType = actor.getConcreteType();

	if (actorType == PxConcreteType::eRIGID_DYNAMIC)
		return &static_cast<NpRigidDynamic&>(actor).getShapeManager();
	else if(actorType == PxConcreteType::eRIGID_STATIC)
		return &static_cast<NpRigidStatic&>(actor).getShapeManager();
	else if (actorType == PxConcreteType::eARTICULATION_LINK)
		return &static_cast<NpArticulationLink&>(actor).getShapeManager();
	else
	{
		PX_ASSERT(0);
		return NULL;
	}
}

const NpShapeManager* NpActor::getShapeManager(const PxRigidActor& actor)
{
	// DS: if the performance here becomes an issue we can use the same kind of offset hack as below

	const PxType actorType = actor.getConcreteType();

	if (actorType == PxConcreteType::eRIGID_DYNAMIC)
		return &static_cast<const NpRigidDynamic&>(actor).getShapeManager();
	else if(actorType == PxConcreteType::eRIGID_STATIC)
		return &static_cast<const NpRigidStatic&>(actor).getShapeManager();
	else if (actorType == PxConcreteType::eARTICULATION_LINK)
		return &static_cast<const NpArticulationLink&>(actor).getShapeManager();
	else
	{
		PX_ASSERT(0);
		return NULL;
	}
}

void NpActor::getGlobalPose(PxTransform& globalPose, const NpShape& shape, const PxRigidActor& actor)
{
	const Scb::Actor& scbActor = NpActor::getScbFromPxActor(actor);
	const Scb::Shape& scbShape = shape.getScbShape();

	NpActor::getGlobalPose(globalPose, scbShape, scbActor);
}

void NpActor::getGlobalPose(PxTransform& globalPose, const Scb::Shape& scbShape, const Scb::Actor& scbActor)
{
	const PxTransform& shape2Actor = scbShape.getShape2Actor();

	// PT: TODO: duplicated from SqBounds.cpp. Refactor.
	const ScbType::Enum actorType = scbActor.getScbType();
	if(actorType==ScbType::eRIGID_STATIC)
	{
		Cm::getStaticGlobalPoseAligned(static_cast<const Scb::RigidStatic&>(scbActor).getActor2World(), shape2Actor, globalPose);
	}
	else
	{
		PX_ASSERT(actorType==ScbType::eBODY || actorType == ScbType::eBODY_FROM_ARTICULATION_LINK);

		const Scb::Body& body = static_cast<const Scb::Body&>(scbActor);
		PX_ALIGN(16, PxTransform) kinematicTarget;
		const PxU16 sqktFlags = PxRigidBodyFlag::eKINEMATIC | PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES;
		const bool useTarget = (PxU16(body.getFlags()) & sqktFlags) == sqktFlags;
		const PxTransform& body2World = (useTarget && body.getKinematicTarget(kinematicTarget)) ? kinematicTarget : body.getBody2World();
		Cm::getDynamicGlobalPoseAligned(body2World, shape2Actor, body.getBody2Actor(), globalPose);
	}
}

namespace
{
	template <typename N> NpActor* pxToNpActor(PxActor *p) 
	{  
		return static_cast<NpActor*>(static_cast<N*>(p));
	}
}

NpActor::Offsets::Offsets()
{
	for(PxU32 i=0;i<PxConcreteType::ePHYSX_CORE_COUNT;i++)
		pxActorToScbActor[i] = pxActorToNpActor[i] = 0;
	size_t addr = 0x100;	// casting the null ptr takes a special-case code path, which we don't want
	PxActor* n = reinterpret_cast<PxActor*>(addr);
	pxActorToNpActor[PxConcreteType::eRIGID_STATIC]			= reinterpret_cast<size_t>(pxToNpActor<NpRigidStatic>(n)) - addr;
	pxActorToNpActor[PxConcreteType::eRIGID_DYNAMIC]		= reinterpret_cast<size_t>(pxToNpActor<NpRigidDynamic>(n)) - addr;
#if PX_USE_PARTICLE_SYSTEM_API
	pxActorToNpActor[PxConcreteType::ePARTICLE_SYSTEM]		= reinterpret_cast<size_t>(pxToNpActor<NpParticleSystem>(n)) - addr;
	pxActorToNpActor[PxConcreteType::ePARTICLE_FLUID]		= reinterpret_cast<size_t>(pxToNpActor<NpParticleFluid>(n)) - addr;
#endif
	pxActorToNpActor[PxConcreteType::eARTICULATION_LINK]	= reinterpret_cast<size_t>(pxToNpActor<NpArticulationLink>(n)) - addr;
#if PX_USE_CLOTH_API
	pxActorToNpActor[PxConcreteType::eCLOTH]				= reinterpret_cast<size_t>(pxToNpActor<NpCloth>(n)) - addr;
#endif

	pxActorToScbActor[PxConcreteType::eRIGID_STATIC]		= PX_OFFSET_OF_RT(NpRigidStatic, getScbActorFast());
	pxActorToScbActor[PxConcreteType::eRIGID_DYNAMIC]		= PX_OFFSET_OF_RT(NpRigidDynamic, getScbActorFast());
#if PX_USE_PARTICLE_SYSTEM_API
	pxActorToScbActor[PxConcreteType::ePARTICLE_SYSTEM]		= PX_OFFSET_OF_RT(NpParticleSystem, getScbActor());
	pxActorToScbActor[PxConcreteType::ePARTICLE_FLUID]		= PX_OFFSET_OF_RT(NpParticleFluid, getScbActor());
#endif
	pxActorToScbActor[PxConcreteType::eARTICULATION_LINK]	= PX_OFFSET_OF_RT(NpArticulationLink, getScbActorFast());
#if PX_USE_CLOTH_API
	pxActorToScbActor[PxConcreteType::eCLOTH]				= PX_OFFSET_OF_RT(NpCloth, getScbCloth());
#endif
}

const NpActor::Offsets NpActor::sOffsets;


NpScene* NpActor::getOwnerScene(const PxActor& actor)
{
	const Scb::Actor& scbActor = getScbFromPxActor(actor);
	Scb::Scene* scbScene = scbActor.getScbScene();
	return scbScene? static_cast<NpScene*>(scbScene->getPxScene()) : NULL;
}

NpScene* NpActor::getAPIScene(const PxActor& actor)
{
	const Scb::Actor& scbActor = getScbFromPxActor(actor);
	Scb::Scene* scbScene = scbActor.getScbSceneForAPI();
	return scbScene? static_cast<NpScene*>(scbScene->getPxScene()) : NULL;
}

void NpActor::onActorRelease(PxActor* actor)
{
	NpFactory::getInstance().onActorRelease(actor);
}
