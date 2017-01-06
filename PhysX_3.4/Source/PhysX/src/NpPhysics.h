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


#ifndef PX_PHYSICS_NP_PHYSICS
#define PX_PHYSICS_NP_PHYSICS

#include "foundation/PxProfiler.h"
#include "PxPhysics.h"
#include "PsUserAllocated.h"
#include "GuMeshFactory.h"
#include "NpMaterial.h"
#include "NpPhysicsInsertionCallback.h"
#include "NpMaterialManager.h"
#include "ScPhysics.h"
#include "PsHashSet.h"
#include "PsHashMap.h"

#if PX_SUPPORT_GPU_PHYSX
#include "gpu/NpPhysicsGpu.h"
#endif

#ifdef LINUX
#include <string.h>
#endif

#if PX_SUPPORT_GPU_PHYSX
#include "device/PhysXIndicator.h"
#endif

#include "PsPvd.h"

namespace physx
{
#if PX_SUPPORT_PVD
namespace Vd
{
	class PvdPhysicsClient;
}
#endif
	struct NpMaterialIndexTranslator
	{
		NpMaterialIndexTranslator() : indicesNeedTranslation(false) {}

		Ps::HashMap<PxU16, PxU16>	map;
		bool						indicesNeedTranslation;
	};

	class NpScene;	
	struct PxvOffsetTable;

#if PX_VC
#pragma warning(push)
#pragma warning(disable:4996)	// We have to implement deprecated member functions, do not warn.
#endif

class NpPhysics : public PxPhysics, public Ps::UserAllocated
{
	NpPhysics& operator=(const NpPhysics&);
	NpPhysics(const NpPhysics &);

	struct NpDelListenerEntry : public UserAllocated
	{
		NpDelListenerEntry(const PxDeletionEventFlags& de, bool restrictedObjSet)
			: flags(de)
			, restrictedObjectSet(restrictedObjSet)
		{
		}

		Ps::HashSet<const PxBase*> registeredObjects;  // specifically registered objects for deletion events
		PxDeletionEventFlags flags;
		bool restrictedObjectSet;
	};


									NpPhysics(	const PxTolerancesScale& scale, 
												const PxvOffsetTable& pxvOffsetTable,
												bool trackOutstandingAllocations, 
                                                physx::pvdsdk::PsPvd* pvd);
	virtual							~NpPhysics();

public:
	
	static      NpPhysics*			createInstance(	PxU32 version, 
													PxFoundation& foundation, 
													const PxTolerancesScale& scale,
													bool trackOutstandingAllocations,
													physx::pvdsdk::PsPvd* pvd);

	static		PxU32			releaseInstance();

	static      NpPhysics&		getInstance() { return *mInstance; }

	virtual     void			release();

