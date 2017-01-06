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

#ifndef PXPVDSDK_PXPROFILECONTEXTPROVIDER_H
#define PXPVDSDK_PXPROFILECONTEXTPROVIDER_H

#include "PxProfileBase.h"

namespace physx { namespace profile {

	struct PxProfileEventExecutionContext
	{
		uint32_t				mThreadId;
		uint8_t					mCpuId;
		uint8_t					mThreadPriority;

		PxProfileEventExecutionContext( uint32_t inThreadId = 0, uint8_t inThreadPriority = 2 /*eThreadPriorityNormal*/, uint8_t inCpuId = 0 )
			: mThreadId( inThreadId )
			, mCpuId( inCpuId )
			, mThreadPriority( inThreadPriority )
		{
		}

		bool operator==( const PxProfileEventExecutionContext& inOther ) const
		{
			return mThreadId == inOther.mThreadId
				&& mCpuId == inOther.mCpuId
				&& mThreadPriority == inOther.mThreadPriority;
		}
	};

	//Provides the context in which the event is happening.
	class PxProfileContextProvider
	{
	protected:
		virtual ~PxProfileContextProvider(){}
	public:
		virtual PxProfileEventExecutionContext getExecutionContext() = 0;
		virtual uint32_t getThreadId() = 0;
	};
	//Provides pre-packaged context.
	struct PxProfileTrivialContextProvider
	{
		PxProfileEventExecutionContext mContext;
		PxProfileTrivialContextProvider( PxProfileEventExecutionContext inContext = PxProfileEventExecutionContext() )
			: mContext( inContext )
		{
		}
		PxProfileEventExecutionContext getExecutionContext() { return mContext; }
		uint32_t getThreadId() { return mContext.mThreadId; }
	};
	
	//Forwards the get context calls to another (perhaps shared) context.
	template<typename TProviderType>
	struct PxProfileContextProviderForward
	{
		TProviderType* mProvider;
		PxProfileContextProviderForward( TProviderType* inProvider ) : mProvider( inProvider ) {}
		PxProfileEventExecutionContext getExecutionContext() { return mProvider->getExecutionContext(); }
		uint32_t getThreadId() { return mProvider->getThreadId(); }
	};

	template<typename TProviderType>
	struct PxProfileContextProviderImpl : public PxProfileContextProvider
	{
		PxProfileContextProviderForward<TProviderType> mContext;
		PxProfileContextProviderImpl( TProviderType* inP ) : mContext( inP ) {}
		PxProfileEventExecutionContext getExecutionContext() { return mContext.getExecutionContext(); }
		uint32_t getThreadId() { return mContext.getThreadId(); }
	};

} }

#endif // PXPVDSDK_PXPROFILECONTEXTPROVIDER_H
