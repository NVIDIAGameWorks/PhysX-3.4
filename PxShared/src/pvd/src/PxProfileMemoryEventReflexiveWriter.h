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

#ifndef PXPVDSDK_PXPROFILEMEMORYEVENTREFLEXIVEWRITER_H
#define PXPVDSDK_PXPROFILEMEMORYEVENTREFLEXIVEWRITER_H

#include "PxProfileMemoryBuffer.h"
#include "PxProfileFoundationWrapper.h"
#include "PxProfileMemoryEvents.h"

namespace physx { namespace profile {

	struct MemoryEventReflexiveWriter
	{
		typedef PxProfileWrapperReflectionAllocator<uint8_t>	TAllocatorType;
		typedef MemoryBuffer<TAllocatorType>		TMemoryBufferType;
		typedef EventSerializer<TMemoryBufferType>	TSerializerType;


		PxProfileAllocatorWrapper	mWrapper;
		TMemoryBufferType	mBuffer;
		TSerializerType		mSerializer;

		MemoryEventReflexiveWriter( PxAllocatorCallback* inFoundation )
			: mWrapper( inFoundation )
			, mBuffer( TAllocatorType( mWrapper ) )
			, mSerializer( &mBuffer )
		{
		}

		template<typename TDataType>
		void operator()( const MemoryEventHeader& inHeader, const TDataType& inType )
		{
			//copy to get rid of const.
			MemoryEventHeader theHeader( inHeader );
			TDataType theData( inType );

			//write them out.
			theHeader.streamify( mSerializer );
			theData.streamify( mSerializer, theHeader );
		}
	};
}}

#endif // PXPVDSDK_PXPROFILEMEMORYEVENTREFLEXIVEWRITER_H