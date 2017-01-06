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

#include "PxVehicleSDK.h"
#include "PxPhysics.h"
#include "PxTolerancesScale.h"
#include "CmPhysXCommon.h"
#include "PsFoundation.h"
#include "PsUtilities.h"
#include "PxVehicleDrive4W.h"
#include "PxVehicleMetaDataObjects.h"
#include "PxVehicleSerialization.h"
#include "SnRepXSerializerImpl.h"
#include "PxSerializer.h"
#include "PxVehicleDriveTank.h"
#include "PxVehicleNoDrive.h"
#include "PxVehicleDriveNW.h"

namespace physx
{

void setVehicleToleranceScale(const PxTolerancesScale& ts);
void resetVehicleToleranceScale();
void setSerializationRegistryPtr(const PxSerializationRegistry* sr);
const PxSerializationRegistry* resetSerializationRegistryPtr();
void setVehicleDefaults();

bool PxInitVehicleSDK(PxPhysics& physics, PxSerializationRegistry* sr)
{
	PX_ASSERT(static_cast<Ps::Foundation*>(&physics.getFoundation()) == &Ps::Foundation::getInstance());
	Ps::Foundation::incRefCount();
	setVehicleToleranceScale(physics.getTolerancesScale());

	setVehicleDefaults();

	setSerializationRegistryPtr(sr);
	if(sr)
	{
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleDrive4W,		PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleDrive4W>));
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleDriveTank,	PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleDriveTank>));
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleDriveNW,		PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleDriveNW>));
		sr->registerRepXSerializer(PxVehicleConcreteType::eVehicleNoDrive,		PX_NEW_REPX_SERIALIZER(PxVehicleRepXSerializer<PxVehicleNoDrive>));
		
		sr->registerSerializer(PxVehicleConcreteType::eVehicleDrive4W,   		PX_NEW_SERIALIZER_ADAPTER(PxVehicleDrive4W));
		sr->registerSerializer(PxVehicleConcreteType::eVehicleDriveTank, 		PX_NEW_SERIALIZER_ADAPTER(PxVehicleDriveTank));
		sr->registerSerializer(PxVehicleConcreteType::eVehicleNoDrive,   		PX_NEW_SERIALIZER_ADAPTER(PxVehicleNoDrive));
		sr->registerSerializer(PxVehicleConcreteType::eVehicleDriveNW,   		PX_NEW_SERIALIZER_ADAPTER(PxVehicleDriveNW));

		sr->registerBinaryMetaDataCallback(PxVehicleDrive4W::getBinaryMetaData);	
		sr->registerBinaryMetaDataCallback(PxVehicleDriveTank::getBinaryMetaData);	
		sr->registerBinaryMetaDataCallback(PxVehicleNoDrive::getBinaryMetaData);
		sr->registerBinaryMetaDataCallback(PxVehicleDriveNW::getBinaryMetaData);
	}
	return true;
}

void PxCloseVehicleSDK(PxSerializationRegistry* sr)
{
	Ps::Foundation::decRefCount();
	resetVehicleToleranceScale();

	setVehicleDefaults();

	if (sr != resetSerializationRegistryPtr())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxCloseVehicleSDK called with different PxSerializationRegistry instance than PxInitVehicleSDK.");
		return;
	}

	if(sr)
	{
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleDrive4W));
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleDriveTank));
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleNoDrive));
		PX_DELETE_SERIALIZER_ADAPTER(sr->unregisterSerializer(PxVehicleConcreteType::eVehicleDriveNW));
		
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleDrive4W));
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleDriveTank));
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleNoDrive));
		PX_DELETE_REPX_SERIALIZER(sr->unregisterRepXSerializer(PxVehicleConcreteType::eVehicleDriveNW));
	}
}
/////////////////////////




}//physx

