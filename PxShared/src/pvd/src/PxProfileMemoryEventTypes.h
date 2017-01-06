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

#ifndef PXPVDSDK_PXPROFILEMEMORYEVENTTYPES_H
#define PXPVDSDK_PXPROFILEMEMORYEVENTTYPES_H

#include "PxProfileBase.h"
#include "PxProfileEventBufferClientManager.h"
#include "PxProfileEventSender.h"
#include "PsBroadcast.h"

namespace physx { namespace profile {

	struct PxProfileMemoryEventType
	{
		enum Enum
		{
			Unknown = 0,
			Allocation,
			Deallocation
		};
	};

	struct PxProfileBulkMemoryEvent
	{
		uint64_t mAddress;
		uint32_t mDatatype;
		uint32_t mFile;
		uint32_t mLine;
		uint32_t mSize;
		PxProfileMemoryEventType::Enum mType;

		PxProfileBulkMemoryEvent(){}

		PxProfileBulkMemoryEvent( uint32_t size, uint32_t type, uint32_t file, uint32_t line, uint64_t addr )
			: mAddress( addr )
			, mDatatype( type )
			, mFile( file )
			, mLine( line )
			, mSize( size )
			, mType( PxProfileMemoryEventType::Allocation )
		{
		}
		
		PxProfileBulkMemoryEvent( uint64_t addr )
			: mAddress( addr )
			, mDatatype( 0 )
			, mFile( 0 )
			, mLine( 0 )
			, mSize( 0 )
			, mType( PxProfileMemoryEventType::Deallocation )
		{
		}
	};
	
	class PxProfileBulkMemoryEventHandler
	{
	protected:
		virtual ~PxProfileBulkMemoryEventHandler(){}
	public:
		virtual void handleEvents( const PxProfileBulkMemoryEvent* inEvents, uint32_t inBufferSize ) = 0;
		static void parseEventBuffer( const uint8_t* inBuffer, uint32_t inBufferSize, PxProfileBulkMemoryEventHandler& inHandler, bool inSwapBytes, PxAllocatorCallback* inAlloc );
	};
} }

#endif // PXPVDSDK_PXPROFILEMEMORYEVENTTYPES_H
