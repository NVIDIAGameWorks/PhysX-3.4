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

#include "foundation/PxAssert.h"
#include "Types.h"
#include "Range.h"

namespace physx
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

	virtual uint32_t getNumTriangles() const = 0;

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
} // namespace physx
