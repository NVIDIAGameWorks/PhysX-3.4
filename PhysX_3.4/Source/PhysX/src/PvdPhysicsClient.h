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

#ifndef PVD_PHYSICS_CLIENT_H
#define PVD_PHYSICS_CLIENT_H
#if PX_SUPPORT_PVD
#include "foundation/PxErrorCallback.h"
#include "PxPvdClient.h"
#include "PvdMetaDataPvdBinding.h"
#include "NpFactory.h"
#include "PsHashMap.h"
#include "PsMutex.h"
#include "PsPvd.h"

namespace physx
{
class PxProfileMemoryEventBuffer;

namespace Vd
{

class PvdPhysicsClient : public PvdClient, public PxErrorCallback, public NpFactoryListener, public shdfnd::UserAllocated
{
	PX_NOCOPY(PvdPhysicsClient)
  public:
	PvdPhysicsClient(PsPvd* pvd);
	virtual ~PvdPhysicsClient();

	bool isConnected() const;
	void onPvdConnected();
	void onPvdDisconnected();
	void flush();

	physx::pvdsdk::PvdDataStream* getDataStream();
	PvdMetaDataBinding* getMetaDataBinding();
	PvdUserRenderer* getUserRender();
	
	void sendEntireSDK();	
	void destroyPvdInstance(const PxPhysics* physics);

	// NpFactoryListener
	virtual void onGuMeshFactoryBufferRelease(const PxBase* object, PxType typeID);
#if PX_USE_CLOTH_API
	virtual void onNpFactoryBufferRelease(PxClothFabric& data);
#endif
	/// NpFactoryListener

	// PxErrorCallback
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);

  private:
	void createPvdInstance(const PxTriangleMesh* triMesh);
	void destroyPvdInstance(const PxTriangleMesh* triMesh);
	void createPvdInstance(const PxConvexMesh* convexMesh);
	void destroyPvdInstance(const PxConvexMesh* convexMesh);
	void createPvdInstance(const PxHeightField* heightField);
	void destroyPvdInstance(const PxHeightField* heightField);
	void createPvdInstance(const PxMaterial* mat);
	void destroyPvdInstance(const PxMaterial* mat);
	void updatePvdProperties(const PxMaterial* mat);
#if PX_USE_CLOTH_API
	void createPvdInstance(const PxClothFabric* fabric);
	void destroyPvdInstance(const PxClothFabric* fabric);
#endif

	PsPvd*  mPvd;
	PvdDataStream* mPvdDataStream;
	PvdMetaDataBinding mMetaDataBinding;	
	bool mIsConnected;	
};

} // namespace Vd
} // namespace physx

#endif // PX_SUPPORT_PVD
#endif // PVD_PHYSICS_CLIENT_H
