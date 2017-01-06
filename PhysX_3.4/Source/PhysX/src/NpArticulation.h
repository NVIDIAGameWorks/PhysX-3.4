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


#ifndef PX_PHYSICS_NP_ARTICULATION
#define PX_PHYSICS_NP_ARTICULATION

#include "PxArticulation.h"
#include "CmPhysXCommon.h"

#if PX_ENABLE_DEBUG_VISUALIZATION
#include "CmRenderOutput.h"
#endif

#include "ScbArticulation.h"
#include "NpArticulationLink.h"

namespace physx
{

class NpArticulationLink;
class NpScene;
class NpAggregate;
class PxAggregate;


class NpArticulation : public PxArticulation, public Ps::UserAllocated
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
	virtual										~NpArticulation();

// PX_SERIALIZATION
												NpArticulation(PxBaseFlags baseFlags) :	PxArticulation(baseFlags), mArticulation(PxEmpty), mArticulationLinks(PxEmpty)	{}
	virtual			void						exportExtraData(PxSerializationContext& stream);
					void						importExtraData(PxDeserializationContext& context);
					void						resolveReferences(PxDeserializationContext& context);
	virtual	        void						requires(PxProcessPxBaseCallback& c);
	static			NpArticulation*				createObject(PxU8*& address, PxDeserializationContext& context);
	static			void						getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	//---------------------------------------------------------------------------------
	// PxArticulation implementation
	//---------------------------------------------------------------------------------
	virtual			void						release();

	virtual			PxScene*					getScene() const;

	virtual			PxU32						getInternalDriveIterations() const;
	virtual			void						setInternalDriveIterations(PxU32 iterations);

	virtual			PxU32						getExternalDriveIterations() const;
	virtual			void						setExternalDriveIterations(PxU32 iterations);

	virtual			PxU32						getMaxProjectionIterations() const;
	virtual			void						setMaxProjectionIterations(PxU32 iterations);

	virtual			PxReal						getSeparationTolerance() const;
	virtual			void						setSeparationTolerance(PxReal tolerance);

	virtual			void						setSleepThreshold(PxReal threshold);
	virtual			PxReal						getSleepThreshold() const;

	virtual			void						setStabilizationThreshold(PxReal threshold);
	virtual			PxReal						getStabilizationThreshold() const;

	virtual			void						setWakeCounter(PxReal wakeCounterValue);
	virtual			PxReal						getWakeCounter() const;

	virtual			void						setSolverIterationCounts(PxU32 positionIters, PxU32 velocityIters);
	virtual			void						getSolverIterationCounts(PxU32 & positionIters, PxU32 & velocityIters) const;

	virtual			bool						isSleeping() const;
	virtual			void						wakeUp();
	virtual			void						putToSleep();

	virtual			PxArticulationLink*			createLink(PxArticulationLink* parent, const PxTransform& pose);

	virtual			PxU32						getNbLinks() const;
	virtual			PxU32						getLinks(PxArticulationLink** userBuffer, PxU32 bufferSize, PxU32 startIndex) const;

	virtual			PxBounds3					getWorldBounds(float inflation=1.01f) const;

	virtual			PxAggregate*				getAggregate() const;

	// Debug name
	virtual			void						setName(const char*);
	virtual			const char*					getName() const;

	virtual		PxArticulationDriveCache* 
								createDriveCache(PxReal compliance, PxU32 driveIterations) const;

	virtual		void			updateDriveCache(PxArticulationDriveCache& cache, PxReal compliance, PxU32 driveIterations) const;

	virtual		void			releaseDriveCache(PxArticulationDriveCache&) const;

	virtual		void			applyImpulse(PxArticulationLink*,
											 const PxArticulationDriveCache& driveCache,
											 const PxVec3& force,
											 const PxVec3& torque);

	virtual		void			computeImpulseResponse(PxArticulationLink*,
													   PxVec3& linearResponse, 
													   PxVec3& angularResponse,
													   const PxArticulationDriveCache& driveCache,
													   const PxVec3& force,
													   const PxVec3& torque) const;




	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
												NpArticulation();

	PX_INLINE		void						addToLinkList(NpArticulationLink& link)				{ mArticulationLinks.pushBack(&link); }
					bool						removeLinkFromList(NpArticulationLink& link)		{ PX_ASSERT(mArticulationLinks.find(&link) != mArticulationLinks.end()); return mArticulationLinks.findAndReplaceWithLast(&link); }
	PX_FORCE_INLINE	NpArticulationLink* const*	getLinks()											{ return &mArticulationLinks.front(); }

					NpArticulationLink*			getRoot();

	PX_FORCE_INLINE	const Scb::Articulation&	getArticulation()							const	{ return mArticulation; }
	PX_FORCE_INLINE	Scb::Articulation&			getArticulation()									{ return mArticulation; }

					NpScene*					getAPIScene() const;
					NpScene*					getOwnerScene() const;		// the scene the user thinks the actor is in, or from which the actor is pending removal

	PX_FORCE_INLINE	void						setAggregate(NpAggregate* a) { mAggregate = a; }

					void						wakeUpInternal(bool forceWakeUp, bool autowake);

	PX_FORCE_INLINE	Scb::Articulation&			getScbArticulation()			{ return mArticulation; }
	PX_FORCE_INLINE	const Scb::Articulation&	getScbArticulation() const		{ return mArticulation; }


#if PX_ENABLE_DEBUG_VISUALIZATION
public:
					void						visualize(Cm::RenderOutput& out, NpScene* scene);
#endif

private:
					Scb::Articulation 			mArticulation;
					NpArticulationLinkArray		mArticulationLinks;
					NpAggregate*				mAggregate;
					const char*					mName;
};


}

#endif
