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

#ifndef PXPVDSDK_PXPROFILEEVENTNAMES_H
#define PXPVDSDK_PXPROFILEEVENTNAMES_H

#include "PxProfileBase.h"
#include "PxProfileEventId.h"

namespace physx { namespace profile {

	/**
	\brief Mapping from event id to name.
	*/
	struct PxProfileEventName
	{
		const char*					name;
		PxProfileEventId			eventId;

		/**
		\brief Default constructor.
		\param inName Profile event name.
		\param inId Profile event id.
		*/
		PxProfileEventName( const char* inName, PxProfileEventId inId ) : name( inName ), eventId( inId ) {}
	};

	/**
	\brief Aggregator of event id -> name mappings
	*/
	struct PxProfileNames
	{
		/**
		\brief Default constructor that doesn't point to any names.
		\param inEventCount Number of provided events.
		\param inSubsystems Event names array.
		*/
		PxProfileNames( uint32_t inEventCount = 0, const PxProfileEventName* inSubsystems = NULL )
			: eventCount( inEventCount )
			, events( inSubsystems )
		{
		}

		uint32_t							eventCount;
		const PxProfileEventName*			events;
	};

	/**
	\brief Provides a mapping from event ID -> name.
	*/
	class PxProfileNameProvider
	{
	public:
		/**
		\brief Returns profile event names.
		\return Profile event names.
		*/
		virtual PxProfileNames getProfileNames() const = 0;

	protected:
		virtual ~PxProfileNameProvider(){}
		PxProfileNameProvider& operator=(const PxProfileNameProvider&) { return *this; }
	};
} }

#endif // PXPVDSDK_PXPROFILEEVENTNAMES_H
