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

#include "NpPhysics.h"

// PX_SERIALIZATION
#include "foundation/PxProfiler.h"
#include "foundation/PxIO.h"
#include "foundation/PxErrorCallback.h"
#include "PxPhysicsVersion.h"
#include "CmCollection.h"
#include "CmUtils.h"
#include "NpClothFabric.h"
#include "NpCloth.h"
#include "NpParticleSystem.h"
#include "NpParticleFluid.h"
#include "NpRigidStatic.h"
#include "NpRigidDynamic.h"
#include "NpArticulation.h"
#include "NpArticulationLink.h"
#include "NpArticulationJoint.h"
#include "NpMaterial.h"
#include "GuHeightFieldData.h"
#include "GuHeightField.h"
#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "PsIntrinsics.h"
#include "PxTolerancesScale.h"
#include "PxvGlobals.h"		// dynamic registration of HFs & articulations in LL
#include "GuOverlapTests.h" // dynamic registration of HFs in Gu
#include "PxDeletionListener.h"
#include "PxPhysicsSerialization.h"
#include "PsString.h"
#include "PvdPhysicsClient.h"
#include "SqPruningStructure.h"

//~PX_SERIALIZATION

#if PX_USE_PARTICLE_SYSTEM_API
#include "PtContext.h"
#endif

#include "NpFactory.h"


#if PX_USE_CLOTH_API
#include "NpCloth.h"
#endif

#if PX_SWITCH
#include "switch/NpMiddlewareInfo.h"
#endif

using namespace physx;
using namespace Cm;

bool		NpPhysics::apiReentryLock	= false;
NpPhysics*	NpPhysics::mInstance		= NULL;
PxU32		NpPhysics::mRefCount		= 0;

#if PX_CHECKED
bool		NpPhysics::mHeightFieldsRegistered = false;	//just for error checking
#endif



NpPhysics::NpPhysics(const PxTolerancesScale& scale, const PxvOffsetTable& pxvOffsetTable, bool trackOutstandingAllocations,
	                 physx::pvdsdk::PsPvd* pvd) :
	mSceneArray(PX_DEBUG_EXP("physicsSceneArray"))
	, mSceneRunning(NULL)
	, mPhysics(scale, pxvOffsetTable)
	, mDeletionListenersExist(false)
#if PX_SUPPORT_GPU_PHYSX
	, mNbRegisteredGpuClients(0)
	, mPhysicsGpu(*this)	
#endif	
{

	PX_UNUSED(trackOutstandingAllocations);

	//mMasterMaterialTable.reserve(10);
		
#if PX_SUPPORT_PVD	
	mPvd = pvd;
	if(pvd)
	{		
	    mPvdPhysicsClient = PX_NEW(Vd::PvdPhysicsClient)(mPvd);	
	    shdfnd::getFoundation().registerErrorCallback(*mPvdPhysicsClient);
		shdfnd::getFoundation().registerAllocationListener(*mPvd);	
	}
	else
	{		
		mPvdPhysicsClient = NULL;
	}
#else
	PX_UNUSED(pvd);
#endif
}

NpPhysics::~NpPhysics()
{
	// Release all scenes in case the user didn't do it
	PxU32 nbScenes = mSceneArray.size();
	NpScene** scenes = mSceneArray.begin();
	for(PxU32 i=0;i<nbScenes;i++)
		PX_DELETE_AND_RESET(scenes[i]);
	mSceneArray.clear();

	//PxU32 matCount = mMasterMaterialTable.size();
	//while (mMasterMaterialTable.size() > 0)
	//{
	//	// It's done this way since the material destructor removes the material from the table and adjusts indices

	//	PX_ASSERT(mMasterMaterialTable[0]->getRefCount() == 1);
	//	mMasterMaterialTable[0]->decRefCount();
	//}
	//mMasterMaterialTable.clear();

	mMasterMaterialManager.releaseMaterials();

#if PX_SUPPORT_PVD	
	if(mPvd)
	{	
		mPvdPhysicsClient->destroyPvdInstance(this);
		mPvd->removeClient(mPvdPhysicsClient);
		shdfnd::getFoundation().deregisterErrorCallback(*mPvdPhysicsClient);
		PX_DELETE_AND_RESET(mPvdPhysicsClient);	
		shdfnd::getFoundation().deregisterAllocationListener(*mPvd);
	}	
#endif

	const DeletionListenerMap::Entry* delListenerEntries = mDeletionListenerMap.getEntries();
	const PxU32 delListenerEntryCount = mDeletionListenerMap.size();
	for(PxU32 i=0; i < delListenerEntryCount; i++)
	{
		PX_DELETE(delListenerEntries[i].second);
	}
	mDeletionListenerMap.clear();
}

