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

#ifndef PXPVDSDK_PXPROFILEEVENTFILTER_H
#define PXPVDSDK_PXPROFILEEVENTFILTER_H

#include "foundation/PxAssert.h"
#include "PxProfileBase.h"
#include "PxProfileEventId.h"

namespace physx { namespace profile {

	/**
	\brief Called upon every event to give a quick-out before adding the event
	to the event buffer.

	\note: not thread safe, can be called from different threads at the same time
	*/
	class PxProfileEventFilter
	{
	protected:
		virtual ~PxProfileEventFilter(){}
	public:
		/**
		\brief Disabled events will not go into the event buffer and will not be 
		transmitted to clients.
		\param inId Profile event id.
		\param isEnabled True if event should be enabled.
		*/
		virtual void setEventEnabled( const PxProfileEventId& inId, bool isEnabled ) = 0;

		/**
		\brief Returns the current state of the profile event.
		\return True if profile event is enabled.
		*/
		virtual bool isEventEnabled( const PxProfileEventId& inId ) const = 0;
	};

	/**
	\brief Forwards the filter requests to another event filter.
	*/
	template<typename TFilterType>
	struct PxProfileEventFilterForward
	{		
		/**
		\brief Default constructor.
		*/
		PxProfileEventFilterForward( TFilterType* inFilter ) : filter( inFilter ) {}

		/**
		\brief Disabled events will not go into the event buffer and will not be 
		transmitted to clients.
		\param inId Profile event id.
		\param isEnabled True if event should be enabled.
		*/
		void setEventEnabled( const PxProfileEventId& inId, bool isEnabled ) { filter->setEventEnabled( inId, isEnabled ); }

		/**
		\brief Returns the current state of the profile event.
		\return True if profile event is enabled.
		*/
		bool isEventEnabled( const PxProfileEventId& inId ) const { return filter->isEventEnabled( inId ); }

		TFilterType* filter;
	};

} }

#endif // PXPVDSDK_PXPROFILEEVENTFILTER_H
