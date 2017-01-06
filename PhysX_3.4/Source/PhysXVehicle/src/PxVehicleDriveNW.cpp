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

#include "PxVehicleDriveNW.h"
#include "PxVehicleDrive.h"
#include "PxVehicleSDK.h"
#include "PxVehicleSuspWheelTire4.h"
#include "PxVehicleSuspLimitConstraintShader.h"
#include "PxVehicleDefaults.h"
#include "PxRigidDynamic.h"
#include "PxShape.h"
#include "PsFoundation.h"
#include "PsUtilities.h"
#include "CmPhysXCommon.h"
#include "PxScene.h"
#include "CmUtils.h"


namespace physx
{

extern PxF32 gToleranceScaleLength;

void PxVehicleDriveSimDataNW::setDiffData(const PxVehicleDifferentialNWData& diff)
{
	PX_CHECK_AND_RETURN(diff.isValid(), "Invalid PxVehicleCoreSimulationData.mDiff");
	mDiff=diff;
}

bool PxVehicleDriveSimDataNW::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(mDiff.isValid(), "Invalid PxVehicleDifferentialNWData", false);
	PX_CHECK_AND_RETURN_VAL(PxVehicleDriveSimData::isValid(), "Invalid PxVehicleDriveSimDataNW", false);
	return true;
}

///////////////////////////////////

bool PxVehicleDriveNW::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleDrive::isValid(), "invalid PxVehicleDrive", false);
	PX_CHECK_AND_RETURN_VAL(mDriveSimData.isValid(), "Invalid PxVehicleNW.mCoreSimData", false);
	return true;
}

PxVehicleDriveNW* PxVehicleDriveNW::allocate(const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN_NULL(numWheels>0, "Cars with zero wheels are illegal");
	PX_CHECK_AND_RETURN_NULL(gToleranceScaleLength > 0, "PxVehicleDriveNW::allocate - need to call PxInitVehicleSDK");

	//Compute the bytes needed.
	const PxU32 byteSize = sizeof(PxVehicleDriveNW) + PxVehicleDrive::computeByteSize(numWheels);

	//Allocate the memory.
	PxVehicleDriveNW* veh = static_cast<PxVehicleDriveNW*>(PX_ALLOC(byteSize, "PxVehicleDriveNW"));
	Cm::markSerializedMem(veh, byteSize);
	new(veh) PxVehicleDriveNW();

	//Patch up the pointers.
	PxU8* ptr = reinterpret_cast<PxU8*>(veh) + sizeof(PxVehicleDriveNW);
	ptr=PxVehicleDrive::patchupPointers(numWheels, veh, ptr);

	//Initialise
	veh->init(numWheels);

	//Set the vehicle type.
	veh->mType = PxVehicleTypes::eDRIVENW;

	return veh;
}

void PxVehicleDriveNW::free()
{
	PxVehicleDrive::free();
}

void PxVehicleDriveNW::setup
(PxPhysics* physics, PxRigidDynamic* vehActor,
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimDataNW& driveData,
 const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN(driveData.isValid(), "PxVehicleDriveNW::setup - invalid driveData");

	//Set up the wheels.
	PxVehicleDrive::setup(physics,vehActor,wheelsData,numWheels,0);

	//Start setting up the drive.
	PX_CHECK_MSG(driveData.isValid(), "PxVehicleNWDrive - invalid driveData");

	//Copy the simulation data.
	mDriveSimData = driveData;
}

PxVehicleDriveNW* PxVehicleDriveNW::create
(PxPhysics* physics, PxRigidDynamic* vehActor,
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimDataNW& driveData,
 const PxU32 numWheels)
{
	PxVehicleDriveNW* vehNW=PxVehicleDriveNW::allocate(numWheels);
	vehNW->setup(physics,vehActor,wheelsData,driveData,numWheels);
	return vehNW;
}


void PxVehicleDriveNW::setToRestState()
{
	//Set core to rest state.
	PxVehicleDrive::setToRestState();
}












} //namespace physx

