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

#ifndef PXPVDSDK_PXPROFILEEVENTID_H
#define PXPVDSDK_PXPROFILEEVENTID_H

#include "PxProfileBase.h"

namespace physx { namespace profile {
	/**
	\brief A event id structure. Optionally includes information about
	if the event was enabled at compile time.
	*/
	struct PxProfileEventId
	{
		uint16_t		eventId;
		mutable bool	compileTimeEnabled; 

		/**
		\brief Profile event id constructor.
		\param inId Profile event id.
		\param inCompileTimeEnabled Compile time enabled.
		*/
		PxProfileEventId( uint16_t inId = 0, bool inCompileTimeEnabled = true )
			: eventId( inId )
			, compileTimeEnabled( inCompileTimeEnabled )
		{
		}

		operator uint16_t () const { return eventId; }

		bool operator==( const PxProfileEventId& inOther ) const 
		{ 
			return eventId == inOther.eventId;
		}
	};

	/**
	\brief Template event id structure.
	*/
	template<bool TEnabled>
	struct PxProfileCompileTimeFilteredEventId : public PxProfileEventId
	{
		/**
		\brief Constructor.
		\param inId Profile event id.		
		*/
		PxProfileCompileTimeFilteredEventId( uint16_t inId = 0 )
			: PxProfileEventId( inId, TEnabled )
		{
		}
	};
		
} }

#endif // PXPVDSDK_PXPROFILEEVENTID_H