void NpPhysics::initOffsetTables(PxvOffsetTable& pxvOffsetTable)
{
	// init offset tables for Pxs/Sc/Scb/Px conversions
	{
		Sc::OffsetTable& offsetTable =  Sc::gOffsetTable;
		offsetTable.scRigidStatic2PxActor				= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpRigidStatic*>(0)->getScbRigidStaticFast())) - static_cast<ptrdiff_t>(Scb::RigidStatic::getScOffset());
		offsetTable.scRigidDynamic2PxActor				= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpRigidDynamic*>(0)->getScbBodyFast()))		- static_cast<ptrdiff_t>(Scb::Body::getScOffset());
		offsetTable.scArticulationLink2PxActor			= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpArticulationLink*>(0)->getScbBodyFast()))	- static_cast<ptrdiff_t>(Scb::Body::getScOffset());
		offsetTable.scArticulation2Px					= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpArticulation*>(0)->getScbArticulation()))	- static_cast<ptrdiff_t>(Scb::Articulation::getScOffset());
		offsetTable.scConstraint2Px						= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpConstraint*>(0)->getScbConstraint()))		- static_cast<ptrdiff_t>(Scb::Constraint::getScOffset());
		offsetTable.scShape2Px							= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpShape*>(0)->getScbShape()))					- static_cast<ptrdiff_t>(Scb::Shape::getScOffset());
#if PX_USE_PARTICLE_SYSTEM_API
		offsetTable.scParticleSystem2PxParticleFluid	= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpParticleFluid*>(0)->getScbParticleSystem())) - static_cast<ptrdiff_t>(Scb::ParticleSystem::getScOffset());	
		offsetTable.scParticleSystem2Px					= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpParticleSystem*>(0)->getScbParticleSystem())) - static_cast<ptrdiff_t>(Scb::ParticleSystem::getScOffset());	
#endif
#if PX_USE_CLOTH_API
		offsetTable.scCloth2Px							= -reinterpret_cast<ptrdiff_t>(&(reinterpret_cast<NpCloth*>(0)->getScbCloth()))					- static_cast<ptrdiff_t>(Scb::Cloth::getScOffset());
