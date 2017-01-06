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


#ifndef PXD_THRESHOLDTABLE_H
#define PXD_THRESHOLDTABLE_H
#include "Ps.h"
#include "PsArray.h"
#include "CmPhysXCommon.h"
#include "PsAllocator.h"
#include "PsHash.h"
#include "foundation/PxMemory.h"

namespace physx
{

class PxsRigidBody;

namespace Sc
{
	class ShapeInteraction;
}

namespace Dy
{

struct ThresholdStreamElement
{
	Sc::ShapeInteraction*	shapeInteraction;			//4		8
	PxReal					normalForce;				//8		12
	PxReal					threshold;					//12	16
	PxU32					nodeIndexA; //this is the unique node index in island gen which corresonding to that body and it is persistent	16	20
	PxU32					nodeIndexB; //This is the unique node index in island gen which corresonding to that body and it is persistent	20	24
	PxReal					accumulatedForce;			//24	28
	PxU32					pad;						//28	32

#if !PX_P64_FAMILY
	PxU32					pad1;						//32
#endif // !PX_X64

	PX_CUDA_CALLABLE bool operator <= (const ThresholdStreamElement& otherPair) const
	{
		return ((nodeIndexA < otherPair.nodeIndexA) ||(nodeIndexA == otherPair.nodeIndexA && nodeIndexB <= otherPair.nodeIndexB));
	}

};

typedef Ps::Array<ThresholdStreamElement, Ps::VirtualAllocator> ThresholdArray;

class ThresholdStream : public ThresholdArray
{
public:
	ThresholdStream(Ps::VirtualAllocatorCallback& allocatorCallback) : ThresholdArray(Ps::VirtualAllocator(&allocatorCallback))
	{
	}

};

class ThresholdTable
{
public:

	ThresholdTable()
		:	mBuffer(NULL),
			mHash(NULL),
			mHashSize(0),
			mHashCapactiy(0),
			mPairs(NULL),
			mNexts(NULL),
			mPairsSize(0),
			mPairsCapacity(0)
	{
	}

	~ThresholdTable()
	{
		if(mBuffer) PX_FREE(mBuffer);
	}

	void build(const ThresholdStream& stream);

	bool check(const ThresholdStream& stream, const PxU32 nodexIndexA, const PxU32 nodexIndexB, PxReal dt);

	bool check(const ThresholdStream& stream, const ThresholdStreamElement& elem, PxU32& thresholdIndex);

//private:

	static const PxU32 NO_INDEX = 0xffffffff;

	struct Pair 
	{	
		PxU32			thresholdStreamIndex;
		PxReal			accumulatedForce;
		//PxU32			next;		// hash key & next ptr
	};

	PxU8*					mBuffer;

	PxU32*					mHash;
	PxU32					mHashSize;
	PxU32					mHashCapactiy;

