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
#ifndef PX_XML_MEMORY_ALLOCATOR_H
#define PX_XML_MEMORY_ALLOCATOR_H

#include "foundation/PxSimpleTypes.h"

namespace physx { 

	class XmlMemoryAllocator
	{
	protected:
		virtual ~XmlMemoryAllocator(){}
	public:
		virtual PxU8* allocate(PxU32 inSize) = 0;
		virtual void deallocate( PxU8* inMem ) = 0;
		virtual PxAllocatorCallback& getAllocator() = 0;
		template<typename TObjectType>
		TObjectType* allocate()
		{
			TObjectType* retval = reinterpret_cast< TObjectType* >( allocate( sizeof( TObjectType ) ) );
			new (retval) TObjectType();
			return retval;
		}

		template<typename TObjectType, typename TArgType>
		TObjectType* allocate(const TArgType &arg)
		{
			TObjectType* retval = reinterpret_cast< TObjectType* >( allocate( sizeof( TObjectType ) ) );
			new (retval) TObjectType(arg);
			return retval;
		}

		template<typename TObjectType>
		void deallocate( TObjectType* inObject )
		{
			deallocate( reinterpret_cast<PxU8*>( inObject ) );
		}
		template<typename TObjectType>
		inline TObjectType* batchAllocate(PxU32 inCount )
		{
			TObjectType* retval = reinterpret_cast<TObjectType*>( allocate( sizeof(TObjectType) * inCount ) );
			for ( PxU32 idx = 0; idx < inCount; ++idx )
			{
				new (retval + idx) TObjectType();
			}
			return retval;
		}

		template<typename TObjectType, typename TArgType>
		inline TObjectType* batchAllocate(PxU32 inCount, const TArgType &arg)
		{
			TObjectType* retval = reinterpret_cast<TObjectType*>( allocate( sizeof(TObjectType) * inCount ) );
			for ( PxU32 idx = 0; idx < inCount; ++idx )
			{
				new (retval + idx) TObjectType(arg);
			}
			return retval;
		}


		//Duplicate function definition for gcc.
		template<typename TObjectType>
		inline TObjectType* batchAllocate(TObjectType*, PxU32 inCount )
		{
			TObjectType* retval = reinterpret_cast<TObjectType*>( allocate( sizeof(TObjectType) * inCount ) );
			for ( PxU32 idx = 0; idx < inCount; ++idx )
			{
				new (retval + idx) TObjectType();
			}
			return retval;
		}
	};
	
	struct XmlMemoryAllocatorImpl : public XmlMemoryAllocator
	{
		Sn::TMemoryPoolManager mManager;

		XmlMemoryAllocatorImpl( PxAllocatorCallback& inAllocator )
			: mManager( inAllocator )
		{
		}
		XmlMemoryAllocatorImpl &operator=(const XmlMemoryAllocatorImpl &);
		virtual PxAllocatorCallback& getAllocator()
		{
			return mManager.getWrapper().getAllocator();
		}
		
		virtual PxU8* allocate(PxU32 inSize )
		{
			if ( !inSize )
				return NULL;

			return mManager.allocate( inSize );
		}
		virtual void deallocate( PxU8* inMem )
		{
			if ( inMem )
				mManager.deallocate( inMem );
		}
	};
}
#endif
