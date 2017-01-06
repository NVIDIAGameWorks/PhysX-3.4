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

#include "PxVehicleDriveTank.h"
#include "PxVehicleWheels.h"
#include "PxVehicleSDK.h"
#include "PxVehicleDefaults.h"
#include "PxRigidDynamic.h"
#include "CmPhysXCommon.h"
#include "CmUtils.h"
#include "PsFoundation.h"

namespace physx
{

extern PxF32 gToleranceScaleLength;

bool PxVehicleDriveTank::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleDrive::isValid(), "invalid PxVehicleDrive", false);
	PX_CHECK_AND_RETURN_VAL(mDriveSimData.isValid(), "Invalid PxVehicleDriveTank.mCoreSimData", false);
	return true;
}

PxVehicleDriveTank* PxVehicleDriveTank::allocate(const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN_NULL(numWheels>0, "Cars with zero wheels are illegal");
	PX_CHECK_AND_RETURN_NULL(0 == (numWheels % 2), "PxVehicleDriveTank::allocate - needs to have even number of wheels");
	PX_CHECK_AND_RETURN_NULL(gToleranceScaleLength > 0, "PxVehicleDriveTank::allocate - need to call PxInitVehicleSDK");

	//Compute the bytes needed.
	const PxU32 byteSize = sizeof(PxVehicleDriveTank) + PxVehicleDrive::computeByteSize(numWheels);

	//Allocate the memory.
	PxVehicleDriveTank* veh = static_cast<PxVehicleDriveTank*>(PX_ALLOC(byteSize, "PxVehicleDriveTank"));
	Cm::markSerializedMem(veh, byteSize);
	new(veh) PxVehicleDriveTank();

	//Patch up the pointers.
	PxU8* ptr = reinterpret_cast<PxU8*>(veh) + sizeof(PxVehicleDriveTank);
	PxVehicleDrive::patchupPointers(numWheels, veh, ptr);

	//Initialise.
	veh->init(numWheels);

	//Set the vehicle type.
	veh->mType = PxVehicleTypes::eDRIVETANK;

	//Set the default drive model.
	veh->mDriveModel = PxVehicleDriveTankControlModel::eSTANDARD;

	return veh;
}

void PxVehicleDriveTank::free()
{
	PxVehicleDrive::free();
}

void PxVehicleDriveTank::setup
(PxPhysics* physics, PxRigidDynamic* vehActor, 
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData& driveData,
 const PxU32 numDrivenWheels)
{
	PX_CHECK_AND_RETURN(driveData.isValid(), "PxVehicleDriveTank::setup - illegal drive data");

	//Set up the wheels.
	PxVehicleDrive::setup(physics,vehActor,wheelsData,numDrivenWheels,0);

	//Start setting up the drive.
	PX_CHECK_MSG(driveData.isValid(), "PxVehicle4WDrive - invalid driveData");

	//Copy the simulation data.
	mDriveSimData = driveData;
}

PxVehicleDriveTank* PxVehicleDriveTank::create
(PxPhysics* physics, PxRigidDynamic* vehActor, 
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData& driveData,
 const PxU32 numDrivenWheels)
{
	PxVehicleDriveTank* tank=PxVehicleDriveTank::allocate(numDrivenWheels);
	tank->setup(physics,vehActor,wheelsData,driveData,numDrivenWheels);
	return tank;
}


void PxVehicleDriveTank::setToRestState()
{
	//Set core to rest state.
	PxVehicleDrive::setToRestState();
}
} //namespace physx

