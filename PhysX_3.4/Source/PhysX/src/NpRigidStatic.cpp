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


#include "NpRigidStatic.h"
#include "NpPhysics.h"
#include "ScbNpDeps.h"
#include "NpScene.h"
#include "NpRigidActorTemplateInternal.h"

using namespace physx;

NpRigidStatic::NpRigidStatic(const PxTransform& pose)
: NpRigidStaticT(PxConcreteType::eRIGID_STATIC, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
, mRigidStatic(pose)
{
}

NpRigidStatic::~NpRigidStatic()
{
}

// PX_SERIALIZATION
void NpRigidStatic::requires(PxProcessPxBaseCallback& c)
{
	NpRigidStaticT::requires(c);	
}

NpRigidStatic* NpRigidStatic::createObject(PxU8*& address, PxDeserializationContext& context)
{
	NpRigidStatic* obj = new (address) NpRigidStatic(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(NpRigidStatic);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}
//~PX_SERIALIZATION

void NpRigidStatic::release()
{
	releaseActorT(this, mRigidStatic);
}

void NpRigidStatic::setGlobalPose(const PxTransform& pose, bool /*wake*/)
{
	PX_CHECK_AND_RETURN(pose.isSane(), "PxRigidStatic::setGlobalPose: pose is not valid.");

	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	NpScene* npScene = NpActor::getAPIScene(*this);
#if PX_CHECKED
	if(npScene)
		npScene->checkPositionSanity(*this, pose, "PxRigidStatic::setGlobalPose");
#endif

	mRigidStatic.setActor2World(pose.getNormalized());

	if(npScene)
	{
		mShapeManager.markAllSceneQueryForUpdate(npScene->getSceneQueryManagerFast());
		npScene->getSceneQueryManagerFast().get(Sq::PruningIndex::eSTATIC).invalidateTimestamp();
	}

#if PX_SUPPORT_PVD
	// have to do this here since this call gets not forwarded to Scb::RigidStatic
	Scb::Scene* scbScene = NpActor::getScbFromPxActor(*this).getScbSceneForAPI();
	if(scbScene)
		scbScene->getScenePvdClient().updatePvdProperties(&mRigidStatic);
#endif

	// invalidate the pruning structure if the actor bounds changed
	if (mShapeManager.getPruningStructure())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "PxRigidStatic::setGlobalPose: Actor is part of a pruning structure, pruning structure is now invalid!");
		mShapeManager.getPruningStructure()->invalidate(this);
	}

	updateShaderComs();
}

PxTransform NpRigidStatic::getGlobalPose() const
{
	NP_READ_CHECK(NpActor::getOwnerScene(*this));
	return mRigidStatic.getActor2World();
}

PxShape* NpRigidStatic::createShape(const PxGeometry& geometry, PxMaterial*const* materials, PxU16 materialCount, PxShapeFlags shapeFlags)
{
	NP_WRITE_CHECK(NpActor::getOwnerScene(*this));

	PX_CHECK_AND_RETURN_NULL(materials, "createShape: material pointer is NULL");
	PX_CHECK_AND_RETURN_NULL(materialCount>0, "createShape: material count is zero");

	NpShape* shape = static_cast<NpShape*>(NpPhysics::getInstance().createShape(geometry, materials, materialCount, true, shapeFlags));

	if ( shape != NULL )
	{
		mShapeManager.attachShape(*shape, *this);
		shape->releaseInternal();
	}
	return shape;
}

PxU32 physx::NpRigidStaticGetShapes(Scb::RigidStatic& rigid, void* const *&shapes)
{
	NpRigidStatic* a = static_cast<NpRigidStatic*>(rigid.getScRigidCore().getPxActor());
	NpShapeManager& sm = a->getShapeManager();
	shapes = reinterpret_cast<void *const *>(sm.getShapes());
	return sm.getNbShapes();
}

void NpRigidStatic::switchToNoSim()
{
	getScbRigidStaticFast().switchToNoSim(false);
}

void NpRigidStatic::switchFromNoSim()
{
	getScbRigidStaticFast().switchFromNoSim(false);
}

#if PX_CHECKED
bool NpRigidStatic::checkConstraintValidity() const
{
	// Perhaps NpConnectorConstIterator would be worth it...
	NpConnectorIterator iter = (const_cast<NpRigidStatic*>(this))->getConnectorIterator(NpConnectorType::eConstraint); 
	while (PxBase* ser = iter.getNext())
	{
		NpConstraint* c = static_cast<NpConstraint*>(ser);
		if(!c->NpConstraint::isValid())
			return false;
	}
	return true;
}
#endif

#if PX_ENABLE_DEBUG_VISUALIZATION
void NpRigidStatic::visualize(Cm::RenderOutput& out, NpScene* scene)
{
	NpRigidStaticT::visualize(out, scene);

	if (getScbRigidStaticFast().getActorFlags() & PxActorFlag::eVISUALIZATION)
	{
		Scb::Scene& scbScene = scene->getScene();
		PxReal scale = scbScene.getVisualizationParameter(PxVisualizationParameter::eSCALE);

		//visualize actor frames
		PxReal actorAxes = scale * scbScene.getVisualizationParameter(PxVisualizationParameter::eACTOR_AXES);
		if (actorAxes != 0)
			out << getGlobalPose() << Cm::DebugBasis(PxVec3(actorAxes));
	}
}
#endif