#endif
		for(PxU32 i=0;i<PxActorType::eACTOR_COUNT;i++)
			offsetTable.scCore2PxActor[i] = 0;
		offsetTable.scCore2PxActor[PxActorType::eRIGID_STATIC] = offsetTable.scRigidStatic2PxActor;
		offsetTable.scCore2PxActor[PxActorType::eRIGID_DYNAMIC] = offsetTable.scRigidDynamic2PxActor;
		offsetTable.scCore2PxActor[PxActorType::eARTICULATION_LINK] = offsetTable.scArticulationLink2PxActor;
	}
	{
		Sq::OffsetTable& offsetTable =  Sq::gOffsetTable;
		// init pxActorToScbActor
		for(PxU32 i=0;i<PxConcreteType::ePHYSX_CORE_COUNT;i++)
			offsetTable.pxActorToScbActor[i] = 0;
		ptrdiff_t addr = 0x100;	// casting the null ptr takes a special-case code path, which we don't want
		PxActor* n = reinterpret_cast<PxActor*>(addr);
		offsetTable.pxActorToScbActor[PxConcreteType::eRIGID_STATIC]		= reinterpret_cast<ptrdiff_t>(&static_cast<NpRigidStatic*>(n)->getScbActorFast()) - addr;
		offsetTable.pxActorToScbActor[PxConcreteType::eRIGID_DYNAMIC]		= reinterpret_cast<ptrdiff_t>(&static_cast<NpRigidDynamic*>(n)->getScbActorFast()) - addr;
#if PX_USE_PARTICLE_SYSTEM_API
		offsetTable.pxActorToScbActor[PxConcreteType::ePARTICLE_SYSTEM]		= reinterpret_cast<ptrdiff_t>(&static_cast<NpParticleSystem*>(n)->getScbActor()) - addr;
		offsetTable.pxActorToScbActor[PxConcreteType::ePARTICLE_FLUID]		= reinterpret_cast<ptrdiff_t>(&static_cast<NpParticleFluid*>(n)->getScbActor()) - addr;
#endif
		offsetTable.pxActorToScbActor[PxConcreteType::eARTICULATION_LINK]	= reinterpret_cast<ptrdiff_t>(&static_cast<NpArticulationLink*>(n)->getScbActorFast()) - addr;
#if PX_USE_CLOTH_API
		offsetTable.pxActorToScbActor[PxConcreteType::eCLOTH]				= reinterpret_cast<ptrdiff_t>(&static_cast<NpCloth*>(n)->getScbCloth()) - addr;
#endif
		// init scb2sc
		for(PxU32 i=0;i<ScbType::eTYPE_COUNT;i++)
			offsetTable.scbToSc[i] = 0;
		ptrdiff_t staticOffset = static_cast<ptrdiff_t>(Scb::RigidStatic::getScOffset());
		ptrdiff_t bodyOffset = static_cast<ptrdiff_t>(Scb::Body::getScOffset());
		offsetTable.scbToSc[ScbType::eRIGID_STATIC] = staticOffset;
		offsetTable.scbToSc[ScbType::eBODY] = bodyOffset;
		offsetTable.scbToSc[ScbType::eBODY_FROM_ARTICULATION_LINK] = bodyOffset;
#if PX_USE_PARTICLE_SYSTEM_API
		offsetTable.scbToSc[ScbType::ePARTICLE_SYSTEM] = static_cast<ptrdiff_t>(Scb::ParticleSystem::getScOffset());
#endif
#if PX_USE_CLOTH_API
		offsetTable.scbToSc[ScbType::eCLOTH] = static_cast<ptrdiff_t>(Scb::Cloth::getScOffset());
#endif
	}
	{
		Sc::OffsetTable& scOffsetTable = Sc::gOffsetTable;
		pxvOffsetTable.pxsShapeCore2PxShape			= scOffsetTable.scShape2Px				- reinterpret_cast<ptrdiff_t>(&static_cast<Sc::ShapeCore*>(0)->getCore());
		pxvOffsetTable.pxsRigidCore2PxRigidBody		= scOffsetTable.scRigidDynamic2PxActor	- reinterpret_cast<ptrdiff_t>(&static_cast<Sc::BodyCore*>(0)->getCore());
		pxvOffsetTable.pxsRigidCore2PxRigidStatic	= scOffsetTable.scRigidStatic2PxActor	- reinterpret_cast<ptrdiff_t>(&static_cast<Sc::StaticCore*>(0)->getCore());
	}
}

NpPhysics* NpPhysics::createInstance(PxU32 version, PxFoundation& foundation, const PxTolerancesScale& scale, bool trackOutstandingAllocations,
	                                 physx::pvdsdk::PsPvd* pvd)
{
	PX_UNUSED(foundation);

#if PX_SWITCH
	NpSetMiddlewareInfo();  // register middleware info such that PhysX usage can be tracked
#endif
	
	if (version!=PX_PHYSICS_VERSION) 
	{
		char buffer[256];
		Ps::snprintf(buffer, 256, "Wrong version: PhysX version is 0x%08x, tried to create 0x%08x", PX_PHYSICS_VERSION, version);
		foundation.getErrorCallback().reportError(PxErrorCode::eINVALID_PARAMETER, buffer, __FILE__, __LINE__);
		return NULL;
	}

	if (!scale.isValid())
	{
		foundation.getErrorCallback().reportError(PxErrorCode::eINVALID_PARAMETER, "Scale invalid.\n", __FILE__, __LINE__);
		return NULL; 
	}

	if(0 == mRefCount)
	{
		PX_ASSERT(static_cast<Ps::Foundation*>(&foundation) == &Ps::Foundation::getInstance());

		Ps::Foundation::incRefCount();

		// init offset tables for Pxs/Sc/Scb/Px conversions
		PxvOffsetTable pxvOffsetTable;
		initOffsetTables(pxvOffsetTable);

		//SerialFactory::createInstance();
		mInstance = PX_NEW (NpPhysics)(scale, pxvOffsetTable, trackOutstandingAllocations, pvd);
		NpFactory::createInstance();
		
#if PX_SUPPORT_PVD			
	    if(pvd)
		{			
			NpFactory::getInstance().setNpFactoryListener( *mInstance->mPvdPhysicsClient );					
			pvd->addClient(mInstance->mPvdPhysicsClient);
		}
#endif

		NpFactory::getInstance().addFactoryListener(mInstance->mDeletionMeshListener);
	}
	++mRefCount;

	return mInstance;
}

