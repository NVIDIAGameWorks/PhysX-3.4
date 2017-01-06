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

#include "Types.h"
#include "PxAssert.h"
#include "Range.h"

namespace nvidia
{
namespace cloth
{

class Factory;

// abstract cloth constraints and triangle indices
class Fabric
{
  protected:
	Fabric(const Fabric&);
	Fabric& operator=(const Fabric&);

  protected:
	Fabric() : mRefCount(0)
	{
	}

  public:
	virtual ~Fabric()
	{
		PX_ASSERT(!mRefCount);
	}

	virtual Factory& getFactory() const = 0;

	virtual uint32_t getNumPhases() const = 0;
	virtual uint32_t getNumRestvalues() const = 0;

	virtual uint32_t getNumSets() const = 0;
	virtual uint32_t getNumIndices() const = 0;

	virtual uint32_t getNumParticles() const = 0;

	virtual uint32_t getNumTethers() const = 0;

	virtual void scaleRestvalues(float) = 0;
	virtual void scaleTetherLengths(float) = 0;

	uint16_t getRefCount() const
	{
		return mRefCount;
	}
	void incRefCount()
	{
		++mRefCount;
		PX_ASSERT(mRefCount > 0);
	}
	void decRefCount()
	{
		PX_ASSERT(mRefCount > 0);
		--mRefCount;
	}

  protected:
	uint16_t mRefCount;
};

} // namespace cloth
} // namespace nvidia
