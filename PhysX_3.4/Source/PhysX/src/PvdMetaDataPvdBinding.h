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

#ifndef PX_META_DATA_PVD_BINDING_H
#define PX_META_DATA_PVD_BINDING_H

#if PX_SUPPORT_PVD

#include "PxPhysXConfig.h"
#include "PsArray.h"

namespace physx
{
	namespace pvdsdk
	{
		class PsPvd;
		class PvdDataStream;
        struct PvdMetaDataBindingData;
	}
}

namespace physx
{

namespace Sc
{
struct Contact;
struct ClothBulkData;
}

namespace Vd
{

using namespace physx::pvdsdk;

class PvdVisualizer
{
  protected:
	virtual ~PvdVisualizer()
	{
	}

  public:
	virtual void visualize(PxArticulationLink& link) = 0;
};

class PvdMetaDataBinding
{
	PvdMetaDataBindingData* mBindingData;
	
  public:
	PvdMetaDataBinding();
	~PvdMetaDataBinding();
	void registerSDKProperties(PvdDataStream& inStream);

	void sendAllProperties(PvdDataStream& inStream, const PxPhysics& inPhysics);

	void sendAllProperties(PvdDataStream& inStream, const PxScene& inScene);
	// per frame update
	void sendBeginFrame(PvdDataStream& inStream, const PxScene* inScene, PxReal simulateElapsedTime);
	void sendContacts(PvdDataStream& inStream, const PxScene& inScene, shdfnd::Array<Sc::Contact>& inContacts);
	void sendContacts(PvdDataStream& inStream, const PxScene& inScene);
	void sendSceneQueries(PvdDataStream& inStream, const PxScene& inScene, PsPvd* pvd);
	void sendStats(PvdDataStream& inStream, const PxScene* inScene, void* triMeshCacheStats);
	void sendEndFrame(PvdDataStream& inStream, const PxScene* inScene);

	void createInstance(PvdDataStream& inStream, const PxMaterial& inMaterial, const PxPhysics& ownerPhysics);
	void sendAllProperties(PvdDataStream& inStream, const PxMaterial& inMaterial);
	void destroyInstance(PvdDataStream& inStream, const PxMaterial& inMaterial, const PxPhysics& ownerPhysics);

	void createInstance(PvdDataStream& inStream, const PxHeightField& inData, const PxPhysics& ownerPhysics);
	void sendAllProperties(PvdDataStream& inStream, const PxHeightField& inData);
	void destroyInstance(PvdDataStream& inStream, const PxHeightField& inData, const PxPhysics& ownerPhysics);

	void createInstance(PvdDataStream& inStream, const PxConvexMesh& inData, const PxPhysics& ownerPhysics);
	void destroyInstance(PvdDataStream& inStream, const PxConvexMesh& inData, const PxPhysics& ownerPhysics);

	void createInstance(PvdDataStream& inStream, const PxTriangleMesh& inData, const PxPhysics& ownerPhysics);
	void destroyInstance(PvdDataStream& inStream, const PxTriangleMesh& inData, const PxPhysics& ownerPhysics);

	void createInstance(PvdDataStream& inStream, const PxRigidStatic& inObj, const PxScene& ownerScene, const PxPhysics& ownerPhysics, PsPvd* pvd);
	void sendAllProperties(PvdDataStream& inStream, const PxRigidStatic& inObj);
	void destroyInstance(PvdDataStream& inStream, const PxRigidStatic& inObj, const PxScene& ownerScene);

	void createInstance(PvdDataStream& inStream, const PxRigidDynamic& inObj, const PxScene& ownerScene, const PxPhysics& ownerPhysics, PsPvd* pvd);
	void sendAllProperties(PvdDataStream& inStream, const PxRigidDynamic& inObj);
	void destroyInstance(PvdDataStream& inStream, const PxRigidDynamic& inObj, const PxScene& ownerScene);

	void createInstance(PvdDataStream& inStream, const PxArticulation& inObj, const PxScene& ownerScene, const PxPhysics& ownerPhysics, PsPvd* pvd);
	void sendAllProperties(PvdDataStream& inStream, const PxArticulation& inObj);
	void destroyInstance(PvdDataStream& inStream, const PxArticulation& inObj, const PxScene& ownerScene);

	void createInstance(PvdDataStream& inStream, const PxArticulationLink& inObj, const PxPhysics& ownerPhysics, PsPvd* pvd);
	void sendAllProperties(PvdDataStream& inStream, const PxArticulationLink& inObj);
	void destroyInstance(PvdDataStream& inStream, const PxArticulationLink& inObj);