PxU32 NpPhysics::releaseInstance()
{
	PX_ASSERT(mRefCount > 0);
	if (--mRefCount) 
		return mRefCount;

#if PX_SUPPORT_PVD	
	if(mInstance->mPvd)
	{	
		NpFactory::getInstance().removeFactoryListener( *mInstance->mPvdPhysicsClient );		
	}
#endif
	
	NpFactory::destroyInstance();

	PX_ASSERT(mInstance);
	PX_DELETE_AND_RESET(mInstance);

	Ps::Foundation::decRefCount();

	return mRefCount;
}

void NpPhysics::release()
{
	NpPhysics::releaseInstance();
}

PxScene* NpPhysics::createScene(const PxSceneDesc& desc)
{
	PX_CHECK_AND_RETURN_NULL(desc.isValid(), "Physics::createScene: desc.isValid() is false!");

	const PxTolerancesScale& scale = mPhysics.getTolerancesScale();
	const PxTolerancesScale& descScale = desc.getTolerancesScale();
	PX_UNUSED(scale);
	PX_UNUSED(descScale);
	PX_CHECK_AND_RETURN_NULL((descScale.length == scale.length) && (descScale.mass == scale.mass) && (descScale.speed == scale.speed), "Physics::createScene: PxTolerancesScale must be the same as used for creation of PxPhysics!");

	Ps::Mutex::ScopedLock lock(mSceneAndMaterialMutex);  // done here because scene constructor accesses profiling manager of the SDK

	NpScene* npScene = PX_NEW (NpScene)(desc);
	if(!npScene)
	{
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Unable to create scene.");
		return NULL;
	}
	if(!npScene->getTaskManager())
	{
		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Unable to create scene. Task manager creation failed.");
		return NULL;
	}

	npScene->loadFromDesc(desc);

#if PX_SUPPORT_PVD
	if(mPvd)
	{
		npScene->mScene.getScenePvdClient().setPsPvd(mPvd);		
		mPvd->addClient(&npScene->mScene.getScenePvdClient());
	}
#endif

	if (!sendMaterialTable(*npScene) || !npScene->getScene().isValid())
	{
		PX_DELETE(npScene);
		Ps::getFoundation().error(PxErrorCode::eOUT_OF_MEMORY, __FILE__, __LINE__, "Unable to create scene.");
		return NULL;
	}

	mSceneArray.pushBack(npScene);
	return npScene;
}


void NpPhysics::releaseSceneInternal(PxScene& scene)
{
	NpScene* pScene =  static_cast<NpScene*>(&scene);

	Ps::Mutex::ScopedLock lock(mSceneAndMaterialMutex);
	for(PxU32 i=0;i<mSceneArray.size();i++)
	{
		if(mSceneArray[i]==pScene)
		{
			mSceneArray.replaceWithLast(i);
			PX_DELETE_AND_RESET(pScene);
			return;
		}
	}
}


PxU32 NpPhysics::getNbScenes() const
{
	Ps::Mutex::ScopedLock lock(const_cast<Ps::Mutex&>(mSceneAndMaterialMutex));
	return mSceneArray.size();
}


PxU32 NpPhysics::getScenes(PxScene** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	Ps::Mutex::ScopedLock lock(const_cast<Ps::Mutex&>(mSceneAndMaterialMutex));
	return Cm::getArrayOfPointers(userBuffer, bufferSize, startIndex, mSceneArray.begin(), mSceneArray.size());
}


