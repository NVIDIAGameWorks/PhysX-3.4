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


#ifndef PX_PHYSICS_NP_FACTORY
#define PX_PHYSICS_NP_FACTORY

#include "PsPool.h"
#include "PsMutex.h"
#include "PsHashSet.h"

#include "GuMeshFactory.h"
#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#include "PxShape.h"

#if PX_USE_CLOTH_API
#include "cloth/PxClothTypes.h"
#include "cloth/PxClothFabric.h"
#endif

namespace physx
{

class PxActor;

class PxRigidActor;

class PxRigidStatic;
class NpRigidStatic;

class PxRigidDynamic;
class NpRigidDynamic;

class NpConnectorArray;

struct PxConstraintShaderTable;
class PxConstraintConnector;
class PxConstraint;
class NpConstraint;

class PxArticulation;
class NpArticulation;
class PxArticulationLink;
class NpArticulationLink;
class NpArticulationJoint;

class PxParticleBase;
class PxParticleSystem;
class NpParticleSystem;
class PxParticleFluid;
class NpParticleFluid;

class PxClothFabric;
class NpClothFabric;
class PxCloth;
class NpCloth;
class PxMaterial;
class NpMaterial;

class PxGeometry;

class NpShape;

class NpScene;

class PxAggregate;
class NpAggregate;

class NpConnectorArray;
class NpPtrTableStorageManager;

namespace Cm
{
   class Collection;
}

namespace Scb
{
	class RigidObject;
}

class NpFactoryListener : public GuMeshFactoryListener
{
protected:
	virtual ~NpFactoryListener(){}
public:
#if PX_USE_CLOTH_API
	virtual void onNpFactoryBufferRelease(PxClothFabric& data) = 0;
#endif
};



class NpFactory : public GuMeshFactory
{
	PX_NOCOPY(NpFactory)
public:
												NpFactory();
private:
												~NpFactory();

public:
	static		void							createInstance();
	static		void							destroyInstance();
	static		void							registerArticulations();
	static		void							registerCloth();
	static		void							registerParticles();

				void							release();

				void							addCollection(const Cm::Collection& collection);

	PX_INLINE static NpFactory&					getInstance() { return *mInstance; }

				PxRigidDynamic*					createRigidDynamic(const PxTransform& pose);
				void							addRigidDynamic(PxRigidDynamic*, bool lock=true);
				void							releaseRigidDynamicToPool(NpRigidDynamic&);

				PxRigidStatic*					createRigidStatic(const PxTransform& pose);
				void							addRigidStatic(PxRigidStatic*, bool lock=true);
				void							releaseRigidStaticToPool(NpRigidStatic&);

				NpShape*						createShape(const PxGeometry& geometry,
															PxShapeFlags shapeFlags,
															PxMaterial*const* materials,
															PxU16 materialCount,
															bool isExclusive);

				void							addShape(PxShape*, bool lock=true);
				void							releaseShapeToPool(NpShape&);