	virtual		PxScene*		createScene(const PxSceneDesc&);
				void			releaseSceneInternal(PxScene&);
	virtual		PxU32			getNbScenes()	const;
	virtual		PxU32			getScenes(PxScene** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	virtual		PxRigidStatic*		createRigidStatic(const PxTransform&);
	virtual		PxRigidDynamic*		createRigidDynamic(const PxTransform&);
	virtual		PxArticulation*		createArticulation();
	virtual		PxConstraint*		createConstraint(PxRigidActor* actor0, PxRigidActor* actor1, PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize);
	virtual		PxAggregate*		createAggregate(PxU32 maxSize, bool selfCollision);

	virtual		PxShape*			createShape(const PxGeometry&, PxMaterial*const *, PxU16, bool, PxShapeFlags shapeFlags);
	virtual		PxU32				getNbShapes()	const;
	virtual		PxU32				getShapes(PxShape** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

#if PX_USE_PARTICLE_SYSTEM_API
	virtual		PxParticleSystem*	createParticleSystem(PxU32 maxParticles, bool perParticleRestOffset);
	virtual		PxParticleFluid*	createParticleFluid(PxU32 maxParticles, bool perParticleRestOffset);
#endif

#if PX_USE_CLOTH_API
	virtual		PxCloth*			createCloth(const PxTransform& globalPose, PxClothFabric& fabric, const PxClothParticle* particles, PxClothFlags flags);
#endif

	virtual		PxMaterial*			createMaterial(PxReal staticFriction, PxReal dynamicFriction, PxReal restitution);
	virtual		PxU32				getNbMaterials() const;
	virtual		PxU32				getMaterials(PxMaterial** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

	virtual		PxTriangleMesh*		createTriangleMesh(PxInputStream&);
	virtual		PxU32				getNbTriangleMeshes()	const;
	virtual		PxU32				getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;

	virtual		PxHeightField*		createHeightField(PxInputStream& stream);
	virtual		PxU32				getNbHeightFields()	const;
	virtual		PxU32				getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex=0)	const;

	virtual		PxConvexMesh*		createConvexMesh(PxInputStream&);
	virtual		PxU32				getNbConvexMeshes() const;
	virtual		PxU32				getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

#if PX_USE_CLOTH_API
	virtual		PxClothFabric*		createClothFabric(PxInputStream&);
	virtual		PxClothFabric*		createClothFabric(const PxClothFabricDesc& desc);
	virtual		PxU32				getNbClothFabrics() const;
	virtual		PxU32				getClothFabrics(PxClothFabric** userBuffer, PxU32 bufferSize) const;
				void				registerCloth();
#endif

#if PX_SUPPORT_GPU_PHYSX
	void							registerPhysXIndicatorGpuClient();
	void							unregisterPhysXIndicatorGpuClient();
#else
	PX_FORCE_INLINE void			registerPhysXIndicatorGpuClient() {}
	PX_FORCE_INLINE void			unregisterPhysXIndicatorGpuClient() {}
#endif

	virtual		PxPruningStructure*	createPruningStructure(PxRigidActor*const* actors, PxU32 nbActors);

	virtual		const PxTolerancesScale&		getTolerancesScale() const;

	virtual		PxFoundation&		getFoundation();

	bool							lockScene();
	bool							unlockScene();

	PX_INLINE	NpScene*			getScene(PxU32 i) const { return mSceneArray[i]; }
	PX_INLINE	PxU32				getNumScenes() const { return mSceneArray.size(); }
#if PX_CHECKED
	static PX_INLINE	void		heightfieldsAreRegistered() { mHeightFieldsRegistered = true;  }
#endif

#if PX_SUPPORT_GPU_PHYSX
	virtual		NpPhysicsGpu&		getNpPhysicsGpu() { return mPhysicsGpu; }
#endif

	virtual		void				registerDeletionListener(PxDeletionListener& observer, const PxDeletionEventFlags& deletionEvents, bool restrictedObjectSet);
	virtual		void				unregisterDeletionListener(PxDeletionListener& observer);
	virtual		void				registerDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount);
	virtual		void				unregisterDeletionListenerObjects(PxDeletionListener& observer, const PxBase* const* observables, PxU32 observableCount);

				void				notifyDeletionListeners(const PxBase*, void* userData, PxDeletionEventFlag::Enum deletionEvent);
	PX_FORCE_INLINE void			notifyDeletionListenersUserRelease(const PxBase* b, void* userData) { notifyDeletionListeners(b, userData, PxDeletionEventFlag::eUSER_RELEASE); }
	PX_FORCE_INLINE void			notifyDeletionListenersMemRelease(const PxBase* b, void* userData) { notifyDeletionListeners(b, userData, PxDeletionEventFlag::eMEMORY_RELEASE); }

	virtual		PxPhysicsInsertionCallback&	getPhysicsInsertionCallback() { return mObjectInsertion; }

				void				removeMaterialFromTable(NpMaterial&);
				void				updateMaterial(NpMaterial&);
				bool				sendMaterialTable(NpScene&);

				NpMaterialManager&	getMaterialManager()	{	return mMasterMaterialManager;	}

				NpMaterial*			addMaterial(NpMaterial* np);

	static		void				initOffsetTables(PxvOffsetTable& pxvOffsetTable);

	static bool apiReentryLock;

private:
				typedef Ps::CoalescedHashMap<PxDeletionListener*, NpDelListenerEntry*> DeletionListenerMap;

				Ps::Array<NpScene*>	mSceneArray;

				Ps::Mutex			mSceneRunning;

				Sc::Physics					mPhysics;
				NpMaterialManager			mMasterMaterialManager;

				NpPhysicsInsertionCallback	mObjectInsertion;

				struct MeshDeletionListener: public GuMeshFactoryListener
				{
					void onGuMeshFactoryBufferRelease(const PxBase* object, PxType type)
					{
						PX_UNUSED(type);
						NpPhysics::getInstance().notifyDeletionListeners(object, NULL, PxDeletionEventFlag::eMEMORY_RELEASE);
					}
				};

				Ps::Mutex								mDeletionListenerMutex;
				DeletionListenerMap						mDeletionListenerMap;
				MeshDeletionListener					mDeletionMeshListener;
				bool									mDeletionListenersExist;

				Ps::Mutex								mSceneAndMaterialMutex;  // guarantees thread safety for API calls related to scene and material containers

#if PX_SUPPORT_GPU_PHYSX
				PhysXIndicator		mPhysXIndicator;
				PxU32				mNbRegisteredGpuClients;
				Ps::Mutex			mPhysXIndicatorMutex;
				NpPhysicsGpu		mPhysicsGpu;
#endif
#if PX_SUPPORT_PVD	
				physx::pvdsdk::PsPvd*  mPvd;
                Vd::PvdPhysicsClient*   mPvdPhysicsClient;
#endif

	static		PxU32				mRefCount;
	static		NpPhysics*			mInstance;

#if PX_CHECKED
	static		bool				mHeightFieldsRegistered;	//just for error checking
#endif

	friend class NpCollection;
};

#if PX_VC
#pragma warning(pop)
#endif
}

#endif
