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


#ifndef PX_PHYSICS_NP_RIGIDACTOR_TEMPLATE
#define PX_PHYSICS_NP_RIGIDACTOR_TEMPLATE

#include "NpActorTemplate.h"
#include "NpShapeManager.h"
#include "NpConstraint.h"
#include "NpFactory.h"

// PX_SERIALIZATION
#include "foundation/PxErrors.h"
//~PX_SERIALIZATION

namespace physx
{

template<class APIClass>
class NpRigidActorTemplate : public NpActorTemplate<APIClass>
{
private:
	typedef NpActorTemplate<APIClass> ActorTemplateClass;

public:
// PX_SERIALIZATION
											NpRigidActorTemplate(PxBaseFlags baseFlags) : ActorTemplateClass(baseFlags), mShapeManager(PxEmpty), mIndex(0xFFFFFFFF)	{}
	virtual			void					requires(PxProcessPxBaseCallback& c);
	virtual			void					exportExtraData(PxSerializationContext& stream);
					void					importExtraData(PxDeserializationContext& context);
					void					resolveReferences(PxDeserializationContext& context);
//~PX_SERIALIZATION
	virtual									~NpRigidActorTemplate();

	//---------------------------------------------------------------------------------
	// PxActor implementation
	//---------------------------------------------------------------------------------
	virtual			void					release();

	// The rule is: If an API method is used somewhere in here, it has to be redeclared, else GCC whines
	virtual			PxActorType::Enum		getType() const = 0;

	virtual			PxBounds3				getWorldBounds(float inflation=1.01f) const;

	virtual			void					setActorFlag(PxActorFlag::Enum flag, bool value);
	virtual			void					setActorFlags(PxActorFlags inFlags);

	//---------------------------------------------------------------------------------
	// PxRigidActor implementation
	//---------------------------------------------------------------------------------