PxRigidStatic* NpPhysics::createRigidStatic(const PxTransform& globalPose)
{
	PX_CHECK_AND_RETURN_NULL(globalPose.isSane(), "PxPhysics::createRigidStatic: invalid transform");
	return NpFactory::getInstance().createRigidStatic(globalPose.getNormalized());
}

PxShape* NpPhysics::createShape(const PxGeometry& geometry, PxMaterial*const * materials, PxU16 materialCount, bool isExclusive, PxShapeFlags shapeFlags)
{
	PX_CHECK_AND_RETURN_NULL(materials, "createShape: material pointer is NULL");
	PX_CHECK_AND_RETURN_NULL(materialCount>0, "createShape: material count is zero");

#if PX_CHECKED
	const bool isHeightfield = geometry.getType() == PxGeometryType::eHEIGHTFIELD;
	if (isHeightfield)
		{
		PX_CHECK_AND_RETURN_NULL(mHeightFieldsRegistered, "NpPhysics::createShape: Creating Heightfield shape without having called PxRegister[Unified]HeightFields()!");
		}
	const bool hasMeshTypeGeom = isHeightfield || (geometry.getType() == PxGeometryType::eTRIANGLEMESH);
	PX_CHECK_AND_RETURN_NULL(!(hasMeshTypeGeom && (shapeFlags & PxShapeFlag::eTRIGGER_SHAPE)), "NpPhysics::createShape: triangle mesh and heightfield triggers are not supported!");
	PX_CHECK_AND_RETURN_NULL(!((shapeFlags & PxShapeFlag::eSIMULATION_SHAPE) && (shapeFlags & PxShapeFlag::eTRIGGER_SHAPE)), "NpPhysics::createShape: shapes cannot simultaneously be trigger shapes and simulation shapes.");
#endif

	return NpFactory::getInstance().createShape(geometry, shapeFlags, materials, materialCount, isExclusive);
}

PxU32 NpPhysics::getNbShapes()	const
{
	return NpFactory::getInstance().getNbShapes();
}

