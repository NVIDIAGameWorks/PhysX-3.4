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

// PX_DUMMY_SYMBOL

#if PX_SUPPORT_PVD

#include "pvd/PxPvdTransport.h"
#include "PxPhysics.h"
#include "PxPvdClient.h"
#include "PxPvdDataStream.h"
#include "PxPvdObjectModelBaseTypes.h"
#include "PvdPhysicsClient.h"
#include "PvdTypeNames.h"

using namespace physx;
using namespace physx::Vd;

PvdPhysicsClient::PvdPhysicsClient(PsPvd* pvd)
: mPvd(pvd), mPvdDataStream(NULL), mIsConnected(false)
{	
}

PvdPhysicsClient::~PvdPhysicsClient()
{
	mPvd->removeClient(this);	
}

PvdDataStream* PvdPhysicsClient::getDataStream()
{
	return mPvdDataStream;
}

PvdMetaDataBinding* PvdPhysicsClient::getMetaDataBinding()
{
	return &mMetaDataBinding;
}

PvdUserRenderer* PvdPhysicsClient::getUserRender()
{
	PX_ASSERT(0);
	return NULL;
}

bool PvdPhysicsClient::isConnected() const
{
	return mIsConnected;
}

void PvdPhysicsClient::onPvdConnected()
{
	if(mIsConnected || !mPvd)
		return;

	mIsConnected = true;	
	mPvdDataStream = PvdDataStream::create(mPvd); 	
	sendEntireSDK();
}

void PvdPhysicsClient::onPvdDisconnected()
{
	if(!mIsConnected)
		return;
	mIsConnected = false;

	mPvdDataStream->release();
	mPvdDataStream = NULL;	
}

void PvdPhysicsClient::flush()
{
}

void PvdPhysicsClient::sendEntireSDK()
{
	PxPhysics& physics = PxGetPhysics();
	
	mMetaDataBinding.registerSDKProperties(*mPvdDataStream);
	mPvdDataStream->createInstance(&physics);
	
	mPvdDataStream->setIsTopLevelUIElement(&physics, true);
	mMetaDataBinding.sendAllProperties(*mPvdDataStream, physics);

#define SEND_BUFFER_GROUP(type, name)                                                                                  \
	{                                                                                                                  \
		physx::shdfnd::Array<type*> buffers;                                                                          \
		PxU32 numBuffers = physics.getNb##name();                                                                      \
		buffers.resize(numBuffers);                                                                                    \
		physics.get##name(buffers.begin(), numBuffers);                                                                \
		for(PxU32 i = 0; i < numBuffers; i++)                                                                          \
		{                                                                                                              \
		if(mPvd->registerObject(buffers[i]))                                                                   \
				createPvdInstance(buffers[i]);                                                                         \
		}                                                                                                         \
	}
	
	SEND_BUFFER_GROUP(PxMaterial, Materials);
	SEND_BUFFER_GROUP(PxTriangleMesh, TriangleMeshes);
	SEND_BUFFER_GROUP(PxConvexMesh, ConvexMeshes);
	SEND_BUFFER_GROUP(PxHeightField, HeightFields);

#if PX_USE_CLOTH_API
	SEND_BUFFER_GROUP(PxClothFabric, ClothFabrics);
#endif
}

void PvdPhysicsClient::destroyPvdInstance(const PxPhysics* physics)
{
	if(mPvdDataStream)
	     mPvdDataStream->destroyInstance(physics);
}

void PvdPhysicsClient::createPvdInstance(const PxTriangleMesh* triMesh)
{
	mMetaDataBinding.createInstance(*mPvdDataStream, *triMesh, PxGetPhysics());
}

void PvdPhysicsClient::destroyPvdInstance(const PxTriangleMesh* triMesh)
{
	mMetaDataBinding.destroyInstance(*mPvdDataStream, *triMesh, PxGetPhysics());
}

void PvdPhysicsClient::createPvdInstance(const PxConvexMesh* convexMesh)
{
	mMetaDataBinding.createInstance(*mPvdDataStream, *convexMesh, PxGetPhysics());
}

void PvdPhysicsClient::destroyPvdInstance(const PxConvexMesh* convexMesh)
{
	mMetaDataBinding.destroyInstance(*mPvdDataStream, *convexMesh, PxGetPhysics());
}

void PvdPhysicsClient::createPvdInstance(const PxHeightField* heightField)
{
	mMetaDataBinding.createInstance(*mPvdDataStream, *heightField, PxGetPhysics());
}

void PvdPhysicsClient::destroyPvdInstance(const PxHeightField* heightField)
{
	mMetaDataBinding.destroyInstance(*mPvdDataStream, *heightField, PxGetPhysics());
}

void PvdPhysicsClient::createPvdInstance(const PxMaterial* mat)
{
	mMetaDataBinding.createInstance(*mPvdDataStream, *mat, PxGetPhysics());
}

void PvdPhysicsClient::updatePvdProperties(const PxMaterial* mat)
{
	mMetaDataBinding.sendAllProperties(*mPvdDataStream, *mat);
}

void PvdPhysicsClient::destroyPvdInstance(const PxMaterial* mat)
{
	mMetaDataBinding.destroyInstance(*mPvdDataStream, *mat, PxGetPhysics());
}

#if PX_USE_CLOTH_API
void PvdPhysicsClient::createPvdInstance(const PxClothFabric* fabric)
{
	mMetaDataBinding.createInstance(*mPvdDataStream, *fabric, PxGetPhysics());
	// Physics level buffer object update, must be flushed immediatedly.
}

void PvdPhysicsClient::destroyPvdInstance(const PxClothFabric* fabric)
{
	mMetaDataBinding.destroyInstance(*mPvdDataStream, *fabric, PxGetPhysics());
	// Physics level buffer object update, must be flushed immediatedly.
}
#endif

void PvdPhysicsClient::onGuMeshFactoryBufferRelease(const PxBase* object, PxType typeID)
{
	if(!mIsConnected || !mPvd)
		return;

	if(mPvd->unRegisterObject(object))
	{
		switch(typeID)
		{
		case PxConcreteType::eHEIGHTFIELD:
			destroyPvdInstance(static_cast<const PxHeightField*>(object));
			break;

		case PxConcreteType::eCONVEX_MESH:
			destroyPvdInstance(static_cast<const PxConvexMesh*>(object));
			break;

		case PxConcreteType::eTRIANGLE_MESH_BVH33:
		case PxConcreteType::eTRIANGLE_MESH_BVH34:
			destroyPvdInstance(static_cast<const PxTriangleMesh*>(object));
			break;

		default:
			break;
		}
	}
}
#if PX_USE_CLOTH_API
void PvdPhysicsClient::onNpFactoryBufferRelease(PxClothFabric& data)
{
	if(!mIsConnected || !mPvd)
		return;
	if(mPvd->unRegisterObject(&data))
		destroyPvdInstance(&data);
}
#endif

void PvdPhysicsClient::reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
{
    if(mIsConnected)
	{
		mPvdDataStream->sendErrorMessage(code, message, file, PxU32(line));
	}
}

#endif // PX_SUPPORT_PVD
