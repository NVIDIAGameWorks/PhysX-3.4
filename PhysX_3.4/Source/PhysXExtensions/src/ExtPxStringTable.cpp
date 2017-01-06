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


#include "foundation/PxAllocatorCallback.h"
#include "PxStringTableExt.h"
#include "PxProfileAllocatorWrapper.h" //tools for using a custom allocator
#include "PsString.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"

namespace physx
{
	using namespace physx::profile;

	class PxStringTableImpl : public PxStringTable, public Ps::UserAllocated
	{
		typedef PxProfileHashMap<const char*, PxU32> THashMapType;
		PxProfileAllocatorWrapper mWrapper;
		THashMapType mHashMap;
	public:

		PxStringTableImpl( PxAllocatorCallback& inAllocator )
			: mWrapper ( inAllocator )
			, mHashMap ( mWrapper )
		{
		}

		virtual ~PxStringTableImpl()
		{
			for ( THashMapType::Iterator iter = mHashMap.getIterator();
				iter.done() == false;
				++iter )
				PX_PROFILE_DELETE( mWrapper, const_cast<char*>( iter->first ) );
			mHashMap.clear();
		}


		virtual const char* allocateStr( const char* inSrc )
		{
			if ( inSrc == NULL )
				inSrc = "";
			const THashMapType::Entry* existing( mHashMap.find( inSrc ) );
			if ( existing == NULL )
			{
				size_t len( strlen( inSrc ) );
				len += 1;
				char* newMem = reinterpret_cast<char*>(mWrapper.getAllocator().allocate( len, "PxStringTableImpl: const char*", __FILE__, __LINE__ ));
				physx::shdfnd::strlcpy( newMem, len, inSrc );
				mHashMap.insert( newMem, 1 );
				return newMem;
			}
			else
			{
				++const_cast<THashMapType::Entry*>(existing)->second;
				return existing->first;
			}
		}

		/**
		 *	Release the string table and all the strings associated with it.
		 */
		virtual void release()
		{
			PX_PROFILE_DELETE( mWrapper.getAllocator(), this );
		}
	};

	PxStringTable& PxStringTableExt::createStringTable( PxAllocatorCallback& inAllocator )
	{
		return *PX_PROFILE_NEW( inAllocator, PxStringTableImpl )( inAllocator );
	}
}
