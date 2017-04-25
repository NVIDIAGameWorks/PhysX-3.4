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

#ifndef PX_VEHICLE_SERIALIZATION_H
#define PX_VEHICLE_SERIALIZATION_H

#include "extensions/PxRepXSimpleType.h"
#include "SnRepXSerializerImpl.h"

namespace physx
{
	class PxRepXSerializer;
	class PxSerializationRegistry;
	class XmlReader;
	class XmlMemoryAllocator;
	class XmlWriter;
	class MemoryBuffer;

	PX_DEFINE_TYPEINFO(PxVehicleNoDrive,		PxVehicleConcreteType::eVehicleNoDrive)
	PX_DEFINE_TYPEINFO(PxVehicleDrive4W,		PxVehicleConcreteType::eVehicleDrive4W)
	PX_DEFINE_TYPEINFO(PxVehicleDriveNW,		PxVehicleConcreteType::eVehicleDriveNW)
	PX_DEFINE_TYPEINFO(PxVehicleDriveTank,		PxVehicleConcreteType::eVehicleDriveTank)
	
	template<typename TVehicleType>
	struct PxVehicleRepXSerializer : public RepXSerializerImpl<TVehicleType>
	{
		PxVehicleRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<TVehicleType>( inCallback ) {}
		virtual PxRepXObject fileToObject( XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection );
		virtual void objectToFileImpl( const TVehicleType* , PxCollection* , XmlWriter& , MemoryBuffer& , PxRepXInstantiationArgs& );
		virtual TVehicleType* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};

#if PX_SUPPORT_EXTERN_TEMPLATE
	// explicit template instantiation declarations
	extern template struct PxVehicleRepXSerializer<PxVehicleDrive4W>;
	extern template struct PxVehicleRepXSerializer<PxVehicleDriveTank>;
	extern template struct PxVehicleRepXSerializer<PxVehicleDriveNW>;
	extern template struct PxVehicleRepXSerializer<PxVehicleNoDrive>;
#endif

}


#endif//PX_VEHICLE_REPX_SERIALIZER_H
