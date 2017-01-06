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

#ifndef PXPVDSDK_PXPROFILECOMPILETIMEEVENTFILTER_H
#define PXPVDSDK_PXPROFILECOMPILETIMEEVENTFILTER_H

#include "PxProfileBase.h"
#include "PxProfileEventId.h"

//Define before including header in order to enable a different
//compile time event profile threshold.
#ifndef PX_PROFILE_EVENT_PROFILE_THRESHOLD
#define PX_PROFILE_EVENT_PROFILE_THRESHOLD EventPriorities::Medium
#endif

namespace physx { namespace profile {

	/**
	\brief Profile event priorities. Used to filter out events.
	*/
	struct EventPriorities
	{
		enum Enum
		{
			None,		// the filter setting to kill all events
			Coarse,
			Medium,
			Detail,
			Never		// the priority to set for an event if it should never fire.
		};
	};

	/**
	\brief Gets the priority for a given event.
	Specialize this object in order to get the priorities setup correctly.
	*/
	template<uint16_t TEventId>
	struct EventPriority { static const uint32_t val = EventPriorities::Medium; };

	/**
	\brief 	Filter events by given event priority and set threshold.
	*/
	template<uint16_t TEventId>
	struct EventFilter
	{
		static const bool val = EventPriority<TEventId>::val <= PX_PROFILE_EVENT_PROFILE_THRESHOLD;
	};

}}

#endif // PXPVDSDK_PXPROFILECOMPILETIMEEVENTFILTER_H
