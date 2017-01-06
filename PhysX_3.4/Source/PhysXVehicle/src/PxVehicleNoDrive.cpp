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

#include "foundation/PxMemory.h"
#include "PxVehicleNoDrive.h"
#include "PxVehicleWheels.h"
#include "PxVehicleDefaults.h"
#include "PxRigidDynamic.h"
#include "CmPhysXCommon.h"
#include "CmUtils.h"
#include "PsFoundation.h"

namespace physx
{

extern PxF32 gToleranceScaleLength;

bool PxVehicleNoDrive::isValid() const
{
	PX_CHECK_AND_RETURN_VAL(PxVehicleWheels::isValid(), "invalid PxVehicleDrive", false);
	return true;
}

PxVehicleNoDrive* PxVehicleNoDrive::allocate(const PxU32 numWheels)
{
	PX_CHECK_AND_RETURN_NULL(numWheels>0, "Cars with zero wheels are illegal");
	PX_CHECK_AND_RETURN_NULL(gToleranceScaleLength > 0, "PxVehicleNoDrive::allocate - need to call PxInitVehicleSDK");

	//Compute the bytes needed.
	const PxU32 numWheels4 = (((numWheels + 3) & ~3) >> 2);
	const PxU32 inputByteSize16 = sizeof(PxReal)*numWheels4*4;
	const PxU32 byteSize = sizeof(PxVehicleNoDrive) + 3*inputByteSize16 + PxVehicleWheels::computeByteSize(numWheels);

	//Allocate the memory.
	PxVehicleNoDrive* veh = static_cast<PxVehicleNoDrive*>(PX_ALLOC(byteSize, "PxVehicleNoDrive"));
	Cm::markSerializedMem(veh, byteSize);
	new(veh) PxVehicleNoDrive();

	//Patch up the pointers.
	PxU8* ptr = reinterpret_cast<PxU8*>(veh) + sizeof(PxVehicleNoDrive);
	veh->mSteerAngles = reinterpret_cast<PxReal*>(ptr);
	ptr += inputByteSize16;
	veh->mDriveTorques = reinterpret_cast<PxReal*>(ptr);
	ptr += inputByteSize16;
	veh->mBrakeTorques = reinterpret_cast<PxReal*>(ptr);
	ptr += inputByteSize16;
	ptr = PxVehicleWheels::patchupPointers(numWheels, veh, ptr);

	//Initialise.
	PxMemZero(veh->mSteerAngles, inputByteSize16);
	PxMemZero(veh->mDriveTorques, inputByteSize16);
	PxMemZero(veh->mBrakeTorques, inputByteSize16);
	veh->init(numWheels);

	//Set the vehicle type.
	veh->mType = PxVehicleTypes::eNODRIVE;

	return veh;
}

void PxVehicleNoDrive::free()
{
	PxVehicleWheels::free();
}

void PxVehicleNoDrive::setup
(PxPhysics* physics, PxRigidDynamic* vehActor, const PxVehicleWheelsSimData& wheelsData)
{
	//Set up the wheels.
	PxVehicleWheels::setup(physics,vehActor,wheelsData,0,wheelsData.getNbWheels());
}

PxVehicleNoDrive* PxVehicleNoDrive::create
(PxPhysics* physics, PxRigidDynamic* vehActor, 
 const PxVehicleWheelsSimData& wheelsData)
{
	PxVehicleNoDrive* veh=PxVehicleNoDrive::allocate(wheelsData.getNbWheels());
	veh->setup(physics,vehActor,wheelsData);
	return veh;
}

void PxVehicleNoDrive::setToRestState()
{
	const PxU32 numWheels4 = (((mWheelsSimData.getNbWheels() + 3) & ~3) >> 2);
	const PxU32 inputByteSize = sizeof(PxReal)*numWheels4*4;
	const PxU32 inputByteSize16 = (inputByteSize + 15) & ~15;
	PxMemZero(mSteerAngles, 3*inputByteSize16);

	//Set core to rest state.
	PxVehicleWheels::setToRestState();
}

void PxVehicleNoDrive::setBrakeTorque(const PxU32 id, const PxReal brakeTorque)
{
	PX_CHECK_AND_RETURN(id < mWheelsSimData.getNbWheels(), "PxVehicleNoDrive::setBrakeTorque - Illegal wheel");
	PX_CHECK_AND_RETURN(brakeTorque>=0, "PxVehicleNoDrive::setBrakeTorque - negative brake torques are illegal");
	mBrakeTorques[id] = brakeTorque;
}

void PxVehicleNoDrive::setDriveTorque(const PxU32 id, const PxReal driveTorque)
{
	PX_CHECK_AND_RETURN(id < mWheelsSimData.getNbWheels(), "PxVehicleNoDrive::setDriveTorque - Illegal wheel");
	mDriveTorques[id] = driveTorque;
}

void PxVehicleNoDrive::setSteerAngle(const PxU32 id, const PxReal steerAngle)
{
	PX_CHECK_AND_RETURN(id < mWheelsSimData.getNbWheels(), "PxVehicleNoDrive::setSteerAngle - Illegal wheel");
	mSteerAngles[id] = steerAngle;
}

PxReal PxVehicleNoDrive::getBrakeTorque(const PxU32 id) const
{
	PX_CHECK_AND_RETURN_VAL(id < mWheelsSimData.getNbWheels(), "PxVehicleNoDrive::getBrakeTorque - Illegal wheel", 0);
	return mBrakeTorques[id];
}

PxReal PxVehicleNoDrive::getDriveTorque(const PxU32 id) const
{
	PX_CHECK_AND_RETURN_VAL(id < mWheelsSimData.getNbWheels(), "PxVehicleNoDrive::getDriveTorque - Illegal wheel",0);
	return mDriveTorques[id];
}

PxReal PxVehicleNoDrive::getSteerAngle(const PxU32 id) const
{
	PX_CHECK_AND_RETURN_VAL(id < mWheelsSimData.getNbWheels(), "PxVehicleNoDrive::getSteerAngle - Illegal wheel",0);
	return mSteerAngles[id];
}

} //namespace physx