	// Shapes
	virtual			PxU32					getNbShapes() const;
	virtual			PxU32					getShapes(PxShape** buffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	// Constraint shaders
	virtual			PxU32					getNbConstraints() const;
	virtual			PxU32					getConstraints(PxConstraint** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	// Multiclient
	virtual			void					setClientBehaviorFlags(PxActorClientBehaviorFlags);

	// shared shapes
	virtual			void					attachShape(PxShape& s);
	virtual			void					detachShape(PxShape& s, bool wakeOnLostTouch);


	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
											NpRigidActorTemplate(PxType concreteType, PxBaseFlags baseFlags);

	// not optimal but the template alternative is hardly more readable and perf is not that critical here
	virtual			void					switchToNoSim() { PX_ASSERT(false); }
	virtual			void					switchFromNoSim() { PX_ASSERT(false); }

					void					releaseShape(NpShape& s);

	PX_FORCE_INLINE	NpShapeManager&			getShapeManager()		{ return mShapeManager; }
	PX_FORCE_INLINE	const NpShapeManager&	getShapeManager() const { return mShapeManager; }

					void					updateShaderComs();

	PX_FORCE_INLINE PxU32					getRigidActorArrayIndex()				const	{ return mIndex;	}
	PX_FORCE_INLINE void					setRigidActorArrayIndex(const PxU32& index)		{ mIndex = index;	}

					bool					resetFiltering(Scb::RigidObject& ro, PxShape*const* shapes, PxU32 shapeCount);

#if PX_ENABLE_DEBUG_VISUALIZATION
public:
					void					visualize(Cm::RenderOutput& out, NpScene* scene);
#endif
protected:
	PX_FORCE_INLINE void					setActorSimFlag(bool value);

					NpShapeManager			mShapeManager;
					PxU32					mIndex;    // index for the NpScene rigid actor array
};

// PX_SERIALIZATION

template<class APIClass>
void NpRigidActorTemplate<APIClass>::requires(PxProcessPxBaseCallback& c)
{
	// export shapes
	PxU32 nbShapes = mShapeManager.getNbShapes();
	for(PxU32 i=0;i<nbShapes;i++)
	{
		NpShape* np = mShapeManager.getShapes()[i];
		c.process(*np);
	}
}

template<class APIClass>
void NpRigidActorTemplate<APIClass>::exportExtraData(PxSerializationContext& stream)
{
	mShapeManager.exportExtraData(stream);
	ActorTemplateClass::exportExtraData(stream);
}

template<class APIClass>
void NpRigidActorTemplate<APIClass>::importExtraData(PxDeserializationContext& context)
{
	mShapeManager.importExtraData(context);
	ActorTemplateClass::importExtraData(context);
}

template<class APIClass>
void NpRigidActorTemplate<APIClass>::resolveReferences(PxDeserializationContext& context)
{
	const PxU32 nbShapes = mShapeManager.getNbShapes();
	NpShape** shapes = const_cast<NpShape**>(mShapeManager.getShapes());
	for(PxU32 j=0;j<nbShapes;j++)
	{						
		context.translatePxBase(shapes[j]);
		shapes[j]->onActorAttach(*this);
	}

	ActorTemplateClass::resolveReferences(context);
}

//~PX_SERIALIZATION

template<class APIClass>
NpRigidActorTemplate<APIClass>::NpRigidActorTemplate(PxType concreteType, PxBaseFlags baseFlags) 
: ActorTemplateClass(concreteType, baseFlags, NULL, NULL)
{
}

template<class APIClass>
NpRigidActorTemplate<APIClass>::~NpRigidActorTemplate()
{
	// TODO: no mechanism for notifying shaders of actor destruction yet
}


template<class APIClass>
void NpRigidActorTemplate<APIClass>::release()
{
	NpActor::releaseConstraints(*this);
	NpScene* scene = NpActor::getAPIScene(*this);
	if(mShapeManager.getPruningStructure())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxRigidActor::release: Actor is part of a pruning structure, pruning structure is now invalid!");
		mShapeManager.getPruningStructure()->invalidate(this);
	}

	mShapeManager.detachAll(scene);

// PX_AGGREGATE
	ActorTemplateClass::release();	// PT: added for PxAggregate
//~PX_AGGREGATE
}


template<class APIClass>
void NpRigidActorTemplate<APIClass>::releaseShape(NpShape& s)
{
	// invalidate the pruning structure if the actor bounds changed
	if (mShapeManager.getPruningStructure())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxRigidActor::releaseShape: Actor is part of a pruning structure, pruning structure is now invalid!");
		mShapeManager.getPruningStructure()->invalidate(this);
	}
	mShapeManager.detachShape(s, *this); 
}


template<class APIClass>
void NpRigidActorTemplate<APIClass>::attachShape(PxShape& shape)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	PX_CHECK_AND_RETURN(!static_cast<NpShape&>(shape).isExclusive() || shape.getActor()==NULL, "PxRigidActor::attachShape: shape must be shared or unowned");
	PX_SIMD_GUARD
	// invalidate the pruning structure if the actor bounds changed
	if (mShapeManager.getPruningStructure())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxRigidActor::attachShape: Actor is part of a pruning structure, pruning structure is now invalid!");
		mShapeManager.getPruningStructure()->invalidate(this);
	}

	mShapeManager.attachShape(static_cast<NpShape&>(shape), *this);
}

template<class APIClass>
void NpRigidActorTemplate<APIClass>::detachShape(PxShape& shape, bool wakeOnLostTouch)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));
	if (mShapeManager.getPruningStructure())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxRigidActor::detachShape: Actor is part of a pruning structure, pruning structure is now invalid!");
		mShapeManager.getPruningStructure()->invalidate(this);
	}

	if(!mShapeManager.detachShape(static_cast<NpShape&>(shape), *this, wakeOnLostTouch))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxRigidActor::detachShape: shape is not attached to this actor!");
	}
}


