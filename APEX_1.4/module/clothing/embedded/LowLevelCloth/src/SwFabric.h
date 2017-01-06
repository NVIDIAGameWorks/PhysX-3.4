/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#pragma once

#include "Allocator.h"
#include "Fabric.h"
#include "Types.h"
#include "Range.h"
#include "PxVec4.h"

namespace nvidia
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
#if PX_WINDOWS_FAMILY
	typedef AlignedVector<float, 32>::Type RestvalueContainer; // avx
#else
	typedef AlignedVector<float, 16>::Type RestvalueContainer;
#endif

	SwFabric(SwFactory& factory, uint32_t numParticles, Range<const uint32_t> phases, Range<const uint32_t> sets,
	         Range<const float> restvalues, Range<const uint32_t> indices, Range<const uint32_t> anchors,
	         Range<const float> tetherLengths, uint32_t id);

	SwFabric& operator=(const SwFabric&);

	virtual ~SwFabric();

	virtual Factory& getFactory() const;

	virtual uint32_t getNumPhases() const;
	virtual uint32_t getNumRestvalues() const;

	virtual uint32_t getNumSets() const;
	virtual uint32_t getNumIndices() const;

	virtual uint32_t getNumParticles() const;

	virtual uint32_t getNumTethers() const;

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

	uint32_t mId;

	uint32_t mOriginalNumRestvalues;

} PX_ALIGN_SUFFIX(16);
}
}
