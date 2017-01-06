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

#pragma once

#include "Fabric.h"
#include "Range.h"
#include "Types.h"
#include "Allocator.h"
#include "CuContextLock.h"
#include "CuDeviceVector.h"

namespace physx
{

namespace cloth
{

struct CuTether
{
	CuTether(uint16_t, uint16_t);
	uint16_t mAnchor;
	uint16_t mLength;
};

class CuFabric : public UserAllocated, private CuContextLock, public Fabric
{
	PX_NOCOPY(CuFabric)
  public:
	CuFabric(CuFactory& factory, uint32_t numParticles, Range<const uint32_t> phases, Range<const uint32_t> sets,
	         Range<const float> restvalues, Range<const uint32_t> indices, Range<const uint32_t> anchors,
	         Range<const float> tetherLengths, Range<const uint32_t> triangles, uint32_t id);

	virtual ~CuFabric();

	virtual Factory& getFactory() const;

	virtual uint32_t getNumPhases() const;
	virtual uint32_t getNumRestvalues() const;

	virtual uint32_t getNumSets() const;
	virtual uint32_t getNumIndices() const;

	virtual uint32_t getNumParticles() const;

	virtual uint32_t getNumTethers() const;

	virtual uint32_t getNumTriangles() const;

	virtual void scaleRestvalues(float);
	virtual void scaleTetherLengths(float);

  public:
	CuFactory& mFactory;

	uint32_t mNumParticles;

	CuDeviceVector<uint32_t> mPhases; // index of set to use
	CuDeviceVector<uint32_t> mSets;   // offset of first restvalue, with 0 prefix

	CuDeviceVector<float> mRestvalues;
	CuDeviceVector<uint16_t> mIndices;

	CuDeviceVector<CuTether> mTethers;
	float mTetherLengthScale;

	CuDeviceVector<uint16_t> mTriangles;

	Vector<uint32_t>::Type mNumConstraintsInPhase;
	Vector<CuDevicePointer<const float> >::Type mRestvaluesInPhase;
	Vector<CuDevicePointer<const uint16_t> >::Type mIndicesInPhase;

	uint32_t mId;
};
}
}
