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
#include "PxExtensionsAPI.h"
#include "PsFoundation.h"
#include "PxMetaData.h"
#include "ExtDistanceJoint.h"
#include "ExtD6Joint.h"
#include "ExtFixedJoint.h"
#include "ExtPrismaticJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtSphericalJoint.h"
#include "PxRepXSerializer.h"
#include "SnRepXCoreSerializer.h"
#include "SnJointRepXSerializer.h"
#include "PxExtensionMetaDataObjects.h"
#include "PxSerializer.h"
#include "ExtSerialization.h"

#if PX_SUPPORT_PVD
#include "ExtPvd.h"
#include "PxPvdDataStream.h"
#include "PxPvdClient.h"
#include "PsPvd.h"
#endif

using namespace physx;
using namespace physx::pvdsdk;

#if PX_SUPPORT_PVD
struct JointConnectionHandler : public PvdClient
{
	JointConnectionHandler() : mPvd(NULL),mConnected(false){}

	PvdDataStream*		getDataStream()
	{
		return NULL;
	}	
	PvdUserRenderer*    getUserRender()
	{
		return NULL;
	}

	void onPvdConnected()
	{
		PvdDataStream* stream = PvdDataStream::create(mPvd);
		if(stream)
		{
			mConnected = true;
			Ext::Pvd::sendClassDescriptions(*stream);	
			stream->release();
		}		
	}

	bool isConnected() const
	{
		return mConnected;
	}

	void onPvdDisconnected()
	{
		mConnected = false;
	}

	void flush()
	{
	}

	PsPvd* mPvd;
	bool mConnected;
};

static JointConnectionHandler gPvdHandler;
#endif

bool PxInitExtensions(PxPhysics& physics, PxPvd* pvd)
{
	PX_ASSERT(static_cast<Ps::Foundation*>(&physics.getFoundation()) == &Ps::Foundation::getInstance());
	PX_UNUSED(physics);
	PX_UNUSED(pvd);
	Ps::Foundation::incRefCount();

#if PX_SUPPORT_PVD
	if(pvd)
	{
		gPvdHandler.mPvd = static_cast<PsPvd*>(pvd);
		gPvdHandler.mPvd->addClient(&gPvdHandler);
	}
#endif

	return true;
}

void PxCloseExtensions(void)
{	
	Ps::Foundation::decRefCount();

#if PX_SUPPORT_PVD
	if(gPvdHandler.mConnected)
	{	
		PX_ASSERT(gPvdHandler.mPvd);
		gPvdHandler.mPvd->removeClient(&gPvdHandler);
		gPvdHandler.mPvd = NULL;
	}
#endif
}

void Ext::RegisterExtensionsSerializers(PxSerializationRegistry& sr)
{
	//for repx serialization
	sr.registerRepXSerializer(PxConcreteType::eMATERIAL,			PX_NEW_REPX_SERIALIZER( PxMaterialRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eSHAPE,				PX_NEW_REPX_SERIALIZER( PxShapeRepXSerializer ));	
//	sr.registerRepXSerializer(PxConcreteType::eTRIANGLE_MESH,		PX_NEW_REPX_SERIALIZER( PxTriangleMeshRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eTRIANGLE_MESH_BVH33,	PX_NEW_REPX_SERIALIZER( PxBVH33TriangleMeshRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eTRIANGLE_MESH_BVH34,	PX_NEW_REPX_SERIALIZER( PxBVH34TriangleMeshRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eHEIGHTFIELD,			PX_NEW_REPX_SERIALIZER( PxHeightFieldRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eCONVEX_MESH,			PX_NEW_REPX_SERIALIZER( PxConvexMeshRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eRIGID_STATIC,		PX_NEW_REPX_SERIALIZER( PxRigidStaticRepXSerializer ));	
	sr.registerRepXSerializer(PxConcreteType::eRIGID_DYNAMIC,		PX_NEW_REPX_SERIALIZER( PxRigidDynamicRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eARTICULATION,		PX_NEW_REPX_SERIALIZER( PxArticulationRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eAGGREGATE,			PX_NEW_REPX_SERIALIZER( PxAggregateRepXSerializer ));
	
#if PX_USE_CLOTH_API
	sr.registerRepXSerializer(PxConcreteType::eCLOTH_FABRIC,		PX_NEW_REPX_SERIALIZER( PxClothFabricRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eCLOTH,				PX_NEW_REPX_SERIALIZER( PxClothRepXSerializer ));
#endif
#if PX_USE_PARTICLE_SYSTEM_API
	sr.registerRepXSerializer(PxConcreteType::ePARTICLE_SYSTEM,		PX_NEW_REPX_SERIALIZER( PxParticleRepXSerializer<PxParticleSystem> ));
	sr.registerRepXSerializer(PxConcreteType::ePARTICLE_FLUID,		PX_NEW_REPX_SERIALIZER( PxParticleRepXSerializer<PxParticleFluid> ));
#endif
	
	sr.registerRepXSerializer(PxJointConcreteType::eFIXED,			PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxFixedJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eDISTANCE,		PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxDistanceJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eD6,				PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxD6Joint> ));
	sr.registerRepXSerializer(PxJointConcreteType::ePRISMATIC,		PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxPrismaticJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eREVOLUTE,		PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxRevoluteJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eSPHERICAL,		PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxSphericalJoint> ));

	//for binary serialization
	sr.registerSerializer(PxJointConcreteType::eFIXED,				PX_NEW_SERIALIZER_ADAPTER( FixedJoint ));
	sr.registerSerializer(PxJointConcreteType::eDISTANCE,			PX_NEW_SERIALIZER_ADAPTER( DistanceJoint ));
	sr.registerSerializer(PxJointConcreteType::eD6,					PX_NEW_SERIALIZER_ADAPTER( D6Joint) );
	sr.registerSerializer(PxJointConcreteType::ePRISMATIC,			PX_NEW_SERIALIZER_ADAPTER( PrismaticJoint ));
	sr.registerSerializer(PxJointConcreteType::eREVOLUTE,			PX_NEW_SERIALIZER_ADAPTER( RevoluteJoint ));
	sr.registerSerializer(PxJointConcreteType::eSPHERICAL,			PX_NEW_SERIALIZER_ADAPTER( SphericalJoint ));
}

void Ext::UnregisterExtensionsSerializers(PxSerializationRegistry& sr)
{
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eFIXED));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eDISTANCE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eD6 ));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::ePRISMATIC));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eREVOLUTE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eSPHERICAL));

	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eMATERIAL));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eSHAPE));	
//	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eTRIANGLE_MESH));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eTRIANGLE_MESH_BVH33));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eTRIANGLE_MESH_BVH34));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eHEIGHTFIELD));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eCONVEX_MESH));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eRIGID_STATIC));	
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eRIGID_DYNAMIC));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eARTICULATION));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eAGGREGATE));
	
#if PX_USE_CLOTH_API
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eCLOTH_FABRIC));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eCLOTH));
#endif
#if PX_USE_PARTICLE_SYSTEM_API
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::ePARTICLE_SYSTEM));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::ePARTICLE_FLUID));
#endif
	
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eFIXED));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eDISTANCE));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eD6));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::ePRISMATIC));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eREVOLUTE));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eSPHERICAL));
}
