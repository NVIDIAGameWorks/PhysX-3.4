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

#ifndef PT_PARTICLE_CONTACT_MANAGER_STREAM_H
#define PT_PARTICLE_CONTACT_MANAGER_STREAM_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

namespace physx
{

struct PxsRigidCore;

namespace Pt
{

class ParticleShape;

struct ParticleStreamContactManager
{
	const PxsRigidCore* rigidCore;
	const PxsShapeCore* shapeCore;
	const PxTransform* w2sOld;
	bool isDrain;
	bool isDynamic;
};

struct ParticleStreamShape
{
	const ParticleShape* particleShape;
	PxU32 numContactManagers;
	const ParticleStreamContactManager* contactManagers;
};

class ParticleContactManagerStreamWriter
{
  public:
	static PxU32 getStreamSize(PxU32 numParticleShapes, PxU32 numContactManagers)
	{
		return sizeof(PxU32) +                                                // particle shape count
		       sizeof(PxU32) +                                                // stream size
		       numParticleShapes * (sizeof(ParticleShape*) + sizeof(PxU32)) + // shape ll pointer + contact manager
		                                                                      // count per shape
		       numContactManagers * sizeof(ParticleStreamContactManager);     // contact manager data
	}

	PX_FORCE_INLINE ParticleContactManagerStreamWriter(PxU8* stream, PxU32 numParticleShapes, PxU32 numContactManagers)
	: mStream(stream), mNumContactsPtr(NULL)
	{
		PX_ASSERT(mStream);
		*reinterpret_cast<PxU32*>(mStream) = numParticleShapes;
		mStream += sizeof(PxU32);

		*reinterpret_cast<PxU32*>(mStream) = getStreamSize(numParticleShapes, numContactManagers);
		mStream += sizeof(PxU32);
	}

	PX_FORCE_INLINE void addParticleShape(const ParticleShape* particleShape)
	{
		*reinterpret_cast<const ParticleShape**>(mStream) = particleShape;
		mStream += sizeof(const ParticleShape*);

		mNumContactsPtr = reinterpret_cast<PxU32*>(mStream);
		mStream += sizeof(PxU32);

		*mNumContactsPtr = 0;
	}

	PX_FORCE_INLINE void addContactManager(const PxsRigidCore* rigidCore, const PxsShapeCore* shapeCore,
	                                       const PxTransform* w2sOld, bool isDrain, bool isDynamic)
	{
		ParticleStreamContactManager& cm = *reinterpret_cast<ParticleStreamContactManager*>(mStream);
		mStream += sizeof(ParticleStreamContactManager);

		cm.rigidCore = rigidCore;
		cm.shapeCore = shapeCore;
		cm.w2sOld = w2sOld;
		cm.isDrain = isDrain;
		cm.isDynamic = isDynamic;

		PX_ASSERT(mNumContactsPtr);
		(*mNumContactsPtr)++;
	}

  private:
	PxU8* mStream;
	PxU32* mNumContactsPtr;
};

class ParticleContactManagerStreamIterator
{
  public:
	PX_FORCE_INLINE ParticleContactManagerStreamIterator()
	{
	}
	PX_FORCE_INLINE ParticleContactManagerStreamIterator(const PxU8* stream) : mStream(stream)
	{
	}

	ParticleContactManagerStreamIterator getNext(ParticleStreamShape& next)
	{
		const ParticleShape* const* tmp0 = reinterpret_cast<const ParticleShape* const*>(mStream);
		mStream += sizeof(ParticleShape*);
		const PxU32* tmp1 = reinterpret_cast<const PxU32*>(mStream);
		next.particleShape = *tmp0;
		next.numContactManagers = *tmp1;
		mStream += sizeof(PxU32);

		next.contactManagers = reinterpret_cast<const ParticleStreamContactManager*>(mStream);
		mStream += next.numContactManagers * sizeof(ParticleStreamContactManager);

		return ParticleContactManagerStreamIterator(mStream);
	}

	PX_FORCE_INLINE ParticleContactManagerStreamIterator getNext()
	{
		mStream += sizeof(ParticleShape*);
		PxU32 numContactManagers = *reinterpret_cast<const PxU32*>(mStream);

		mStream += sizeof(PxU32);
		mStream += numContactManagers * sizeof(ParticleStreamContactManager);
		return ParticleContactManagerStreamIterator(mStream);
	}

	PX_FORCE_INLINE bool operator==(const ParticleContactManagerStreamIterator& it)
	{
		return mStream == it.mStream;
	}

	PX_FORCE_INLINE bool operator!=(const ParticleContactManagerStreamIterator& it)
	{
		return mStream != it.mStream;
	}

	PX_FORCE_INLINE const PxU8* getStream()
	{
		return mStream;
	}

  private:
	friend class ParticleContactManagerStreamReader;

  private:
	const PxU8* mStream;
};

class ParticleContactManagerStreamReader
{
  public:
	/*
	Reads header of stream consisting of shape count and stream end pointer
	*/
	PX_FORCE_INLINE ParticleContactManagerStreamReader(const PxU8* stream)
	{
		PX_ASSERT(stream);
		mStreamDataBegin = stream;
		mNumParticleShapes = *reinterpret_cast<const PxU32*>(mStreamDataBegin);
		mStreamDataBegin += sizeof(PxU32);
		PxU32 streamSize = *reinterpret_cast<const PxU32*>(mStreamDataBegin);
		mStreamDataBegin += sizeof(PxU32);
		mStreamDataEnd = stream + streamSize;
	}

	PX_FORCE_INLINE ParticleContactManagerStreamIterator getBegin() const
	{
		return ParticleContactManagerStreamIterator(mStreamDataBegin);
	}
	PX_FORCE_INLINE ParticleContactManagerStreamIterator getEnd() const
	{
		return ParticleContactManagerStreamIterator(mStreamDataEnd);
	}
	PX_FORCE_INLINE PxU32 getNumParticleShapes() const
	{
		return mNumParticleShapes;
	}

  private:
	PxU32 mNumParticleShapes;
	const PxU8* mStreamDataBegin;
	const PxU8* mStreamDataEnd;
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_PARTICLE_CONTACT_MANAGER_STREAM_H
