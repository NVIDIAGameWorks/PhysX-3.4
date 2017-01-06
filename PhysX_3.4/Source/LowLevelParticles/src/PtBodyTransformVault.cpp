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

#include "PtBodyTransformVault.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "foundation/PxMemory.h"
#include "PxvGeometry.h"
#include "PxvDynamics.h"
#include "PsHash.h"
#include "PsFoundation.h"

using namespace physx;
using namespace Pt;

BodyTransformVault::BodyTransformVault() : mBody2WorldPool("body2WorldPool", 256), mBodyCount(0)
{
	// Make sure the hash size is a power of 2
	PX_ASSERT((((PT_BODY_TRANSFORM_HASH_SIZE - 1) ^ PT_BODY_TRANSFORM_HASH_SIZE) + 1) ==
	          (2 * PT_BODY_TRANSFORM_HASH_SIZE));

	PxMemSet(mBody2WorldHash, 0, PT_BODY_TRANSFORM_HASH_SIZE * sizeof(Body2World*));
}

BodyTransformVault::~BodyTransformVault()
{
}

PX_FORCE_INLINE PxU32 BodyTransformVault::getHashIndex(const PxsBodyCore& body) const
{
	PxU32 index = Ps::hash(&body);
	return (index & (PT_BODY_TRANSFORM_HASH_SIZE - 1)); // Modulo hash size
}

void BodyTransformVault::addBody(const PxsBodyCore& body)
{
	Body2World* entry;
	Body2World* dummy;

	bool hasEntry = findEntry(body, entry, dummy);
	if(!hasEntry)
	{
		Body2World* newEntry;
		if(entry)
		{
			// No entry for the given body but the hash entry has other bodies
			// --> create new entry, link into list
			newEntry = createEntry(body);
			entry->next = newEntry;
		}
		else
		{
			// No entry for the given body and no hash entry --> create new entry
			PxU32 hashIndex = getHashIndex(body);
			newEntry = createEntry(body);
			mBody2WorldHash[hashIndex] = newEntry;
		}
		newEntry->refCount = 1;
		mBodyCount++;
	}
	else
	{
		entry->refCount++;
	}
}

void BodyTransformVault::removeBody(const PxsBodyCore& body)
{
	Body2World* entry;
	Body2World* prevEntry;

	bool hasEntry = findEntry(body, entry, prevEntry);
	PX_ASSERT(hasEntry);
	PX_UNUSED(hasEntry);

	if(entry->refCount == 1)
	{
		if(prevEntry)
		{
			prevEntry->next = entry->next;
		}
		else
		{
			// Shape entry was first in list
			PxU32 hashIndex = getHashIndex(body);

			mBody2WorldHash[hashIndex] = entry->next;
		}
		mBody2WorldPool.destroy(entry);
		PX_ASSERT(mBodyCount > 0);
		mBodyCount--;
	}
	else
	{
		entry->refCount--;
	}
}

void BodyTransformVault::teleportBody(const PxsBodyCore& body)
{
	Body2World* entry;
	Body2World* dummy;

	bool hasEntry = findEntry(body, entry, dummy);
	PX_ASSERT(hasEntry);
	PX_ASSERT(entry);
	PX_UNUSED(hasEntry);

	PX_CHECK_AND_RETURN(body.body2World.isValid(), "BodyTransformVault::teleportBody: body.body2World is not valid.");

	entry->b2w = body.body2World;
}

const PxTransform* BodyTransformVault::getTransform(const PxsBodyCore& body) const
{
	Body2World* entry;
	Body2World* dummy;

	bool hasEntry = findEntry(body, entry, dummy);
	// PX_ASSERT(hasEntry);
	// PX_UNUSED(hasEntry);
	// PX_ASSERT(entry);
	return hasEntry ? &entry->b2w : NULL;
}

void BodyTransformVault::update()
{
	if(mBodyCount)
	{
		for(PxU32 i = 0; i < PT_BODY_TRANSFORM_HASH_SIZE; i++)
		{
			Body2World* entry = mBody2WorldHash[i];

			while(entry)
			{
				PX_ASSERT(entry->body);
				entry->b2w = entry->body->body2World;
				entry = entry->next;
			}
		}
	}
}

BodyTransformVault::Body2World* BodyTransformVault::createEntry(const PxsBodyCore& body)
{
	Body2World* entry = mBody2WorldPool.construct();

	if(entry)
	{
		entry->b2w = body.body2World;
		entry->next = NULL;
		entry->body = &body;
	}

	return entry;
}

bool BodyTransformVault::isInVaultInternal(const PxsBodyCore& body) const
{
	PxU32 hashIndex = getHashIndex(body);

	if(mBody2WorldHash[hashIndex])
	{
		Body2World* curEntry = mBody2WorldHash[hashIndex];

		while(curEntry->next)
		{
			if(curEntry->body == &body)
				break;

			curEntry = curEntry->next;
		}

		if(curEntry->body == &body)
			return true;
	}

	return false;
}

bool BodyTransformVault::findEntry(const PxsBodyCore& body, Body2World*& entry, Body2World*& prevEntry) const
{
	PxU32 hashIndex = getHashIndex(body);

	prevEntry = NULL;
	bool hasEntry = false;
	if(mBody2WorldHash[hashIndex])
	{
		Body2World* curEntry = mBody2WorldHash[hashIndex];

		while(curEntry->next)
		{
			if(curEntry->body == &body)
				break;

			prevEntry = curEntry;
			curEntry = curEntry->next;
		}

		entry = curEntry;
		if(curEntry->body == &body)
		{
			// An entry already exists for the given body
			hasEntry = true;
		}
	}
	else
	{
		entry = NULL;
	}

	return hasEntry;
}

#endif // PX_USE_PARTICLE_SYSTEM_API