	void createInstance(PvdDataStream& inStream, const PxShape& inObj, const PxRigidActor& owner, const PxPhysics& ownerPhysics, PsPvd* pvd);
	void sendAllProperties(PvdDataStream& inStream, const PxShape& inObj);
	void releaseAndRecreateGeometry(PvdDataStream& inStream, const PxShape& inObj, PxPhysics& ownerPhysics, PsPvd* pvd);
	void updateMaterials(PvdDataStream& inStream, const PxShape& inObj, PsPvd* pvd);
	void destroyInstance(PvdDataStream& inStream, const PxShape& inObj, const PxRigidActor& owner);

	// These are created as part of the articulation link's creation process, so outside entities don't need to
	// create them.
	void sendAllProperties(PvdDataStream& inStream, const PxArticulationJoint& inObj);

	// per frame update
	void updateDynamicActorsAndArticulations(PvdDataStream& inStream, const PxScene* inScene, PvdVisualizer* linkJointViz);

	// Origin Shift
	void originShift(PvdDataStream& inStream, const PxScene* inScene, PxVec3 shift);

#if PX_USE_PARTICLE_SYSTEM_API
	void createInstance(PvdDataStream& inStream, const PxParticleSystem& inObj, const PxScene& ownerScene);
	void sendAllProperties(PvdDataStream& inStream, const PxParticleSystem& inObj);
	// per frame update
	void sendArrays(PvdDataStream& inStream, const PxParticleSystem& inObj, PxParticleReadData& inData, PxU32 inFlags);
	void destroyInstance(PvdDataStream& inStream, const PxParticleSystem& inObj, const PxScene& ownerScene);

	void createInstance(PvdDataStream& inStream, const PxParticleFluid& inObj, const PxScene& ownerScene);
	void sendAllProperties(PvdDataStream& inStream, const PxParticleFluid& inObj);
	// per frame update
	void sendArrays(PvdDataStream& inStream, const PxParticleFluid& inObj, PxParticleFluidReadData& inData,
	                PxU32 inFlags);
	void destroyInstance(PvdDataStream& inStream, const PxParticleFluid& inObj, const PxScene& ownerScene);
#endif

#if PX_USE_CLOTH_API
	void createInstance(PvdDataStream& inStream, const PxClothFabric& fabric, const PxPhysics& ownerPhysics);
	void sendAllProperties(PvdDataStream& inStream, const PxClothFabric& fabric);
	void destroyInstance(PvdDataStream& inStream, const PxClothFabric& fabric, const PxPhysics& ownerPhysics);

	void createInstance(PvdDataStream& inStream, const PxCloth& cloth, const PxScene& ownerScene, const PxPhysics& ownerPhysics, PsPvd* pvd);
	void sendAllProperties(PvdDataStream& inStream, const PxCloth& cloth);
	void sendSimpleProperties(PvdDataStream& inStream, const PxCloth& cloth);
	void sendMotionConstraints(PvdDataStream& inStream, const PxCloth& cloth);
	void sendCollisionSpheres(PvdDataStream& inStream, const PxCloth& cloth, bool sendPairs = true);
	void sendCollisionTriangles(PvdDataStream& inStream, const PxCloth& cloth);
	void sendVirtualParticles(PvdDataStream& inStream, const PxCloth& cloth);
	void sendSeparationConstraints(PvdDataStream& inStream, const PxCloth& cloth);
	void sendRestPositions(PvdDataStream& inStream, const PxCloth& cloth);
	void sendSelfCollisionIndices(PvdDataStream& inStream, const PxCloth& cloth);
	void sendParticleAccelerations(PvdDataStream& inStream, const PxCloth& cloth);
	// per frame update
	void updateCloths(PvdDataStream& inStream, const PxScene& inScene);
	void destroyInstance(PvdDataStream& inStream, const PxCloth& cloth, const PxScene& ownerScene);
#endif

	void createInstance(PvdDataStream& inStream, const PxAggregate& inObj, const PxScene& ownerScene);
	void sendAllProperties(PvdDataStream& inStream, const PxAggregate& inObj);
	void destroyInstance(PvdDataStream& inStream, const PxAggregate& inObj, const PxScene& ownerScene);
	void detachAggregateActor(PvdDataStream& inStream, const PxAggregate& inObj, const PxActor& inActor);
	void attachAggregateActor(PvdDataStream& inStream, const PxAggregate& inObj, const PxActor& inActor);

	template <typename TDataType>
	void registrarPhysicsObject(PvdDataStream&, const TDataType&, PsPvd*);
};
}
}

#endif
#endif