PxU32 NpPhysics::getShapes(PxShape** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const
{
	return NpFactory::getInstance().getShapes(userBuffer, bufferSize, startIndex);
}





PxRigidDynamic* NpPhysics::createRigidDynamic(const PxTransform& globalPose)
{
	PX_CHECK_AND_RETURN_NULL(globalPose.isSane(), "PxPhysics::createRigidDynamic: invalid transform");
	return NpFactory::getInstance().createRigidDynamic(globalPose.getNormalized());
}


PxConstraint* NpPhysics::createConstraint(PxRigidActor* actor0, PxRigidActor* actor1, PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize)
{
	return NpFactory::getInstance().createConstraint(actor0, actor1, connector, shaders, dataSize);
}


PxArticulation* NpPhysics::createArticulation()
{
	return NpFactory::getInstance().createArticulation();
}


// PX_AGGREGATE


PxAggregate* NpPhysics::createAggregate(PxU32 maxSize, bool selfCollisionEnabled)
{
	return NpFactory::getInstance().createAggregate(maxSize, selfCollisionEnabled);
}
//~PX_AGGREGATE


#if PX_USE_PARTICLE_SYSTEM_API
PxParticleSystem* NpPhysics::createParticleSystem(PxU32 maxParticles, bool perParticleRestOffset)
{
	return NpFactory::getInstance().createParticleSystem(maxParticles, perParticleRestOffset);
}


PxParticleFluid* NpPhysics::createParticleFluid(PxU32 maxParticles, bool perParticleRestOffset)
{
	return NpFactory::getInstance().createParticleFluid(maxParticles, perParticleRestOffset);
}
#endif

#if PX_USE_CLOTH_API
PxCloth* NpPhysics::createCloth(const PxTransform& globalPose, PxClothFabric& fabric, const PxClothParticle* particles, PxClothFlags flags)
{
	PX_CHECK_AND_RETURN_NULL(globalPose.isSane(), "PxPhysics::createCloth: invalid transform");
	return NpFactory::getInstance().createCloth(globalPose.getNormalized(), fabric, particles, flags);
}
#endif




///////////////////////////////////////////////////////////////////////////////

NpMaterial* NpPhysics::addMaterial(NpMaterial* m)
{
	if(!m)
		return NULL;

	Ps::Mutex::ScopedLock lock(mSceneAndMaterialMutex);

	//the handle is set inside the setMaterial method
	if(mMasterMaterialManager.setMaterial(*m))
	{
		// Let all scenes know of the new material
		for(PxU32 i=0; i < mSceneArray.size(); i++)
		{
			NpScene* s = getScene(i);
			s->addMaterial(*m);
		}
		return m;
	}
	else
	{
		m->release();
		return NULL;
	}
}

PxMaterial* NpPhysics::createMaterial(PxReal staticFriction, PxReal dynamicFriction, PxReal restitution)
{
	PxMaterial* m = NpFactory::getInstance().createMaterial(staticFriction, dynamicFriction, restitution);
	return addMaterial(static_cast<NpMaterial*>(m));
}

PxU32 NpPhysics::getNbMaterials() const
{
	Ps::Mutex::ScopedLock lock(const_cast<Ps::Mutex&>(mSceneAndMaterialMutex));
	return mMasterMaterialManager.getNumMaterials();
}

PxU32 NpPhysics::getMaterials(PxMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	Ps::Mutex::ScopedLock lock(const_cast<Ps::Mutex&>(mSceneAndMaterialMutex));
	NpMaterialManagerIterator iter(mMasterMaterialManager);
	PxU32 writeCount =0;
	PxU32 index = 0;
	NpMaterial* mat;
	while(iter.getNextMaterial(mat))
	{
		if(index++ < startIndex)
			continue;
		if(writeCount == bufferSize)
			break;
		userBuffer[writeCount++] = mat;
	}
	return writeCount;
}

void NpPhysics::removeMaterialFromTable(NpMaterial& m)
{
	Ps::Mutex::ScopedLock lock(mSceneAndMaterialMutex);

	// Let all scenes know of the deleted material
	for(PxU32 i=0; i < mSceneArray.size(); i++)
	{
		NpScene* s = getScene(i);
		s->removeMaterial(m);
	}

	mMasterMaterialManager.removeMaterial(m);
}

void NpPhysics::updateMaterial(NpMaterial& m)
{
	Ps::Mutex::ScopedLock lock(mSceneAndMaterialMutex);

	// Let all scenes know of the updated material
	for(PxU32 i=0; i < mSceneArray.size(); i++)
	{
		NpScene* s = getScene(i);
		s->updateMaterial(m);
	}
	mMasterMaterialManager.updateMaterial(m);
}

bool NpPhysics::sendMaterialTable(NpScene& scene)
{
	// note: no lock here because this method gets only called at scene creation and there we do lock

	NpMaterialManagerIterator iter(mMasterMaterialManager);
	NpMaterial* mat;
	while(iter.getNextMaterial(mat))
		scene.addMaterial(*mat);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
PxTriangleMesh* NpPhysics::createTriangleMesh(PxInputStream& stream)
{
	return NpFactory::getInstance().createTriangleMesh(stream);
}

PxU32 NpPhysics::getNbTriangleMeshes() const
{
	return NpFactory::getInstance().getNbTriangleMeshes();
}

PxU32 NpPhysics::getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getTriangleMeshes(userBuffer, bufferSize, startIndex);
}

PxHeightField* NpPhysics::createHeightField(PxInputStream& stream)
{
	return NpFactory::getInstance().createHeightField(stream);
}

PxU32 NpPhysics::getNbHeightFields() const
{
	return NpFactory::getInstance().getNbHeightFields();
}

PxU32 NpPhysics::getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getHeightFields(userBuffer, bufferSize, startIndex);
}

///////////////////////////////////////////////////////////////////////////////
PxConvexMesh* NpPhysics::createConvexMesh(PxInputStream& stream)
{
	return NpFactory::getInstance().createConvexMesh(stream);
}


PxU32 NpPhysics::getNbConvexMeshes() const
{
	return NpFactory::getInstance().getNbConvexMeshes();
}

PxU32 NpPhysics::getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex) const
{
	return NpFactory::getInstance().getConvexMeshes(userBuffer, bufferSize, startIndex);
}

///////////////////////////////////////////////////////////////////////////////


#if PX_USE_CLOTH_API
PxClothFabric* NpPhysics::createClothFabric(PxInputStream& stream)
{
	return NpFactory::getInstance().createClothFabric(stream);
}

