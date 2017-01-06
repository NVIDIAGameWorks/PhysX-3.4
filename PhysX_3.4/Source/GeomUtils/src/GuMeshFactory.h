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

#ifndef GU_MESH_FACTORY_H
#define GU_MESH_FACTORY_H

#include "foundation/PxIO.h"
#include "PxTriangleMesh.h"
#include "PxConvexMesh.h"
#include "PxHeightField.h"

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#include "PsMutex.h"
#include "PsArray.h"

#include "PsUserAllocated.h"
#include "PsHashSet.h"

namespace physx
{

class PxHeightFieldDesc;

namespace Gu
{
	class ConvexMesh;
	class HeightField;
	class TriangleMesh;
	class TriangleMeshData;
	struct ConvexHullData;
}

class GuMeshFactoryListener
{
protected:
	virtual ~GuMeshFactoryListener(){}
public:
	virtual void onGuMeshFactoryBufferRelease(const PxBase* object, PxType type) = 0;
};

#if PX_VC 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
class PX_PHYSX_COMMON_API GuMeshFactory : public Ps::UserAllocated
{
	PX_NOCOPY(GuMeshFactory)
public:
									GuMeshFactory();
protected:
	virtual							~GuMeshFactory();

public:
	void							release();

	// Triangle mehes
	void							addTriangleMesh(Gu::TriangleMesh* np, bool lock=true);
	PxTriangleMesh*					createTriangleMesh(PxInputStream& stream);
	PxTriangleMesh*					createTriangleMesh(void* triangleMeshData);
	bool							removeTriangleMesh(PxTriangleMesh&);
	PxU32							getNbTriangleMeshes()	const;
	PxU32							getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

	// Convexes
	void							addConvexMesh(Gu::ConvexMesh* np, bool lock=true);
	PxConvexMesh*					createConvexMesh(PxInputStream&);
	PxConvexMesh*					createConvexMesh(void* convexMeshData);
	bool							removeConvexMesh(PxConvexMesh&);
	PxU32							getNbConvexMeshes() const;
	PxU32							getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

	// Heightfields
	void							addHeightField(Gu::HeightField* np, bool lock=true);
	PxHeightField*					createHeightField(void* heightFieldMeshData);
	PxHeightField*					createHeightField(PxInputStream&);
	bool							removeHeightField(PxHeightField&);
	PxU32							getNbHeightFields()	const;
	PxU32							getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

	void							addFactoryListener( GuMeshFactoryListener& listener );
	void							removeFactoryListener( GuMeshFactoryListener& listener );
	void							notifyFactoryListener(const PxBase*, PxType typeID);

protected:

	PxTriangleMesh*					createTriangleMesh(Gu::TriangleMeshData& data);
	PxConvexMesh*					createConvexMesh(Gu::ConvexHullData& data);

#if PX_SUPPORT_GPU_PHYSX
	virtual void					notifyReleaseTriangleMesh(const PxTriangleMesh&) {}
	virtual void					notifyReleaseHeightField(const PxHeightField&) {}
	virtual void					notifyReleaseConvexMesh(const PxConvexMesh&) {}
#endif

	mutable Ps::Mutex				mTrackingMutex;
private:
	Ps::CoalescedHashSet<Gu::TriangleMesh*>	mTriangleMeshes;
	Ps::CoalescedHashSet<Gu::ConvexMesh*>	mConvexMeshes;
	Ps::CoalescedHashSet<Gu::HeightField*>	mHeightFields;

	Ps::Array<GuMeshFactoryListener*>		mFactoryListeners;
};
#if PX_VC 
     #pragma warning(pop) 
#endif
}

#endif