template<class APIClass>
PxU32 NpRigidActorTemplate<APIClass>::getNbShapes() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return mShapeManager.getNbShapes();
}


template<class APIClass>
PxU32 NpRigidActorTemplate<APIClass>::getShapes(PxShape** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return mShapeManager.getShapes(buffer, bufferSize, startIndex);
}


template<class APIClass>
PxU32 NpRigidActorTemplate<APIClass>::getNbConstraints() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return ActorTemplateClass::getNbConnectors(NpConnectorType::eConstraint);
}


template<class APIClass>
PxU32 NpRigidActorTemplate<APIClass>::getConstraints(PxConstraint** buffer, PxU32 bufferSize, PxU32 startIndex) const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return ActorTemplateClass::template getConnectors<PxConstraint>(NpConnectorType::eConstraint, buffer, bufferSize, startIndex);  // Some people will love me for this one... The syntax is to be standard compliant and
																														// picky gcc won't compile without it. It is needed if you call a templated member function
																														// of a templated class
}

template<class APIClass>
void NpRigidActorTemplate<APIClass>::setClientBehaviorFlags(PxActorClientBehaviorFlags b)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	Scb::Actor& scbActor = NpActor::getScbFromPxActor(*this);
	if (scbActor.getClientBehaviorFlags() == b)
		return;

	scbActor.setClientBehaviorFlags(b);
}

template<class APIClass>
PxBounds3 NpRigidActorTemplate<APIClass>::getWorldBounds(float inflation) const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	PX_SIMD_GUARD;

	const PxBounds3 bounds = mShapeManager.getWorldBounds(*this);
	PX_ASSERT(bounds.isValid());

	// PT: unfortunately we can't just scale the min/max vectors, we need to go through center/extents.
	const PxVec3 center = bounds.getCenter();
	const PxVec3 inflatedExtents = bounds.getExtents() * inflation;
	return PxBounds3::centerExtents(center, inflatedExtents);
}


template<class APIClass>
PX_FORCE_INLINE void NpRigidActorTemplate<APIClass>::setActorSimFlag(bool value)
{
	NpScene* scene = NpActor::getOwnerScene(*this);

	PxActorFlags oldFlags = ActorTemplateClass::getActorFlags();
	bool hadNoSimFlag = oldFlags.isSet(PxActorFlag::eDISABLE_SIMULATION);

	PX_CHECK_AND_RETURN((getType() != PxActorType::eARTICULATION_LINK) || (!value && !hadNoSimFlag), "PxActor::setActorFlag: PxActorFlag::eDISABLE_SIMULATION is only supported by PxRigidDynamic and PxRigidStatic objects.");

	if (hadNoSimFlag && (!value))
	{
		switchFromNoSim();
		ActorTemplateClass::setActorFlagsInternal(oldFlags & (~PxActorFlag::eDISABLE_SIMULATION));  // needs to be done before the code below to make sure the latest flags get picked up
		if (scene)
			NpActor::addConstraintsToScene();
	}
	else if ((!hadNoSimFlag) && value)
	{
		if (scene)
			NpActor::removeConstraintsFromScene();
		ActorTemplateClass::setActorFlagsInternal(oldFlags | PxActorFlag::eDISABLE_SIMULATION);
		switchToNoSim();
	}
}


template<class APIClass>
void NpRigidActorTemplate<APIClass>::setActorFlag(PxActorFlag::Enum flag, bool value)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	if (flag == PxActorFlag::eDISABLE_SIMULATION)
		setActorSimFlag(value);
	
	ActorTemplateClass::setActorFlagInternal(flag, value);
}


template<class APIClass>
void NpRigidActorTemplate<APIClass>::setActorFlags(PxActorFlags inFlags)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	bool noSim = inFlags.isSet(PxActorFlag::eDISABLE_SIMULATION);
	setActorSimFlag(noSim);
	
	ActorTemplateClass::setActorFlagsInternal(inFlags);
}


