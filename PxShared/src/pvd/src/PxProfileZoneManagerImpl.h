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


#ifndef PXPVDSDK_PXPROFILEZONEMANAGERIMPL_H
#define PXPVDSDK_PXPROFILEZONEMANAGERIMPL_H

#include "PxProfileZoneManager.h"
#include "PxProfileBase.h"
#include "PxProfileScopedMutexLock.h"
#include "PxProfileZone.h"
#include "PxProfileAllocatorWrapper.h"

#include "PsArray.h"
#include "PsMutex.h"

namespace physx { namespace profile {

	struct NullEventNameProvider : public PxProfileNameProvider
	{
		virtual PxProfileNames getProfileNames() const { return PxProfileNames( 0, 0 ); }
	};

	class ZoneManagerImpl : public PxProfileZoneManager
	{
		typedef ScopedLockImpl<shdfnd::Mutex> TScopedLockType;
		PxProfileAllocatorWrapper					mWrapper;
		PxProfileArray<PxProfileZone*>		mZones;
		PxProfileArray<PxProfileZoneHandler*>	mHandlers;
		shdfnd::Mutex mMutex;

		ZoneManagerImpl( const ZoneManagerImpl& inOther );
		ZoneManagerImpl& operator=( const ZoneManagerImpl& inOther );

	public:

		ZoneManagerImpl(PxAllocatorCallback* inFoundation) 
			: mWrapper( inFoundation )
			, mZones( mWrapper )
			, mHandlers( mWrapper ) 
		{}

		virtual ~ZoneManagerImpl()
		{
			//This assert would mean that a profile zone is outliving us.
			//This will cause a crash when the profile zone is released.
			PX_ASSERT( mZones.size() == 0 );
			while( mZones.size() )
				removeProfileZone( *mZones.back() );
		}

		virtual void addProfileZone( PxProfileZone& inSDK )
		{
			TScopedLockType lock( &mMutex );
			
			if ( inSDK.getProfileZoneManager() != NULL )
			{
				if ( inSDK.getProfileZoneManager() == this )
					return;
				else //there must be two managers in the system somehow.
				{
					PX_ASSERT( false );
					inSDK.getProfileZoneManager()->removeProfileZone( inSDK );
				}
			}
			mZones.pushBack( &inSDK );
			inSDK.setProfileZoneManager( this );
			for ( uint32_t idx =0; idx < mHandlers.size(); ++idx )
				mHandlers[idx]->onZoneAdded( inSDK );
		}

		virtual void removeProfileZone( PxProfileZone& inSDK )
		{
			TScopedLockType lock( &mMutex );
			if ( inSDK.getProfileZoneManager() == NULL )
				return;

			else if ( inSDK.getProfileZoneManager() != this )
			{
				PX_ASSERT( false );
				inSDK.getProfileZoneManager()->removeProfileZone( inSDK );
				return;
			}

			inSDK.setProfileZoneManager( NULL );
			for ( uint32_t idx = 0; idx < mZones.size(); ++idx )
			{
				if ( mZones[idx] == &inSDK )
				{
					for ( uint32_t handler =0; handler < mHandlers.size(); ++handler )
						mHandlers[handler]->onZoneRemoved( inSDK );
					mZones.replaceWithLast( idx );
				}
			}
		}

		virtual void flushProfileEvents()
		{
			uint32_t sdkCount = mZones.size();
			for ( uint32_t idx = 0; idx < sdkCount; ++idx )
				mZones[idx]->flushProfileEvents();
		}

		virtual void addProfileZoneHandler( PxProfileZoneHandler& inHandler )
		{
			TScopedLockType lock( &mMutex );
			mHandlers.pushBack( &inHandler );
			for ( uint32_t idx = 0; idx < mZones.size(); ++idx )
				inHandler.onZoneAdded( *mZones[idx] );
		}

		virtual void removeProfileZoneHandler( PxProfileZoneHandler& inHandler )
		{
			TScopedLockType lock( &mMutex );
			for( uint32_t idx = 0; idx < mZones.size(); ++idx )
				inHandler.onZoneRemoved( *mZones[idx] );
			for( uint32_t idx = 0; idx < mHandlers.size(); ++idx )
			{
				if ( mHandlers[idx] == &inHandler )
					mHandlers.replaceWithLast( idx );
			}
		}
		
		virtual PxProfileZone& createProfileZone( const char* inSDKName, PxProfileNameProvider* inProvider, uint32_t inEventBufferByteSize )
		{
			NullEventNameProvider nullProvider;
			if ( inProvider == NULL )
				inProvider = &nullProvider;
			return createProfileZone( inSDKName, inProvider->getProfileNames(), inEventBufferByteSize );
		}
		
		
		virtual PxProfileZone& createProfileZone( const char* inSDKName, PxProfileNames inNames, uint32_t inEventBufferByteSize )
		{
			PxProfileZone& retval( PxProfileZone::createProfileZone( &mWrapper.getAllocator(), inSDKName, inNames, inEventBufferByteSize ) );
			addProfileZone( retval );
			return retval;
		}

		virtual void release() 
		{  
			PX_PROFILE_DELETE( mWrapper.getAllocator(), this );
		}
	};
} }


#endif // PXPVDSDK_PXPROFILEZONEMANAGERIMPL_H
