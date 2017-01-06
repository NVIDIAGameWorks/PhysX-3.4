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


#ifndef PX_PHYSICS_NP_ACTOR
#define PX_PHYSICS_NP_ACTOR

#include "NpConnector.h"
#include "ScbActor.h" // DM: without this inclusion PVD-diabled android build fails, lacking Scb::Actor definition

namespace physx
{
	class NpShapeManager;
	class NpAggregate;
	class NpScene;
	class NpShape;

class NpActor
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	// We sometimes pass in the PxActor here even though it's always a base class 
	// of the objects which inherit from this class too. But passing 
	// context to functions which need it allows this to be purely a mixin containing shared 
	// utility code rather than an abstract base class.

public:
// PX_SERIALIZATION
												NpActor(const PxEMPTY)	{}					
					
					void						exportExtraData(PxSerializationContext& stream);	
					void						importExtraData(PxDeserializationContext& context);
					void						resolveReferences(PxDeserializationContext& context);
	static			void						getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
												NpActor(const char* name);					

					void						releaseConstraints(PxRigidActor& owner);
					void						release(PxActor& owner);

					NpAggregate*				getNpAggregate(PxU32& index)	const;
					void						setAggregate(NpAggregate* np, PxActor& owner);
					PxAggregate*				getAggregate()					const;

					void						removeConstraintsFromScene();
					PX_FORCE_INLINE void		addConstraintsToScene()		// inline the fast path for addActors()
					{
						if(mConnectorArray)
							addConstraintsToSceneInternal();
					}

	PX_FORCE_INLINE	NpConnectorArray**			getConnectorArrayAddress()		{ return &mConnectorArray;}
					PxU32						findConnector(NpConnectorType::Enum type, PxBase* object)	const;
					void						addConnector(NpConnectorType::Enum type, PxBase* object, const char* errMsg);
					void						removeConnector(PxActor& owner, NpConnectorType::Enum type, PxBase* object, const char* errorMsg);
					PxU32						getNbConnectors(NpConnectorType::Enum type)	const;

					static NpShapeManager*			getShapeManager(PxRigidActor& actor);			// bit misplaced here, but we don't want a separate subclass just for this
					static const NpShapeManager*	getShapeManager(const PxRigidActor& actor);			// bit misplaced here, but we don't want a separate subclass just for this

					static void					getGlobalPose(PxTransform& globalPose, const NpShape& shape, const PxRigidActor& actor);
					static void					getGlobalPose(PxTransform& globalPose, const Scb::Shape& scbShape, const Scb::Actor& scbActor);

					static NpActor&				getFromPxActor(PxActor& actor)			{ 	return *Ps::pointerOffset<NpActor*>(&actor, ptrdiff_t(sOffsets.pxActorToNpActor[actor.getConcreteType()])); }
					static const NpActor&		getFromPxActor(const PxActor& actor)	{	return *Ps::pointerOffset<const NpActor*>(&actor, ptrdiff_t(sOffsets.pxActorToNpActor[actor.getConcreteType()])); }

					static Scb::Actor&			getScbFromPxActor(PxActor& actor)		{	return *Ps::pointerOffset<Scb::Actor*>(&actor, ptrdiff_t(sOffsets.pxActorToScbActor[actor.getConcreteType()])); }
					static const Scb::Actor&	getScbFromPxActor(const PxActor& actor)	{ 	return *Ps::pointerOffset<const Scb::Actor*>(&actor, ptrdiff_t(sOffsets.pxActorToScbActor[actor.getConcreteType()])); }
					
					static NpScene*				getAPIScene(const PxActor& actor);			// the scene the user thinks the actor is in
					static NpScene*				getOwnerScene(const PxActor& actor);		// the scene the user thinks the actor is in, or from which the actor is pending removal

	PX_FORCE_INLINE	NpConnectorIterator			getConnectorIterator(NpConnectorType::Enum type)
												{
													if (mConnectorArray)
														return NpConnectorIterator(&mConnectorArray->front(), mConnectorArray->size(), type);
													else
														return NpConnectorIterator(NULL, 0, type);
												}

		// a couple of methods that sever include dependencies in NpActor.h

					static void					onActorRelease(PxActor* actor);



template<typename T>
PxU32 getConnectors(NpConnectorType::Enum type, T** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const
{
	PxU32 nbConnectors = 0;

	if(mConnectorArray)
	{
		for(PxU32 i=0; i < mConnectorArray->size(); i++)
		{
			NpConnector& c = (*mConnectorArray)[i];
			if (c.mType == type && nbConnectors < bufferSize && i>=startIndex)
			{
				userBuffer[nbConnectors] = static_cast<T*>(c.mObject);
				nbConnectors++;
			}
		}
	}

	return nbConnectors;
}

protected:
									~NpActor();
				const char*			mName;
				// Lazy-create array for connector objects like constraints, observers, ... 
				// Most actors have no such objects, so we bias this class accordingly:
				NpConnectorArray*	mConnectorArray;

private:
					void						addConstraintsToSceneInternal();
					void						removeConnector(PxActor& owner, PxU32 index);
	struct Offsets
	{
		size_t pxActorToNpActor[PxConcreteType::ePHYSX_CORE_COUNT];
		size_t pxActorToScbActor[PxConcreteType::ePHYSX_CORE_COUNT];
		Offsets();
	};
public:
	static const Offsets sOffsets;
};



}

#endif
