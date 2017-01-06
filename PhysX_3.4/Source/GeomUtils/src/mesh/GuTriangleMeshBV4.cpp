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

#include "GuTriangleMesh.h"
#include "GuTriangleMeshBV4.h"

using namespace physx;

namespace physx
{

Gu::BV4TriangleMesh::BV4TriangleMesh(GuMeshFactory& factory, TriangleMeshData& d)
:	TriangleMesh(factory, d)
{
	PX_ASSERT(d.mType==PxMeshMidPhase::eBVH34);

	BV4TriangleData& bv4Data = static_cast<BV4TriangleData&>(d);
	mMeshInterface = bv4Data.mMeshInterface;
	mBV4Tree = bv4Data.mBV4Tree;
	mBV4Tree.mMeshInterface = &mMeshInterface;
}

Gu::TriangleMesh* Gu::BV4TriangleMesh::createObject(PxU8*& address, PxDeserializationContext& context)
{
	BV4TriangleMesh* obj = new (address) BV4TriangleMesh(PxBaseFlag::eIS_RELEASABLE);
	address += sizeof(BV4TriangleMesh);	
	obj->importExtraData(context);
	obj->resolveReferences(context);
	return obj;
}

void Gu::BV4TriangleMesh::exportExtraData(PxSerializationContext& stream)
{
	mBV4Tree.exportExtraData(stream);
	TriangleMesh::exportExtraData(stream);
}

void Gu::BV4TriangleMesh::importExtraData(PxDeserializationContext& context)
{
	mBV4Tree.importExtraData(context);
	TriangleMesh::importExtraData(context);

	if(has16BitIndices())
		mMeshInterface.setPointers(NULL, const_cast<IndTri16*>(reinterpret_cast<const IndTri16*>(getTrianglesFast())), getVerticesFast());
	else
		mMeshInterface.setPointers(const_cast<IndTri32*>(reinterpret_cast<const IndTri32*>(getTrianglesFast())), NULL, getVerticesFast());
	mBV4Tree.mMeshInterface = &mMeshInterface;
}

} // namespace physx