	Pair*					mPairs;
	PxU32*					mNexts;
	PxU32					mPairsSize;
	PxU32					mPairsCapacity;
};

namespace
{
	static PX_FORCE_INLINE PxU32 computeHashKey(const PxU32 nodeIndexA, const PxU32 nodeIndexB, const PxU32 hashCapacity)
	{
		return (Ps::hash(PxU64(nodeIndexA)<<32 | PxU64(nodeIndexB)) % hashCapacity);
	}
}

inline bool ThresholdTable::check(const ThresholdStream& stream, const ThresholdStreamElement& elem, PxU32& thresholdIndex)
{
	PxU32* PX_RESTRICT hashes = mHash;
	PxU32* PX_RESTRICT nextIndices = mNexts;
	Pair* PX_RESTRICT pairs = mPairs;

	PX_ASSERT(elem.nodeIndexA < elem.nodeIndexB);
	PxU32 hashKey = computeHashKey(elem.nodeIndexA, elem.nodeIndexB, mHashSize);

	PxU32 pairIndex = hashes[hashKey];

	while(NO_INDEX != pairIndex)
	{
		Pair& pair = pairs[pairIndex];
		const PxU32 thresholdStreamIndex = pair.thresholdStreamIndex;
		PX_ASSERT(thresholdStreamIndex < stream.size());
		const ThresholdStreamElement& otherElement = stream[thresholdStreamIndex];
		if(otherElement.nodeIndexA==elem.nodeIndexA  && otherElement.nodeIndexB==elem.nodeIndexB && otherElement.shapeInteraction == elem.shapeInteraction)
		{
			thresholdIndex = thresholdStreamIndex;
			return true;
		}
		pairIndex = nextIndices[pairIndex];
	}

	thresholdIndex = NO_INDEX;
	return false;
}


inline void ThresholdTable::build(const ThresholdStream& stream)
{
	//Handle the case of an empty stream.
	if(0==stream.size())
	{
		mPairsSize=0;
		mPairsCapacity=0;
		mHashSize=0;
		mHashCapactiy=0;
		if(mBuffer) PX_FREE(mBuffer);
		mBuffer = NULL;
		return;
	}

	//Realloc/resize if necessary.
	const PxU32 pairsCapacity = stream.size();
	const PxU32 hashCapacity = pairsCapacity*2+1;
	if((pairsCapacity > mPairsCapacity) || (pairsCapacity < (mPairsCapacity >> 2)))
	{
		if(mBuffer) PX_FREE(mBuffer);
		const PxU32 pairsByteSize = sizeof(Pair)*pairsCapacity;
		const PxU32 nextsByteSize = sizeof(PxU32)*pairsCapacity;
		const PxU32 hashByteSize = sizeof(PxU32)*hashCapacity;
		const PxU32 totalByteSize = pairsByteSize + nextsByteSize + hashByteSize;
		mBuffer = reinterpret_cast<PxU8*>(PX_ALLOC(totalByteSize, "PxThresholdStream"));

		PxU32 offset = 0;
		mPairs = reinterpret_cast<Pair*>(mBuffer + offset);
		offset += pairsByteSize;
		mNexts = reinterpret_cast<PxU32*>(mBuffer + offset);
		offset += nextsByteSize;
		mHash = reinterpret_cast<PxU32*>(mBuffer + offset);
		offset += hashByteSize;
		PX_ASSERT(totalByteSize == offset);

		mPairsCapacity = pairsCapacity;
		mHashCapactiy = hashCapacity;
	}


	//Set each entry of the hash table to 0xffffffff
	PxMemSet(mHash, 0xff, sizeof(PxU32)*hashCapacity);

	//Init the sizes of the pairs array and hash array.
	mPairsSize = 0;
	mHashSize = hashCapacity;

	PxU32* PX_RESTRICT hashes = mHash;
	PxU32* PX_RESTRICT nextIndices = mNexts;
	Pair* PX_RESTRICT pairs = mPairs;

	//Add all the pairs from the stream.
	PxU32 pairsSize = 0;
	for(PxU32 i = 0; i < pairsCapacity; i++)
	{
		const ThresholdStreamElement& element = stream[i];
		const PxU32 nodeIndexA = element.nodeIndexA;
		const PxU32 nodeIndexB = element.nodeIndexB;

		const PxF32 force = element.normalForce;
				
		PX_ASSERT(nodeIndexA < nodeIndexB);

		const PxU32 hashKey = computeHashKey(nodeIndexA, nodeIndexB, hashCapacity);

		//Get the index of the first pair found that resulted in a hash that matched hashKey.
		PxU32 prevPairIndex = hashKey;
		PxU32 pairIndex = hashes[hashKey];

		//Search through all pairs found that resulted in a hash that matched hashKey.
		//Search until the exact same body pair is found.
		//Increment the accumulated force if the exact same body pair is found.
		while(NO_INDEX != pairIndex)
		{
			Pair& pair = pairs[pairIndex];
			const PxU32 thresholdStreamIndex = pair.thresholdStreamIndex;
			PX_ASSERT(thresholdStreamIndex < stream.size());
			const ThresholdStreamElement& otherElement = stream[thresholdStreamIndex];
			if(nodeIndexA == otherElement.nodeIndexA && nodeIndexB==otherElement.nodeIndexB)
			{	
				pair.accumulatedForce += force;
				prevPairIndex = NO_INDEX;
				pairIndex = NO_INDEX;
				break;
			}
			prevPairIndex = pairIndex;
			pairIndex = nextIndices[pairIndex];
		}

		if(NO_INDEX != prevPairIndex)
		{
			nextIndices[pairsSize] = hashes[hashKey];
			hashes[hashKey] = pairsSize;
			Pair& newPair = pairs[pairsSize];
			newPair.thresholdStreamIndex = i;
			newPair.accumulatedForce = force;
			pairsSize++;
		}
	}
	mPairsSize = pairsSize;
}

}

}

#endif //DY_THRESHOLDTABLE_H
