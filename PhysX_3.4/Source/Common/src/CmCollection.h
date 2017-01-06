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


#ifndef PX_PHYSICS_CM_COLLECTION
#define PX_PHYSICS_CM_COLLECTION

#include "CmPhysXCommon.h"
#include "PxCollection.h"
#include "PsHashMap.h"
#include "PsUserAllocated.h"
#include "PsAllocator.h"

namespace physx
{
namespace Cm
{	
	template <class Key, 
			  class Value,
			  class HashFn = Ps::Hash<Key>, 
			  class Allocator = Ps::NonTrackingAllocator >
	class CollectionHashMap : public Ps::CoalescedHashMap< Key, Value, HashFn, Allocator>
	{
		typedef physx::shdfnd::internal::HashMapBase< Key, Value, HashFn, Allocator> MapBase;	
		typedef Ps::Pair<const Key,Value> EntryData;

		public:
			CollectionHashMap(PxU32 initialTableSize = 64, float loadFactor = 0.75f):
			    Ps::CoalescedHashMap< Key, Value, HashFn, Allocator>(initialTableSize,loadFactor) {}

			void insertUnique(const Key& k, const Value& v)
			{
				PX_PLACEMENT_NEW(MapBase::mBase.insertUnique(k), EntryData)(k,v);
			}
	};

	
	
	class Collection : public PxCollection, public Ps::UserAllocated
	{
	public:
		typedef CollectionHashMap<PxBase*, PxSerialObjectId> ObjectToIdMap;
		typedef CollectionHashMap<PxSerialObjectId, PxBase*> IdToObjectMap;
					
		virtual void						add(PxBase& object, PxSerialObjectId ref);
		virtual	void						remove(PxBase& object);	
		virtual bool						contains(PxBase& object) const;
		virtual void						addId(PxBase& object, PxSerialObjectId id);
		virtual void						removeId(PxSerialObjectId id);
		virtual PxBase*						find(PxSerialObjectId ref) const;
		virtual void						add(PxCollection& collection);
		virtual void						remove(PxCollection& collection);		
		virtual	PxU32						getNbObjects() const;
		virtual PxBase&						getObject(PxU32 index) const;
		virtual	PxU32						getObjects(PxBase** userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

		virtual PxU32						getNbIds() const;		
		virtual PxSerialObjectId			getId(const PxBase& object) const;
		virtual	PxU32						getIds(PxSerialObjectId* userBuffer, PxU32 bufferSize, PxU32 startIndex=0) const;

		void								release() { PX_DELETE(this); }


		// Only for internal use. Bypasses virtual calls, specialized behaviour.
		PX_INLINE	void		            internalAdd(PxBase* s, PxSerialObjectId id = PX_SERIAL_OBJECT_ID_INVALID)				{ mObjects.insertUnique(s, id);	                   }
		PX_INLINE	PxU32		            internalGetNbObjects()		 const	{ return mObjects.size();								               }
		PX_INLINE	PxBase*		            internalGetObject(PxU32 i)	 const	{ PX_ASSERT(i<mObjects.size());	return mObjects.getEntries()[i].first; }
		PX_INLINE	const ObjectToIdMap::Entry*	internalGetObjects() const  { return mObjects.getEntries(); 			                           }
			
		IdToObjectMap					    mIds;
		ObjectToIdMap                       mObjects;
		
	};
}
}

#endif