PxClothFabric* NpPhysics::createClothFabric(const PxClothFabricDesc& desc)
{
	return NpFactory::getInstance().createClothFabric(desc);
}

PxU32 NpPhysics::getNbClothFabrics() const
{
	return NpFactory::getInstance().getNbClothFabrics();
}


PxU32 NpPhysics::getClothFabrics(PxClothFabric** userBuffer, PxU32 bufferSize) const
{
	return NpFactory::getInstance().getClothFabrics(userBuffer, bufferSize);
}

void NpPhysics::registerCloth()
{
	NpFactory::registerCloth();	

	Ps::Mutex::ScopedLock lock(mSceneAndMaterialMutex);
	for(PxU32 i = 0;  i < mSceneArray.size(); i++)
		mSceneArray[i]->getScene().getScScene().createClothSolver();
}
#endif

PxPruningStructure* NpPhysics::createPruningStructure(PxRigidActor*const* actors, PxU32 nbActors)
{
	PX_SIMD_GUARD;

	PX_ASSERT(actors);
	PX_ASSERT(nbActors > 0);

	Sq::PruningStructure* ps = PX_NEW(Sq::PruningStructure)();	
	if(!ps->build(actors, nbActors))
	{
		PX_DELETE_AND_RESET(ps);		
	}
	return ps;
}

#if PX_SUPPORT_GPU_PHYSX
void NpPhysics::registerPhysXIndicatorGpuClient()
{
	Ps::Mutex::ScopedLock lock(mPhysXIndicatorMutex);

	++mNbRegisteredGpuClients;

	mPhysXIndicator.setIsGpu(mNbRegisteredGpuClients>0);
}

void NpPhysics::unregisterPhysXIndicatorGpuClient()
{
	Ps::Mutex::ScopedLock lock(mPhysXIndicatorMutex);

	if (mNbRegisteredGpuClients)
		--mNbRegisteredGpuClients;

	mPhysXIndicator.setIsGpu(mNbRegisteredGpuClients>0);
}
#endif

///////////////////////////////////////////////////////////////////////////////
void NpPhysics::registerDeletionListener(PxDeletionListener& observer, const PxDeletionEventFlags& deletionEvents, bool restrictedObjectSet)
{
	Ps::Mutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(!entry)
	{
		NpDelListenerEntry* e = PX_NEW(NpDelListenerEntry)(deletionEvents, restrictedObjectSet);
		if (e)
		{
			if (mDeletionListenerMap.insert(&observer, e))
				mDeletionListenersExist = true;
			else
			{
				PX_DELETE(e);
				PX_ALWAYS_ASSERT();
			}
		}
	}
	else
		PX_ASSERT(mDeletionListenersExist);
}

void NpPhysics::unregisterDeletionListener(PxDeletionListener& observer)
{
	Ps::Mutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(entry)
	{
		NpDelListenerEntry* e = entry->second;
		mDeletionListenerMap.erase(&observer);
		PX_DELETE(e);
	}
	mDeletionListenersExist = mDeletionListenerMap.size()>0;
}


void NpPhysics::registerDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount)
{
	Ps::Mutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(entry)
	{
		NpDelListenerEntry* e = entry->second;
		PX_CHECK_AND_RETURN(e->restrictedObjectSet, "PxPhysics::registerDeletionListenerObjects: deletion listener is not configured to receive events from specific objects.");

		e->registeredObjects.reserve(e->registeredObjects.size() + observableCount);
		for(PxU32 i=0; i < observableCount; i++)
			e->registeredObjects.insert(observables[i]);
	}
	else
	{
		PX_CHECK_AND_RETURN(false, "PxPhysics::registerDeletionListenerObjects: deletion listener has to be registered in PxPhysics first.");
	}
}


void NpPhysics::unregisterDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount)
{
	Ps::Mutex::ScopedLock lock(mDeletionListenerMutex);

	const DeletionListenerMap::Entry* entry = mDeletionListenerMap.find(&observer);
	if(entry)
	{
		NpDelListenerEntry* e = entry->second;
		if (e->restrictedObjectSet)
		{
			for(PxU32 i=0; i < observableCount; i++)
				e->registeredObjects.erase(observables[i]);
		}
		else
		{
			PX_CHECK_AND_RETURN(false, "PxPhysics::unregisterDeletionListenerObjects: deletion listener is not configured to receive events from specific objects.");
		}
	}
	else
	{
		PX_CHECK_AND_RETURN(false, "PxPhysics::unregisterDeletionListenerObjects: deletion listener has to be registered in PxPhysics first.");
	}
}


