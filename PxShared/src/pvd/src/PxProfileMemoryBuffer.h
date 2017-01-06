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


#ifndef PXPVDSDK_PXPROFILEMEMORYBUFFER_H
#define PXPVDSDK_PXPROFILEMEMORYBUFFER_H

#include "PxProfileBase.h"
#include "PsAllocator.h"
#include "foundation/PxMemory.h"

namespace physx { namespace profile {

	template<typename TAllocator = typename shdfnd::AllocatorTraits<uint8_t>::Type >
	class MemoryBuffer : public TAllocator
	{
		uint8_t* mBegin;
		uint8_t* mEnd;
		uint8_t* mCapacityEnd;

	public:
		MemoryBuffer( const TAllocator& inAlloc = TAllocator() ) : TAllocator( inAlloc ), mBegin( 0 ), mEnd( 0 ), mCapacityEnd( 0 ) {}
		~MemoryBuffer()
		{
			if ( mBegin ) TAllocator::deallocate( mBegin );
		}
		uint32_t size() const { return static_cast<uint32_t>( mEnd - mBegin ); }
		uint32_t capacity() const { return static_cast<uint32_t>( mCapacityEnd - mBegin ); }
		uint8_t* begin() { return mBegin; }
		uint8_t* end() { return mEnd; }
		void setEnd(uint8_t* nEnd) { mEnd = nEnd; }
		const uint8_t* begin() const { return mBegin; }
		const uint8_t* end() const { return mEnd; }
		void clear() { mEnd = mBegin; }
		uint32_t write( uint8_t inValue )
		{
			growBuf( 1 );
			*mEnd = inValue;
			++mEnd;
			return 1;
		}

		template<typename TDataType>
		uint32_t write( const TDataType& inValue )
		{
			uint32_t writtenSize = sizeof(TDataType);
			growBuf(writtenSize);
			const uint8_t* __restrict readPtr = reinterpret_cast< const uint8_t* >( &inValue );
			uint8_t* __restrict writePtr = mEnd;
			for ( uint32_t idx = 0; idx < sizeof(TDataType); ++idx ) writePtr[idx] = readPtr[idx];
			mEnd += writtenSize;
			return writtenSize;
		}
		
		template<typename TDataType>
		uint32_t write( const TDataType* inValue, uint32_t inLength )
		{
			if ( inValue && inLength )
			{
				uint32_t writeSize = inLength * sizeof( TDataType );
				growBuf( writeSize );
				PxMemCopy( mBegin + size(), inValue, writeSize );
				mEnd += writeSize;
				return writeSize;
			}
			return 0;
		}

		// used by atomic write. Store the data and write the end afterwards
		// we dont check the buffer size, it should not resize on the fly
		template<typename TDataType>
		uint32_t write(const TDataType* inValue, uint32_t inLength, int32_t index)
		{
			if (inValue && inLength)
			{
				uint32_t writeSize = inLength * sizeof(TDataType);
				PX_ASSERT(mBegin + index + writeSize < mCapacityEnd);
				PxMemCopy(mBegin + index, inValue, writeSize);				
				return writeSize;
			}
			return 0;
		}
		
		void growBuf( uint32_t inAmount )
		{
			uint32_t newSize = size() + inAmount;
			reserve( newSize );
		}
		void resize( uint32_t inAmount )
		{
			reserve( inAmount );
			mEnd = mBegin + inAmount;
		}
		void reserve( uint32_t newSize )
		{
			uint32_t currentSize = size();
			if ( newSize >= capacity() )
			{
				const uint32_t allocSize = mBegin ? newSize * 2 : newSize;

				uint8_t* newData = static_cast<uint8_t*>(TAllocator::allocate(allocSize, __FILE__, __LINE__));
				memset(newData, 0xf,allocSize);
				if ( mBegin )
				{
					PxMemCopy( newData, mBegin, currentSize );
					TAllocator::deallocate( mBegin );
				}
				mBegin = newData;
				mEnd = mBegin + currentSize;
				mCapacityEnd = mBegin + allocSize;
			}
		}
	};

	
	class TempMemoryBuffer
	{
		uint8_t* mBegin;
		uint8_t* mEnd;
		uint8_t* mCapacityEnd;

	public:
		TempMemoryBuffer(uint8_t* data, int32_t size) : mBegin(data), mEnd(data), mCapacityEnd(data + size) {}
		~TempMemoryBuffer()
		{			
		}
		uint32_t size() const { return static_cast<uint32_t>(mEnd - mBegin); }
		uint32_t capacity() const { return static_cast<uint32_t>(mCapacityEnd - mBegin); }
		const uint8_t* begin() { return mBegin; }
		uint8_t* end() { return mEnd; }
		const uint8_t* begin() const { return mBegin; }
		const uint8_t* end() const { return mEnd; }		
		uint32_t write(uint8_t inValue)
		{			
			*mEnd = inValue;
			++mEnd;
			return 1;
		}

		template<typename TDataType>
		uint32_t write(const TDataType& inValue)
		{
			uint32_t writtenSize = sizeof(TDataType);			
			const uint8_t* __restrict readPtr = reinterpret_cast<const uint8_t*>(&inValue);
			uint8_t* __restrict writePtr = mEnd;
			for (uint32_t idx = 0; idx < sizeof(TDataType); ++idx) writePtr[idx] = readPtr[idx];
			mEnd += writtenSize;
			return writtenSize;
		}

		template<typename TDataType>
		uint32_t write(const TDataType* inValue, uint32_t inLength)
		{
			if (inValue && inLength)
			{
				uint32_t writeSize = inLength * sizeof(TDataType);
				PxMemCopy(mBegin + size(), inValue, writeSize);
				mEnd += writeSize;
				return writeSize;
			}
			return 0;
		}
	};

}}

#endif // PXPVDSDK_PXPROFILEMEMORYBUFFER_H