				PxU32							getNbShapes() const;
				PxU32							getShapes(PxShape** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

				void							addConstraint(PxConstraint*, bool lock=true);
				PxConstraint*					createConstraint(PxRigidActor* actor0, PxRigidActor* actor1, PxConstraintConnector& connector, const PxConstraintShaderTable& shaders, PxU32 dataSize);
				void							releaseConstraintToPool(NpConstraint&);

				void							addArticulation(PxArticulation*, bool lock=true);
				PxArticulation*					createArticulation();
				NpArticulation*                 createNpArticulation();
				void                            releaseArticulationToPool(NpArticulation& articulation);

				NpArticulationLink*             createNpArticulationLink(NpArticulation&root, NpArticulationLink* parent, const PxTransform& pose);
                void                            releaseArticulationLinkToPool(NpArticulationLink& articulation);
				PxArticulationLink*				createArticulationLink(NpArticulation&, NpArticulationLink* parent, const PxTransform& pose);

				NpArticulationJoint*            createNpArticulationJoint(NpArticulationLink& parent, const PxTransform& parentFrame, NpArticulationLink& child, const PxTransform& childFrame);
               	void                            releaseArticulationJointToPool(NpArticulationJoint& articulationJoint);

				void							addAggregate(PxAggregate*, bool lock=true);
				PxAggregate*					createAggregate(PxU32 maxActors, bool selfCollisions);
				void							releaseAggregateToPool(NpAggregate&);


#if PX_USE_PARTICLE_SYSTEM_API
				NpParticleSystem*               createNpParticleSystem(PxU32 maxParticles, bool perParticleRestOffset);
				void                            releaseParticleSystemToPool(NpParticleSystem& ps);
				NpParticleFluid*                createNpParticleFluid(PxU32 maxParticles, bool perParticleRestOffset);
				void                            releaseParticleFluidToPool(NpParticleFluid& pf);				
				void							addParticleSystem(PxParticleSystem*, bool lock=true);
				PxParticleSystem*				createParticleSystem(PxU32 maxParticles, bool perParticleRestOffset);
				void							addParticleFluid(PxParticleFluid*, bool lock=true);
				PxParticleFluid*				createParticleFluid(PxU32 maxParticles, bool perParticleRestOffset);
#endif

#if PX_USE_CLOTH_API
				NpCloth*                        createNpCloth(const PxTransform& globalPose, PxClothFabric& fabric, const PxClothParticle* particles, PxClothFlags flags);
				void                            releaseClothToPool(NpCloth& cloth);
				void							addCloth(PxCloth* cloth, bool lock=true);
				void							addClothFabric(NpClothFabric* cf, bool lock=true);
				PxClothFabric*					createClothFabric(PxInputStream&);
				PxClothFabric*					createClothFabric(const PxClothFabricDesc& desc);
				NpClothFabric*                  createNpClothFabric();
				void                            releaseClothFabricToPool(NpClothFabric& clothFabric);
				bool							removeClothFabric(PxClothFabric&);
				PxU32							getNbClothFabrics()	const;
				PxU32							getClothFabrics(PxClothFabric** userBuffer, PxU32 bufferSize) const;

				PxCloth*						createCloth(const PxTransform& globalPose, PxClothFabric& fabric, const PxClothParticle* particles, PxClothFlags flags);
#endif

				PxMaterial*						createMaterial(PxReal staticFriction, PxReal dynamicFriction, PxReal restitution);
				void                            releaseMaterialToPool(NpMaterial& material);

				// It's easiest to track these uninvasively, so it's OK to use the Px pointers

				void							onActorRelease(PxActor*);
				void							onConstraintRelease(PxConstraint*);
				void							onAggregateRelease(PxAggregate*);
				void							onArticulationRelease(PxArticulation*);
				void							onShapeRelease(PxShape*);

				NpConnectorArray*				acquireConnectorArray();
				void							releaseConnectorArray(NpConnectorArray*);
				
				NpPtrTableStorageManager&		getPtrTableStorageManager()	{ return *mPtrTableStorageManager; }

#if PX_SUPPORT_PVD
				void							setNpFactoryListener( NpFactoryListener& );
#endif


private:

						void					releaseExclusiveShapeUserReferences();
#if PX_SUPPORT_GPU_PHYSX
				virtual void					notifyReleaseTriangleMesh(const PxTriangleMesh& tm);
				virtual	void					notifyReleaseHeightField(const PxHeightField& hf);
				virtual	void					notifyReleaseConvexMesh(const PxConvexMesh& cm);
#endif

				Ps::Pool<NpConnectorArray>		mConnectorArrayPool;
				Ps::Mutex						mConnectorArrayPoolLock;

				NpPtrTableStorageManager*		mPtrTableStorageManager;

				Ps::HashSet<PxAggregate*>		mAggregateTracking;
				Ps::HashSet<PxArticulation*>	mArticulationTracking;
				Ps::HashSet<PxConstraint*>		mConstraintTracking;
				Ps::HashSet<PxActor*>			mActorTracking;				
				Ps::CoalescedHashSet<PxShape*>	mShapeTracking;

				Ps::Pool2<NpRigidDynamic, 4096>	mRigidDynamicPool;
				Ps::Mutex						mRigidDynamicPoolLock;

				Ps::Pool2<NpRigidStatic, 4096>	mRigidStaticPool;
				Ps::Mutex						mRigidStaticPoolLock;

				Ps::Pool2<NpShape, 4096>		mShapePool;
				Ps::Mutex						mShapePoolLock;

				Ps::Pool2<NpAggregate, 4096>	mAggregatePool;
				Ps::Mutex						mAggregatePoolLock;

				Ps::Pool2<NpConstraint, 4096>	mConstraintPool;
				Ps::Mutex						mConstraintPoolLock;

				Ps::Pool2<NpMaterial, 4096>		mMaterialPool;
				Ps::Mutex						mMaterialPoolLock;

				Ps::Pool2<NpArticulation, 4096>	mArticulationPool;
				Ps::Mutex						mArticulationPoolLock;

				Ps::Pool2<NpArticulationLink, 4096>	mArticulationLinkPool;
				Ps::Mutex						mArticulationLinkPoolLock;

				Ps::Pool2<NpArticulationJoint, 4096> mArticulationJointPool;
				Ps::Mutex						mArticulationJointPoolLock;	

#if PX_USE_PARTICLE_SYSTEM_API
				Ps::Pool2<NpParticleSystem, 4096>	mParticleSystemPool;
				Ps::Mutex						mParticleSystemPoolLock;
				Ps::Pool2<NpParticleFluid, 4096>	mParticleFluidPool;
				Ps::Mutex						mParticleFluidPoolLock;
#endif

#if PX_USE_CLOTH_API
				Ps::Array<NpClothFabric*>		mClothFabricArray;
				Ps::Pool2<NpCloth, 4096>	    mClothPool;
				Ps::Mutex						mClothPoolLock;
				Ps::Pool2<NpClothFabric, 4096>  mClothFabricPool;
				Ps::Mutex						mClothFabricPoolLock;
#endif

	static		NpFactory*						mInstance;

#if PX_SUPPORT_PVD
				NpFactoryListener*				mNpFactoryListener;
#endif

};


}

#endif
