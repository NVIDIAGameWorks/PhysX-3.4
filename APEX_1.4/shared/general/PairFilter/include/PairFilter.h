/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PAIR_FILTER_H
#define PAIR_FILTER_H

#include "PxSimpleTypes.h"
#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"
#include "PsHashSet.h"

#ifndef NULL
#define NULL 0
#endif

namespace nvidia
{

using namespace shdfnd;

class userPair;

class HashKey
{
public:
	HashKey(void)
	{

	}
	// Must always be created in sorted order!
	HashKey(uint64_t _id0,uint64_t _id1)
	{
		if ( _id0 < _id1 )
		{
			id0 = _id0;
			id1 = _id1;
		}
		else
		{
			id0 = _id1;
			id1 = _id0;
		}
	}

	uint32_t hash(void) const
	{
		const uint32_t *h1 = (const uint32_t *)&id0;
		const uint32_t *h2 = (const uint32_t *)&id1;
		uint32_t ret = h1[0];
		ret^=h1[1];
		ret^=h2[0];
		ret^=h2[1];
		return ret;
	}

	bool keyEquals(const HashKey &k) const
	{
		return id0 == k.id0 && id1 == k.id1;
	}

	uint64_t	id0;
	uint64_t	id1;
};

struct HashKeyHash
{
	PX_FORCE_INLINE uint32_t operator()(const HashKey& k) const { return k.hash(); }
	PX_FORCE_INLINE bool equal(const HashKey& k0, const HashKey& k1) const { return k0.keyEquals(k1); }
};

class PairFilter : public UserAllocated
{
public:

	typedef HashSet< HashKey, HashKeyHash > PairMap;

	PairFilter(void)
	{

	}

	~PairFilter(void)
	{

	}

	void	purge(void)
	{
		mPairMap.clear();
	}

	void	addPair(uint64_t id0, uint64_t id1)
	{
		HashKey k(id0,id1);
		mPairMap.insert(k);
	}

	bool	removePair(uint64_t id0, uint64_t id1)
	{
		bool ret = false;
		PX_UNUSED(id0);
		PX_UNUSED(id1);
		HashKey k(id0,id1);
		if ( mPairMap.contains(k) )
		{
			ret = true;
			mPairMap.erase(k);
		}
		return ret;
	}

	bool	findPair(uint64_t id0,uint64_t id1)	const
	{
		HashKey k(id0,id1);
		return mPairMap.contains(k);
	}


private:
	PairMap		mPairMap;
};



};// end of namespace

#endif
