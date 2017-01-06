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

#include "foundation/PxVec4.h"
#include "Allocator.h"
#include "Fabric.h"
#include "Types.h"
#include "Range.h"

namespace physx
{

namespace cloth
{

class SwFactory;

struct SwTether
{
	SwTether(uint16_t, float);
	uint16_t mAnchor;
	float mLength;
};

class SwFabric : public UserAllocated, public Fabric
{
  public:
#if PX_WINDOWS
	typedef AlignedVector<float, 32>::Type RestvalueContainer; // avx
#else
	typedef AlignedVector<float, 16>::Type RestvalueContainer;
#endif

	SwFabric(SwFactory& factory, uint32_t numParticles, Range<const uint32_t> phases, Range<const uint32_t> sets,
	         Range<const float> restvalues, Range<const uint32_t> indices, Range<const uint32_t> anchors,
	         Range<const float> tetherLengths, Range<const uint32_t> triangles, uint32_t id);

	SwFabric& operator=(const SwFabric&);

	virtual ~SwFabric();

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
	SwFactory& mFactory;

	uint32_t mNumParticles;

	Vector<uint32_t>::Type mPhases; // index of set to use
	Vector<uint32_t>::Type mSets;   // offset of first restvalue, with 0 prefix

	RestvalueContainer mRestvalues;  // rest values (edge length)
	Vector<uint16_t>::Type mIndices; // particle index pairs

	Vector<SwTether>::Type mTethers;
	float mTetherLengthScale;

	Vector<uint16_t>::Type mTriangles;

	uint32_t mId;

	uint32_t mOriginalNumRestvalues;

} PX_ALIGN_SUFFIX(16);
}
}
