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


#ifndef PXPVDSDK_PXPROFILEALLOCATORWRAPPER_H
#define PXPVDSDK_PXPROFILEALLOCATORWRAPPER_H

#include "foundation/PxPreprocessor.h"
#include "foundation/PxAllocatorCallback.h"
#include "foundation/PxErrorCallback.h"
#include "foundation/PxAssert.h"

#include "PsArray.h"
#include "PsHashMap.h"

namespace physx { namespace profile {

	/**
	\brief Helper struct to encapsulate the user allocator callback
	Useful for array and hash templates
	*/
	struct PxProfileAllocatorWrapper
	{
		PxAllocatorCallback*			mUserAllocator;

		PxProfileAllocatorWrapper( PxAllocatorCallback& inUserAllocator )
			: mUserAllocator( &inUserAllocator )
		{
		}

		PxProfileAllocatorWrapper( PxAllocatorCallback* inUserAllocator )
			: mUserAllocator( inUserAllocator )
		{
		}

		PxAllocatorCallback&		getAllocator() const
		{
			PX_ASSERT( NULL != mUserAllocator );
			return *mUserAllocator;
		}
	};

	/**
	\brief Helper class to encapsulate the reflection allocator
	*/
	template <typename T>
	class PxProfileWrapperReflectionAllocator
	{
		static const char* getName()
		{
#if PX_LINUX || PX_ANDROID || PX_PS4 || PX_IOS || PX_OSX || PX_EMSCRIPTEN || PX_SWITCH
			return __PRETTY_FUNCTION__;
#else
			return typeid(T).name();
#endif
		}
		PxProfileAllocatorWrapper* mWrapper;

	public:
		PxProfileWrapperReflectionAllocator(PxProfileAllocatorWrapper& inWrapper) : mWrapper( &inWrapper )	{}
		PxProfileWrapperReflectionAllocator( const PxProfileWrapperReflectionAllocator& inOther )
			: mWrapper( inOther.mWrapper )
		{
		}
		PxProfileWrapperReflectionAllocator& operator=( const PxProfileWrapperReflectionAllocator& inOther )
		{
			mWrapper = inOther.mWrapper;
			return *this;
		}
		PxAllocatorCallback& getAllocator() { return mWrapper->getAllocator(); }
		void* allocate(size_t size, const char* filename, int line)
		{
#if PX_CHECKED // checked and debug builds
			if(!size)
				return 0;
			return getAllocator().allocate(size, getName(), filename, line);
#else
			return getAllocator().allocate(size, "<no allocation names in this config>", filename, line);
#endif
		}
		void deallocate(void* ptr)
		{
			if(ptr)
				getAllocator().deallocate(ptr);
		}
	};

	/**
	\brief Helper class to encapsulate the named allocator
	*/
	struct PxProfileWrapperNamedAllocator
	{
		PxProfileAllocatorWrapper*	mWrapper;
		const char*			mAllocationName;
		PxProfileWrapperNamedAllocator(PxProfileAllocatorWrapper& inWrapper, const char* inAllocationName)
			: mWrapper( &inWrapper )
			, mAllocationName( inAllocationName )
		{}
		PxProfileWrapperNamedAllocator( const PxProfileWrapperNamedAllocator& inOther )
			: mWrapper( inOther.mWrapper )
			, mAllocationName( inOther.mAllocationName )
		{
		}
		PxProfileWrapperNamedAllocator& operator=( const PxProfileWrapperNamedAllocator& inOther )
		{
			mWrapper = inOther.mWrapper;
			mAllocationName = inOther.mAllocationName;
			return *this;
		}
		PxAllocatorCallback& getAllocator() { return mWrapper->getAllocator(); }
		void* allocate(size_t size, const char* filename, int line)
		{
			if(!size)
				return 0;
			return getAllocator().allocate(size, mAllocationName, filename, line);
		}
		void deallocate(void* ptr)
		{
			if(ptr)
				getAllocator().deallocate(ptr);
		}
	};

	/**
	\brief Helper struct to encapsulate the array
	*/
	template<class T>
	struct PxProfileArray : public shdfnd::Array<T, PxProfileWrapperReflectionAllocator<T> >
	{
		typedef PxProfileWrapperReflectionAllocator<T> TAllocatorType;

		PxProfileArray( PxProfileAllocatorWrapper& inWrapper )
			: shdfnd::Array<T, TAllocatorType >( TAllocatorType( inWrapper ) )
		{
		}

		PxProfileArray( const PxProfileArray< T >& inOther )
			: shdfnd::Array<T, TAllocatorType >( inOther, inOther )
		{
		}
	};

	/**
	\brief Helper struct to encapsulate the array
	*/
	template<typename TKeyType, typename TValueType, typename THashType=shdfnd::Hash<TKeyType> >
	struct PxProfileHashMap : public shdfnd::HashMap<TKeyType, TValueType, THashType, PxProfileWrapperReflectionAllocator< TValueType > >
	{
		typedef shdfnd::HashMap<TKeyType, TValueType, THashType, PxProfileWrapperReflectionAllocator< TValueType > > THashMapType;
		typedef PxProfileWrapperReflectionAllocator<TValueType> TAllocatorType;
		PxProfileHashMap( PxProfileAllocatorWrapper& inWrapper )
			: THashMapType( TAllocatorType( inWrapper ) )
		{
		}
	};

	/**
	\brief Helper function to encapsulate the profile allocation
	*/
	template<typename TDataType>
	inline TDataType* PxProfileAllocate( PxAllocatorCallback* inAllocator, const char* file, int inLine )
	{
		PxProfileAllocatorWrapper wrapper( inAllocator );
		typedef PxProfileWrapperReflectionAllocator< TDataType > TAllocator;
		TAllocator theAllocator( wrapper );
		return reinterpret_cast<TDataType*>( theAllocator.allocate( sizeof( TDataType ), file, inLine ) );
	}

	/**
	\brief Helper function to encapsulate the profile allocation
	*/
	template<typename TDataType>
	inline TDataType* PxProfileAllocate( PxAllocatorCallback& inAllocator, const char* file, int inLine )
	{
		return PxProfileAllocate<TDataType>( &inAllocator, file, inLine );
	}

	/**
	\brief Helper function to encapsulate the profile deallocation
	*/
	template<typename TDataType>
	inline void PxProfileDeleteAndDeallocate( PxProfileAllocatorWrapper& inAllocator, TDataType* inDType )
	{
		PX_ASSERT(inDType);
		PxAllocatorCallback& allocator( inAllocator.getAllocator() );
		inDType->~TDataType();
		allocator.deallocate( inDType );
	}

	/**
	\brief Helper function to encapsulate the profile deallocation
	*/
	template<typename TDataType>
	inline void PxProfileDeleteAndDeallocate( PxAllocatorCallback& inAllocator, TDataType* inDType )
	{
		PxProfileAllocatorWrapper wrapper( &inAllocator );
		PxProfileDeleteAndDeallocate( wrapper, inDType );
	}

} }

#define PX_PROFILE_NEW( allocator, dtype ) new (physx::profile::PxProfileAllocate<dtype>( allocator, __FILE__, __LINE__ )) dtype
#define PX_PROFILE_DELETE( allocator, obj ) physx::profile::PxProfileDeleteAndDeallocate( allocator, obj );

#endif // PXPVDSDK_PXPROFILEALLOCATORWRAPPER_H
