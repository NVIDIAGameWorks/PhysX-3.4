/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_STUB_PX_PROFILE_ZONE_H
#define APEX_STUB_PX_PROFILE_ZONE_H

#include "Px.h"
#include "PxProfileZone.h"
#include "PsUserAllocated.h"

namespace physx
{
	namespace profile
	{
		class PxUserCustomProfiler;
	}
}

namespace nvidia
{
using namespace physx::profile;

namespace apex
{

// This class provides a stub implementation of PhysX's PxProfileZone.
// It would be nice to not be forced to do this, but our scoped profile event macros
// cannot have an if(gProfileZone) because it would ruin the scope.  So here we just
// create a stub that will be called so that the user need not create a PxProfileZoneManager
// in debug mode (and suffer an assertion).

class ApexStubPxProfileZone : public PxProfileZone, public UserAllocated
{
public:	

	// PxProfileZone methods
	virtual const char* getName() { return 0; }
	virtual void release() { PX_DELETE(this); }

	virtual void setProfileZoneManager(PxProfileZoneManager* ) {}
	virtual profile::PxProfileZoneManager* getProfileZoneManager() { return 0; }

	virtual uint16_t getEventIdForName( const char*  ) { return 0; }

	virtual void flushEventIdNameMap() {}

	virtual uint16_t getEventIdsForNames( const char** , uint32_t  ) { return 0; }
	virtual void setUserCustomProfiler(PxUserCustomProfiler* ) {};

	// physx::PxProfileEventBufferClientManager methods
	virtual void addClient( PxProfileZoneClient&  ) {}
	virtual void removeClient( PxProfileZoneClient&  ) {}
	virtual bool hasClients() const { return false; }

	// physx::PxProfileNameProvider methods
	virtual PxProfileNames getProfileNames() const { return PxProfileNames(); }

	// profile::PxProfileEventSender methods
	virtual void startEvent( uint16_t , uint64_t ) {}
	virtual void stopEvent( uint16_t , uint64_t ) {}

	virtual void startEvent( uint16_t , uint64_t , uint32_t ) {}
	virtual void stopEvent( uint16_t , uint64_t , uint32_t  ) {}
	virtual void eventValue( uint16_t , uint64_t , int64_t  ) {}

	// physx::PxProfileEventFlusher methods
	virtual void flushProfileEvents() {}
};

}
} // end namespace nvidia::apex

#endif // APEX_STUB_PX_PROFILE_ZONE_H