void NpPhysics::notifyDeletionListeners(const PxBase* base, void* userData, PxDeletionEventFlag::Enum deletionEvent)
{
	// we don't protect the check for whether there are any listeners, because we don't want to take a hit in the 
	// common case where there are no listeners. Note the API comments here, that users should not register or 
	// unregister deletion listeners while deletions are occurring

	if(mDeletionListenersExist)
	{
		Ps::Mutex::ScopedLock lock(mDeletionListenerMutex);

		const DeletionListenerMap::Entry* delListenerEntries = mDeletionListenerMap.getEntries();
		const PxU32 delListenerEntryCount = mDeletionListenerMap.size();
		for(PxU32 i=0; i < delListenerEntryCount; i++)
		{
			const NpDelListenerEntry* entry = delListenerEntries[i].second;
			
			if (entry->flags & deletionEvent)
			{
				if (entry->restrictedObjectSet)
				{
					if (entry->registeredObjects.contains(base))
						delListenerEntries[i].first->onRelease(base, userData, deletionEvent);
				}
				else
					delListenerEntries[i].first->onRelease(base, userData, deletionEvent);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
const PxTolerancesScale& NpPhysics::getTolerancesScale() const
{
	return mPhysics.getTolerancesScale();
}

PxFoundation& NpPhysics::getFoundation()
{
	return Ps::Foundation::getInstance();
}

PxPhysics& PxGetPhysics()
{
	return NpPhysics::getInstance();
}

PxPhysics* PxCreateBasePhysics(PxU32 version, PxFoundation& foundation, const PxTolerancesScale& scale, bool trackOutstandingAllocations,
	                           physx::PxPvd* pvd)
{
	return NpPhysics::createInstance(version, foundation, scale, trackOutstandingAllocations, static_cast<physx::pvdsdk::PsPvd*>(pvd));
}

void PxRegisterArticulations(PxPhysics& physics)
{
	PX_UNUSED(&physics);	// for the moment
	Dy::PxvRegisterArticulations();
	NpFactory::registerArticulations();	
}

void PxRegisterHeightFields(PxPhysics& physics)
{
	PX_UNUSED(&physics);	// for the moment
	PX_CHECK_AND_RETURN(NpPhysics::getInstance().getNumScenes() == 0, "PxRegisterHeightFields: it is illegal to call a heightfield registration function after you have a scene.");

	PxvRegisterHeightFields();
	Gu::registerHeightFields();	
#if PX_CHECKED
	NpPhysics::heightfieldsAreRegistered();
#endif
}

void PxRegisterLegacyHeightFields(PxPhysics& physics)
{
	PX_CHECK_AND_RETURN(NpPhysics::getInstance().getNumScenes() == 0, "PxRegisterLegacyHeightFields: it is illegal to call a heightfield registration function after you have a scene.");
	PX_UNUSED(&physics);	// for the moment
	PxvRegisterLegacyHeightFields();
	Gu::registerHeightFields();	
#if PX_CHECKED
	NpPhysics::heightfieldsAreRegistered();
#endif
}

void PxRegisterCloth(PxPhysics& physics)
{
	PX_UNUSED(&physics);

#if PX_USE_CLOTH_API
	static_cast<NpPhysics&>(physics).registerCloth();
#endif
}

void PxRegisterParticles(PxPhysics& physics)
{
	PX_UNUSED(&physics);	// for the moment

#if PX_USE_PARTICLE_SYSTEM_API
	Pt::registerParticles();
	NpFactory::registerParticles();
#endif
}

bool NpPhysics::lockScene()
{
	mSceneRunning.lock();

	return true;
}

bool NpPhysics::unlockScene()
{
	mSceneRunning.unlock();

	return true;
}

void PxAddCollectionToPhysics(const PxCollection& collection)
{
	NpFactory& factory = NpFactory::getInstance();
	const Cm::Collection& c = static_cast<const Cm::Collection&>(collection);	
    factory.addCollection(c);
}
