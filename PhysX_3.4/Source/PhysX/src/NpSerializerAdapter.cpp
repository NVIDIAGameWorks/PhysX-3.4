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

#include "GuHeightField.h"
#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "GuTriangleMeshBV4.h"
#include "GuTriangleMeshRTree.h"
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
#include "NpAggregate.h"
#include "GuHeightFieldData.h"

#include "SqPruningStructure.h"

#include "PxBase.h"
#include "PxSerialFramework.h"
#include "PxSerializer.h"
#include "PxPhysicsSerialization.h"

namespace physx
{
	using namespace physx::Gu;

	template<>
	void PxSerializerDefaultAdapter<NpRigidDynamic>::exportData(PxBase& obj, PxSerializationContext& s)   const 
	{
		PxU32 classSize = sizeof(NpRigidDynamic);
		NpRigidDynamic& dynamic = static_cast<NpRigidDynamic&>(obj);

		PxsBodyCore serialCore;
		size_t address = dynamic.getScbBodyFast().getScBody().getSerialCore(serialCore);
		PxU32 offset =  PxU32(address - reinterpret_cast<size_t>(&dynamic));
		PX_ASSERT(offset + sizeof(serialCore) <= classSize);
		s.writeData(&dynamic, offset); 
		s.writeData(&serialCore, sizeof(serialCore));
		void* tail = reinterpret_cast<PxU8*>(&dynamic) + offset + sizeof(serialCore);
		s.writeData(tail, classSize - offset - sizeof(serialCore));
	}

	template<>
	void PxSerializerDefaultAdapter<NpRigidDynamic>::registerReferences(PxBase& obj, PxSerializationContext& s)   const 
	{
		NpRigidDynamic& dynamic = static_cast<NpRigidDynamic&>(obj);

		s.registerReference(obj, PX_SERIAL_REF_KIND_PXBASE, size_t(&obj));

		struct RequiresCallback : public PxProcessPxBaseCallback
		{
			RequiresCallback(physx::PxSerializationContext& c) : context(c) {}
			RequiresCallback& operator=(const RequiresCallback&) { PX_ASSERT(0); return *this; } //PX_NOCOPY doesn't work for local classes
			void process(PxBase& base)
			{
				context.registerReference(base, PX_SERIAL_REF_KIND_PXBASE, size_t(&base));
			}
			PxSerializationContext& context;
		};

		RequiresCallback callback(s);
		dynamic.requires(callback);
	}

	template<>
	void PxSerializerDefaultAdapter<NpShape>::registerReferences(PxBase& obj, PxSerializationContext& s)   const 
	{	
		NpShape& shape = static_cast<NpShape&>(obj);

		s.registerReference(obj, PX_SERIAL_REF_KIND_PXBASE, size_t(&obj));

		struct RequiresCallback : public PxProcessPxBaseCallback
		{
			RequiresCallback(physx::PxSerializationContext& c) : context(c) {}
			RequiresCallback &operator=(const RequiresCallback&) { PX_ASSERT(0); return *this; } //PX_NOCOPY doesn't work for local classes
			void process(PxBase& base)
			{
				PxMaterial* pxMaterial = base.is<PxMaterial>();
				if (!pxMaterial)
				{
					context.registerReference(base, PX_SERIAL_REF_KIND_PXBASE, size_t(&base));
				}
				else
				{
					//ideally we would move this part to ScShapeCore but we don't yet have a MaterialManager available there.
					PxU32 index = static_cast<NpMaterial*>(pxMaterial)->getHandle();
					context.registerReference(base, PX_SERIAL_REF_KIND_MATERIAL_IDX, size_t(index));
				}
			}
			PxSerializationContext& context;
		};

		RequiresCallback callback(s);
		shape.requires(callback);
	}

	template<>
	bool PxSerializerDefaultAdapter<NpConstraint>::isSubordinate() const
	{
		return true;
	}

	template<>
	bool PxSerializerDefaultAdapter<NpArticulationJoint>::isSubordinate() const
	{
		return true;
	}

