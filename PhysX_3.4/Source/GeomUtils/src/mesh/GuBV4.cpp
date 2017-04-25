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

#include "foundation/PxMemory.h"
#include "GuBV4.h"
#include "GuSerialize.h"
#include "CmUtils.h"
#include "PsUtilities.h"

using namespace physx;
using namespace Gu;

#define DELETEARRAY(x)		if (x) { delete []x;	x = NULL; }

SourceMesh::SourceMesh()
{
	reset();
}

SourceMesh::~SourceMesh()
{
	PX_FREE_AND_RESET(mRemap);
}

void SourceMesh::reset()
{
	mNbVerts		= 0;
	mVerts			= NULL;
	mNbTris			= 0;
	mTriangles32	= NULL;
	mTriangles16	= NULL;
	mRemap			= NULL;
}

void SourceMesh::operator=(SourceMesh& v)
{
	mNbVerts		= v.mNbVerts;
	mVerts			= v.mVerts;
	mNbTris			= v.mNbTris;
	mTriangles32	= v.mTriangles32;
	mTriangles16	= v.mTriangles16;
	mRemap			= v.mRemap;
	v.reset();
}

void SourceMesh::remapTopology(const PxU32* order)
{
	if(!mNbTris)
		return;

	if(mTriangles32)
	{
		IndTri32* newTopo = PX_NEW(IndTri32)[mNbTris];
		for(PxU32 i=0;i<mNbTris;i++)
			newTopo[i] = mTriangles32[order[i]];

		PxMemCopy(mTriangles32, newTopo, sizeof(IndTri32)*mNbTris);
		DELETEARRAY(newTopo);
	}
	else
	{
		PX_ASSERT(mTriangles16);
		IndTri16* newTopo = PX_NEW(IndTri16)[mNbTris];
		for(PxU32 i=0;i<mNbTris;i++)
			newTopo[i] = mTriangles16[order[i]];

		PxMemCopy(mTriangles16, newTopo, sizeof(IndTri16)*mNbTris);
		DELETEARRAY(newTopo);
	}

	{
		PxU32* newMap = reinterpret_cast<PxU32*>(PX_ALLOC(sizeof(PxU32)*mNbTris, "OPC2"));
		for(PxU32 i=0;i<mNbTris;i++)
			newMap[i] = mRemap ? mRemap[order[i]] : order[i];

		PX_FREE_AND_RESET(mRemap);
		mRemap = newMap;
	}
}

bool SourceMesh::isValid() const
{
	if(!mNbTris || !mNbVerts)			return false;
	if(!mVerts)							return false;
	if(!mTriangles32 && !mTriangles16)	return false;
	return true;
}

/////

BV4Tree::BV4Tree(SourceMesh* meshInterface, const PxBounds3& localBounds)
{
	reset();
	init(meshInterface, localBounds);
}

BV4Tree::BV4Tree()
{
	reset();
}

void BV4Tree::release()
{
	if(!mUserAllocated)
	{
#ifdef GU_BV4_USE_SLABS
		PX_DELETE_AND_RESET(mNodes);
#else
		DELETEARRAY(mNodes);
#endif
	}

	mNodes = NULL;
	mNbNodes = 0;
}

BV4Tree::~BV4Tree()
{
	release();
}

void BV4Tree::reset()
{
	mMeshInterface		= NULL;
	mNbNodes			= 0;
	mNodes				= NULL;
	mInitData			= 0;
#ifdef GU_BV4_QUANTIZED_TREE
	mCenterOrMinCoeff	= PxVec3(0.0f);
	mExtentsOrMaxCoeff	= PxVec3(0.0f);
#endif
	mUserAllocated		= false;
}

void BV4Tree::operator=(BV4Tree& v)
{
	mMeshInterface		= v.mMeshInterface;
	mLocalBounds		= v.mLocalBounds;
	mNbNodes			= v.mNbNodes;
	mNodes				= v.mNodes;
	mInitData			= v.mInitData;
#ifdef GU_BV4_QUANTIZED_TREE
	mCenterOrMinCoeff	= v.mCenterOrMinCoeff;
	mExtentsOrMaxCoeff	= v.mExtentsOrMaxCoeff;
#endif
	mUserAllocated		= v.mUserAllocated;
	v.reset();
}

bool BV4Tree::init(SourceMesh* meshInterface, const PxBounds3& localBounds)
{
	mMeshInterface	= meshInterface;
	mLocalBounds.init(localBounds);
	return true;
}

// PX_SERIALIZATION
BV4Tree::BV4Tree(const PxEMPTY)
{
	mUserAllocated = true;
}

void BV4Tree::exportExtraData(PxSerializationContext& stream)
{
	if(mNbNodes)
	{
		stream.alignData(16);
		stream.writeData(mNodes, mNbNodes*sizeof(BVDataPacked));
	}
}

void BV4Tree::importExtraData(PxDeserializationContext& context)
{
	if(mNbNodes)
	{
		context.alignExtraData(16);
		mNodes = context.readExtraData<BVDataPacked>(mNbNodes);
	}
}
//~PX_SERIALIZATION

bool BV4Tree::load(PxInputStream& stream, PxU32 meshVersion)
{
	PX_ASSERT(!mUserAllocated);
	PX_UNUSED(meshVersion);

	release();

	PxI8 a, b, c, d;
	readChunk(a, b, c, d, stream);
	if(a!='B' || b!='V' || c!='4' || d!=' ')
		return false;

	const PxU32 version = 1;
	const bool mismatch = (shdfnd::littleEndian() == 1);
	if(readDword(mismatch, stream) != version)
		return false;

	mLocalBounds.mCenter.x = readFloat(mismatch, stream);
	mLocalBounds.mCenter.y = readFloat(mismatch, stream);
	mLocalBounds.mCenter.z = readFloat(mismatch, stream);
	mLocalBounds.mExtentsMagnitude = readFloat(mismatch, stream);

	mInitData = readDword(mismatch, stream);

#ifdef GU_BV4_QUANTIZED_TREE
	mCenterOrMinCoeff.x = readFloat(mismatch, stream);
	mCenterOrMinCoeff.y = readFloat(mismatch, stream);
	mCenterOrMinCoeff.z = readFloat(mismatch, stream);
	mExtentsOrMaxCoeff.x = readFloat(mismatch, stream);
	mExtentsOrMaxCoeff.y = readFloat(mismatch, stream);
	mExtentsOrMaxCoeff.z = readFloat(mismatch, stream);
#endif
	const PxU32 nbNodes = readDword(mismatch, stream);
	mNbNodes = nbNodes;

	if(nbNodes)
	{
#ifdef GU_BV4_USE_SLABS
		BVDataPacked* nodes = reinterpret_cast<BVDataPacked*>(PX_ALLOC(sizeof(BVDataPacked)*nbNodes, "BV4 nodes"));	// PT: PX_NEW breaks alignment here
#else
		BVDataPacked* nodes = PX_NEW(BVDataPacked)[nbNodes];
#endif
		mNodes = nodes;
		Cm::markSerializedMem(nodes, sizeof(BVDataPacked)*nbNodes);

		for(PxU32 i=0;i<nbNodes;i++)
		{
			BVDataPacked& node = nodes[i];
#ifdef GU_BV4_QUANTIZED_TREE
			readWordBuffer(&node.mAABB.mData[0].mExtents, 6, mismatch, stream);
#else
			readFloatBuffer(&node.mAABB.mCenter.x, 6, mismatch, stream);
#endif
			node.mData = readDword(mismatch, stream);
		}
	}
	else mNodes = NULL;

	return true;
}