template<class APIClass>
void NpRigidActorTemplate<APIClass>::updateShaderComs()
{
	NpConnectorIterator iter = ActorTemplateClass::getConnectorIterator(NpConnectorType::eConstraint);
	while (PxBase* ser = iter.getNext())
	{
		NpConstraint* c = static_cast<NpConstraint*>(ser);
		c->comShift(this);
	}
}


PX_FORCE_INLINE static void fillResetFilteringShapeList(Scb::RigidObject& ro, Scb::Shape& scb, Scb::Shape** shapes, PxU32& shapeCount)
{
	// It is important to filter out shapes that have been added while the simulation was running because for those there is nothing to do.

	if (!ro.isBuffered(Scb::RigidObjectBuffer::BF_Shapes) || !ro.isAddedShape(scb))
	{
		shapes[shapeCount] = &scb;
		shapeCount++;
	}
}


template<class APIClass>
bool NpRigidActorTemplate<APIClass>::resetFiltering(Scb::RigidObject& ro, PxShape*const* shapes, PxU32 shapeCount)
{
	PX_CHECK_AND_RETURN_VAL(!(ro.getActorFlags() & PxActorFlag::eDISABLE_SIMULATION), "PxScene::resetFiltering(): Not allowed if PxActorFlag::eDISABLE_SIMULATION is set!", false);
	for(PxU32 i=0; i < shapeCount; i++)
	{
#if PX_CHECKED
		PxRigidActor* ra = shapes[i]->getActor();
		if (ra != this)
		{
			bool found = false;
			if (ra == NULL)
			{
				NpShape*const* sh = mShapeManager.getShapes();
				for(PxU32 j=0; j < mShapeManager.getNbShapes(); j++)
				{
					if (sh[j] == shapes[i])
					{
						found = true;
						break;
					}
				}
			}

			PX_CHECK_AND_RETURN_VAL(found, "PxScene::resetFiltering(): specified shape not in actor!", false);
		}
#endif
		PX_CHECK_AND_RETURN_VAL(static_cast<NpShape*>(shapes[i])->getScbShape().getFlags() & (PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eTRIGGER_SHAPE), "PxScene::resetFiltering(): specified shapes not of type eSIMULATION_SHAPE or eTRIGGER_SHAPE!", false);
	}

	PxU32 sCount;
	if (shapes)
		sCount = shapeCount;
	else
		sCount = mShapeManager.getNbShapes();
	
	PX_ALLOCA(scbShapes, Scb::Shape*, sCount);
	if (scbShapes)
	{
		if (shapes)  // the user specified the shapes
		{
			PxU32 sAccepted = 0;
			for(PxU32 i=0; i < sCount; i++)
			{
				Scb::Shape& scb = static_cast<NpShape*>(shapes[i])->getScbShape();
				fillResetFilteringShapeList(ro, scb, scbShapes, sAccepted);
			}
			sCount = sAccepted;
		}
		else  // the user just specified the actor and the shapes are taken from the actor
		{
			NpShape* const* sh = mShapeManager.getShapes();
			PxU32 sAccepted = 0;
			for(PxU32 i=0; i < sCount; i++)
			{
				Scb::Shape& scb = sh[i]->getScbShape();
				if (scb.getFlags() & (PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eTRIGGER_SHAPE))
				{
					fillResetFilteringShapeList(ro, scb, scbShapes, sAccepted);
				}
			}
			sCount = sAccepted;
		}

		if (sCount)
		{
			ro.resetFiltering(scbShapes, sCount);
		}
	}

	return true;
}


#if PX_ENABLE_DEBUG_VISUALIZATION
template<class APIClass>
void NpRigidActorTemplate<APIClass>::visualize(Cm::RenderOutput& out, NpScene* scene)
{
	mShapeManager.visualize(out, scene, *this);
}
#endif  // PX_ENABLE_DEBUG_VISUALIZATION

}

#endif