	template<>
	bool PxSerializerDefaultAdapter<NpArticulationLink>::isSubordinate() const
	{
		return true;
	}
}

using namespace physx;

void PxRegisterPhysicsSerializers(PxSerializationRegistry& sr)
{
	sr.registerSerializer(PxConcreteType::eCONVEX_MESH,			PX_NEW_SERIALIZER_ADAPTER(ConvexMesh));
	sr.registerSerializer(PxConcreteType::eTRIANGLE_MESH_BVH33,	PX_NEW_SERIALIZER_ADAPTER(RTreeTriangleMesh));
	sr.registerSerializer(PxConcreteType::eTRIANGLE_MESH_BVH34,	PX_NEW_SERIALIZER_ADAPTER(BV4TriangleMesh));
	sr.registerSerializer(PxConcreteType::eHEIGHTFIELD,			PX_NEW_SERIALIZER_ADAPTER(HeightField));
	sr.registerSerializer(PxConcreteType::eRIGID_DYNAMIC,		PX_NEW_SERIALIZER_ADAPTER(NpRigidDynamic));
	sr.registerSerializer(PxConcreteType::eRIGID_STATIC,		PX_NEW_SERIALIZER_ADAPTER(NpRigidStatic));
	sr.registerSerializer(PxConcreteType::eSHAPE,				PX_NEW_SERIALIZER_ADAPTER(NpShape));
	sr.registerSerializer(PxConcreteType::eMATERIAL,			PX_NEW_SERIALIZER_ADAPTER(NpMaterial));
	sr.registerSerializer(PxConcreteType::eCONSTRAINT,			PX_NEW_SERIALIZER_ADAPTER(NpConstraint));
	sr.registerSerializer(PxConcreteType::eAGGREGATE,			PX_NEW_SERIALIZER_ADAPTER(NpAggregate));
	sr.registerSerializer(PxConcreteType::eARTICULATION,		PX_NEW_SERIALIZER_ADAPTER(NpArticulation));
	sr.registerSerializer(PxConcreteType::eARTICULATION_LINK,	PX_NEW_SERIALIZER_ADAPTER(NpArticulationLink));
	sr.registerSerializer(PxConcreteType::eARTICULATION_JOINT,	PX_NEW_SERIALIZER_ADAPTER(NpArticulationJoint));
	sr.registerSerializer(PxConcreteType::ePRUNING_STRUCTURE,	PX_NEW_SERIALIZER_ADAPTER(Sq::PruningStructure));
	
#if PX_USE_CLOTH_API
	sr.registerSerializer(PxConcreteType::eCLOTH,				PX_NEW_SERIALIZER_ADAPTER(NpCloth));
	sr.registerSerializer(PxConcreteType::eCLOTH_FABRIC,		PX_NEW_SERIALIZER_ADAPTER(NpClothFabric));
#endif

#if PX_USE_PARTICLE_SYSTEM_API
	sr.registerSerializer(PxConcreteType::ePARTICLE_SYSTEM,		PX_NEW_SERIALIZER_ADAPTER(NpParticleSystem));
	sr.registerSerializer(PxConcreteType::ePARTICLE_FLUID,		PX_NEW_SERIALIZER_ADAPTER(NpParticleFluid));
#endif

}


void PxUnregisterPhysicsSerializers(PxSerializationRegistry& sr)
{
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCONVEX_MESH));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eTRIANGLE_MESH_BVH33));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eTRIANGLE_MESH_BVH34));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eHEIGHTFIELD));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eRIGID_DYNAMIC));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eRIGID_STATIC));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eSHAPE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eMATERIAL));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCONSTRAINT));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eAGGREGATE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eARTICULATION));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eARTICULATION_LINK));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eARTICULATION_JOINT));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::ePRUNING_STRUCTURE));
	
#if PX_USE_CLOTH_API
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCLOTH));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCLOTH_FABRIC));
#endif

#if PX_USE_PARTICLE_SYSTEM_API
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::ePARTICLE_SYSTEM));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::ePARTICLE_FLUID));
#endif
}
