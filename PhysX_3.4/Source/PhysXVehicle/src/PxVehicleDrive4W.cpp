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

#include "PxVehicleDrive4W.h"
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

bool PxVehicleDriveSimData4W::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleDriveSimData::isValid(), "Invalid PxVehicleDriveSimData4W", false);
	PX_CHECK_AND_RETURN_VAL(mDiff.isValid(), "Invalid PxVehicleCoreSimulationData.mDiff", false);
	PX_CHECK_AND_RETURN_VAL(mAckermannGeometry.isValid(), "Invalid PxVehicleCoreSimulationData.mAckermannGeometry", false);
	return true;
}

void PxVehicleDriveSimData4W::setDiffData(const PxVehicleDifferential4WData& diff)
{
	PX_CHECK_AND_RETURN(diff.mType!=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD || (diff.mFrontRearSplit>=0 && diff.mFrontRearSplit<=1.0f), "Diff torque split between front and rear must be in range (0,1)");
	PX_CHECK_AND_RETURN(diff.mType!=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD || (diff.mCentreBias>=1), "Diff centre bias must be greater than or equal to 1");
	PX_CHECK_AND_RETURN((diff.mType!=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD && diff.mType!=PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD) || (diff.mFrontBias>=1), "Diff front bias must be greater than or equal to 1");
	PX_CHECK_AND_RETURN((diff.mType!=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD && diff.mType!=PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD) || (diff.mRearBias>=1), "Diff rear bias must be greater than or equal to 1");
	PX_CHECK_AND_RETURN(diff.mType<PxVehicleDifferential4WData::eMAX_NB_DIFF_TYPES, "Illegal differential type");

	mDiff=diff;
}

void PxVehicleDriveSimData4W::setAckermannGeometryData(const PxVehicleAckermannGeometryData& ackermannData)
{
	PX_CHECK_AND_RETURN(ackermannData.mFrontWidth > 0, "Illegal ackermannData.mFrontWidth - must be greater than zero");
	PX_CHECK_AND_RETURN(ackermannData.mRearWidth > 0, "Illegal ackermannData.mRearWidth - must be greater than zero");
	PX_CHECK_AND_RETURN(ackermannData.mAxleSeparation > 0, "Illegal ackermannData.mAxleSeparation - must be greater than zero");

	mAckermannGeometry = ackermannData;
}

///////////////////////////////////

bool PxVehicleDrive4W::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleDrive::isValid(), "invalid PxVehicleDrive", false);
	PX_CHECK_AND_RETURN_VAL(mDriveSimData.isValid(), "Invalid PxVehicleNW.mCoreSimData", false);
	return true;
}

PxVehicleDrive4W* PxVehicleDrive4W::allocate(const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN_NULL(numWheels>=4, "PxVehicleDrive4W::allocate - needs to have at least 4 wheels");
	PX_CHECK_AND_RETURN_NULL(gToleranceScaleLength > 0, "PxVehicleDrive4W::allocate - need to call PxInitVehicleSDK");

	//Compute the bytes needed.
	const PxU32 byteSize = sizeof(PxVehicleDrive4W) + PxVehicleDrive::computeByteSize(numWheels);

	//Allocate the memory.
	PxVehicleDrive4W* veh = static_cast<PxVehicleDrive4W*>(PX_ALLOC(byteSize, "PxVehicleDrive4W"));
	Cm::markSerializedMem(veh, byteSize);
	new(veh) PxVehicleDrive4W();

	//Patch up the pointers.
	PxU8* ptr = reinterpret_cast<PxU8*>(veh) + sizeof(PxVehicleDrive4W);
	ptr=PxVehicleDrive::patchupPointers(numWheels, veh, ptr);

	//Initialise wheels.
	veh->init(numWheels);

	//Set the vehicle type.
	veh->mType = PxVehicleTypes::eDRIVE4W;

	return veh;
}

void PxVehicleDrive4W::free()
{
	PxVehicleDrive::free();
}

void PxVehicleDrive4W::setup
(PxPhysics* physics, PxRigidDynamic* vehActor,
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData,
 const PxU32 numNonDrivenWheels)
{
	PX_CHECK_AND_RETURN(driveData.isValid(), "PxVehicleDrive4W::setup - invalid driveData");
	PX_CHECK_AND_RETURN(wheelsData.getNbWheels() >= 4, "PxVehicleDrive4W::setup - needs to have at least 4 wheels");

	//Set up the wheels.
	PxVehicleDrive::setup(physics,vehActor,wheelsData,4,numNonDrivenWheels);

	//Start setting up the drive.
	PX_CHECK_MSG(driveData.isValid(), "PxVehicle4WDrive - invalid driveData");

	//Copy the simulation data.
	mDriveSimData = driveData;
}

PxVehicleDrive4W* PxVehicleDrive4W::create
(PxPhysics* physics, PxRigidDynamic* vehActor,
 const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData,
 const PxU32 numNonDrivenWheels)
{
	PxVehicleDrive4W* veh4W=PxVehicleDrive4W::allocate(4+numNonDrivenWheels);
	veh4W->setup(physics,vehActor,wheelsData,driveData,numNonDrivenWheels);
	return veh4W;
}


void PxVehicleDrive4W::setToRestState()
{
	//Set core to rest state.
	PxVehicleDrive::setToRestState();
}
} //namespace physx

