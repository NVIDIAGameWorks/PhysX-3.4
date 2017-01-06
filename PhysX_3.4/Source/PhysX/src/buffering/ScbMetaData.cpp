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

#include "foundation/PxIO.h"
#include "ScbShape.h"
#include "ScbBody.h"
#include "ScbRigidStatic.h"
#include "ScbConstraint.h"
#include "ScbArticulation.h"
#include "ScbArticulationJoint.h"
#include "ScbAggregate.h"
#include "ScbCloth.h"
#include "ScbParticleSystem.h"

using namespace physx;

///////////////////////////////////////////////////////////////////////////////

void Scb::Base::getBinaryMetaData(PxOutputStream& stream)
{
	// 28 => 12 bytes
	PX_DEF_BIN_METADATA_TYPEDEF(stream,	ScbType::Enum, PxU32)

	PX_DEF_BIN_METADATA_CLASS(stream,	Scb::Base)

	PX_DEF_BIN_METADATA_ITEM(stream,	Scb::Base, Scb::Scene,	mScene,			PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,	Scb::Base, PxU32,		mControlState,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,	Scb::Base, PxU8*,		mStreamPtr,		PxMetaDataFlag::ePTR)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Shape::getBinaryMetaData(PxOutputStream& stream)
{
	// 176 => 160 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Shape)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Shape, Scb::Base)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Shape, ShapeCore,	mShape,	0)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Actor::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Actor)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Actor, Scb::Base)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::RigidObject::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::RigidObject)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::RigidObject, Scb::Actor)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Body::getBinaryMetaData(PxOutputStream& stream)
{
	// 240 => 224 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Body)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Body, Scb::RigidObject)

#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxU32,			mPaddingScbBody1,		PxMetaDataFlag::ePADDING)
#endif
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, Sc::BodyCore,	mBodyCore,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxTransform,		mBufferedBody2World,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxVec3,			mBufferedLinVelocity,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxVec3,			mBufferedAngVelocity,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxReal,			mBufferedWakeCounter,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxU32,			mBufferedIsSleeping,  	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Body, PxU32,			mBodyBufferFlags,		0)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::RigidStatic::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::RigidStatic)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::RigidStatic, Scb::RigidObject)

	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::RigidStatic, Sc::StaticCore,	mStatic,		0)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Articulation::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Articulation)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Articulation, Scb::Base)

	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Articulation, ArticulationCore,	mArticulation,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Articulation, PxReal,				mBufferedWakeCounter,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Articulation, PxU8,				mBufferedIsSleeping,	0)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::ArticulationJoint::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::ArticulationJoint)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::ArticulationJoint, Scb::Base)

	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::ArticulationJoint, ArticulationJointCore,	mJoint,			0)
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Constraint::getBinaryMetaData(PxOutputStream& stream)
{
	// 120 => 108 bytes
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Constraint)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Constraint, Scb::Base)

	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Constraint, ConstraintCore,	mConstraint,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Constraint, PxVec3,			mBufferedForce,				0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Constraint, PxVec3,			mBufferedTorque,			0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Constraint, PxConstraintFlags,	mBrokenFlag,				0)
#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Constraint, PxU16,				mPaddingFromBrokenFlags,	PxMetaDataFlag::ePADDING)
#endif
}

///////////////////////////////////////////////////////////////////////////////

void Scb::Aggregate::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Aggregate)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Aggregate, Scb::Base)

	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Aggregate, PxAggregate,mPxAggregate,	PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Aggregate, PxU32,		mAggregateID,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Aggregate, PxU32,		mMaxNbActors,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Aggregate, bool,		mSelfCollide,	0)

#ifdef EXPLICIT_PADDING_METADATA
	PX_DEF_BIN_METADATA_ITEMS_AUTO(stream,	Scb::Aggregate, bool,		mPaddingFromBool,	PxMetaDataFlag::ePADDING)
#endif
}

///////////////////////////////////////////////////////////////////////////////

#if PX_USE_CLOTH_API
void Scb::Cloth::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::Cloth)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::Cloth, Scb::Actor)

	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::Cloth, Sc::ClothCore, mCloth, 0)	
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if PX_USE_PARTICLE_SYSTEM_API
void Scb::ParticleSystem::getBinaryMetaData(PxOutputStream& stream)
{
	PX_DEF_BIN_METADATA_CLASS(stream,	ForceUpdates)
	PX_DEF_BIN_METADATA_ITEM(stream,	ForceUpdates, BitMap, map,        PxMetaDataFlag::ePTR)	
	PX_DEF_BIN_METADATA_ITEM(stream,	ForceUpdates, PxVec3, values,     PxMetaDataFlag::ePTR)		
	PX_DEF_BIN_METADATA_ITEM(stream,	ForceUpdates, bool,   hasUpdates, 0)	
		
	PX_DEF_BIN_METADATA_CLASS(stream,		Scb::ParticleSystem)
	PX_DEF_BIN_METADATA_BASE_CLASS(stream,	Scb::ParticleSystem, Scb::Actor)
	
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::ParticleSystem, Sc::ParticleSystemCore,  mParticleSystem,        0)	
		
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::ParticleSystem, NpParticleFluidReadData, mReadParticleFluidData, PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::ParticleSystem, ForceUpdates,            mForceUpdatesAcc,       0)
	PX_DEF_BIN_METADATA_ITEM(stream,		Scb::ParticleSystem, ForceUpdates,            mForceUpdatesVel,       0)
}
#endif

///////////////////////////////////////////////////////////////////////////////
