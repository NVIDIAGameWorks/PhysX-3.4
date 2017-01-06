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

#include "foundation/PxMath.h"
#include "PxVehicleUtilSetup.h"
#include "PxVehicleDrive4W.h"
#include "PxVehicleDriveNW.h"
#include "PxVehicleDriveTank.h"
#include "PxVehicleNoDrive.h"
#include "PxVehicleWheels.h"
#include "PxVehicleUtil.h"
#include "PxVehicleUpdate.h"
#include "PsFoundation.h"
#include "CmPhysXCommon.h"

namespace physx
{

void enable3WMode(const PxU32 rightDirection, const PxU32 upDirection, const bool removeFrontWheel, PxVehicleWheelsSimData& wheelsSimData, PxVehicleWheelsDynData& wheelsDynData, PxVehicleDriveSimData4W& driveSimData);

void computeDirection(PxU32& rightDirection, PxU32& upDirection);

void PxVehicle4WEnable3WTadpoleMode(PxVehicleWheelsSimData& wheelsSimData, PxVehicleWheelsDynData& wheelsDynData, PxVehicleDriveSimData4W& driveSimData)
{
	PX_CHECK_AND_RETURN
		(!wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eFRONT_LEFT) &&
		 !wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT) &&
		 !wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eREAR_LEFT) &&
		 !wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eREAR_RIGHT), "PxVehicle4WEnable3WTadpoleMode requires no wheels to be disabled");

	PxU32 rightDirection=0xffffffff;
	PxU32 upDirection=0xffffffff;
	computeDirection(rightDirection, upDirection);
	PX_CHECK_AND_RETURN(rightDirection<3 && upDirection<3, "PxVehicle4WEnable3WTadpoleMode requires the vectors set in PxVehicleSetBasisVectors to be axis-aligned");

	enable3WMode(rightDirection, upDirection, false, wheelsSimData, wheelsDynData, driveSimData);
}

void PxVehicle4WEnable3WDeltaMode(PxVehicleWheelsSimData& wheelsSimData, PxVehicleWheelsDynData& wheelsDynData, PxVehicleDriveSimData4W& driveSimData)
{
	PX_CHECK_AND_RETURN
		(!wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eFRONT_LEFT) &&
		!wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT) &&
		!wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eREAR_LEFT) &&
		!wheelsSimData.getIsWheelDisabled(PxVehicleDrive4WWheelOrder::eREAR_RIGHT), "PxVehicle4WEnable3WDeltaMode requires no wheels to be disabled");

	PxU32 rightDirection=0xffffffff;
	PxU32 upDirection=0xffffffff;
	computeDirection(rightDirection, upDirection);
	PX_CHECK_AND_RETURN(rightDirection<3 && upDirection<3, "PxVehicle4WEnable3WTadpoleMode requires the vectors set in PxVehicleSetBasisVectors to be axis-aligned");

	enable3WMode(rightDirection, upDirection, true, wheelsSimData, wheelsDynData, driveSimData);
}

void computeSprungMasses(const PxU32 numSprungMasses, const PxVec3* sprungMassCoordinates, const PxVec3& centreOfMass, const PxReal totalMass, const PxU32 gravityDirection, PxReal* sprungMasses);

void PxVehicleComputeSprungMasses(const PxU32 numSprungMasses, const PxVec3* sprungMassCoordinates, const PxVec3& centreOfMass, const PxReal totalMass, const PxU32 gravityDirection, PxReal* sprungMasses)
{
	computeSprungMasses(numSprungMasses, sprungMassCoordinates, centreOfMass, totalMass, gravityDirection, sprungMasses);
}

void PxVehicleCopyDynamicsData(const PxVehicleCopyDynamicsMap& wheelMap, const PxVehicleWheels& src, PxVehicleWheels* trg)
{
	PX_CHECK_AND_RETURN(trg, "PxVehicleCopyDynamicsData requires that trg is a valid vehicle pointer");

	PX_CHECK_AND_RETURN(src.getVehicleType() == trg->getVehicleType(), "PxVehicleCopyDynamicsData requires that both src and trg are the same type of vehicle");

#if PX_CHECKED
	{
		const PxU32 numWheelsSrc = src.mWheelsSimData.getNbWheels();
		const PxU32 numWheelsTrg = trg->mWheelsSimData.getNbWheels();
		PxU8 copiedWheelsSrc[PX_MAX_NB_WHEELS];
		PxMemZero(copiedWheelsSrc, sizeof(PxU8) * PX_MAX_NB_WHEELS);
		PxU8 setWheelsTrg[PX_MAX_NB_WHEELS];
		PxMemZero(setWheelsTrg, sizeof(PxU8) * PX_MAX_NB_WHEELS);
		for(PxU32 i = 0; i < PxMin(numWheelsSrc, numWheelsTrg); i++)
		{
			const PxU32 srcWheelId = wheelMap.sourceWheelIds[i];
			PX_CHECK_AND_RETURN(srcWheelId < numWheelsSrc, "PxVehicleCopyDynamicsData - wheelMap contains illegal source wheel id");
			PX_CHECK_AND_RETURN(0 == copiedWheelsSrc[srcWheelId], "PxVehicleCopyDynamicsData - wheelMap contains illegal source wheel id");
			copiedWheelsSrc[srcWheelId] = 1;

			const PxU32 trgWheelId = wheelMap.targetWheelIds[i];
			PX_CHECK_AND_RETURN(trgWheelId < numWheelsTrg, "PxVehicleCopyDynamicsData - wheelMap contains illegal target wheel id");
			PX_CHECK_AND_RETURN(0 == setWheelsTrg[trgWheelId], "PxVehicleCopyDynamicsData - wheelMap contains illegal target wheel id");
			setWheelsTrg[trgWheelId]=1;
		}
	}
#endif


	const PxU32 numWheelsSrc = src.mWheelsSimData.getNbWheels();
	const PxU32 numWheelsTrg = trg->mWheelsSimData.getNbWheels();

	//Set all dynamics data on the target to zero.
	//Be aware that setToRestState sets the rigid body to 
	//rest so set the momentum back after calling setToRestState.
	PxRigidDynamic* actorTrg = trg->getRigidDynamicActor();
	PxVec3 linVel = actorTrg->getLinearVelocity();
	PxVec3 angVel = actorTrg->getAngularVelocity();
	switch(src.getVehicleType())
	{
	case PxVehicleTypes::eDRIVE4W:
		static_cast<PxVehicleDrive4W*>(trg)->setToRestState();
		break;
	case PxVehicleTypes::eDRIVENW:
		static_cast<PxVehicleDriveNW*>(trg)->setToRestState();
		break;
	case PxVehicleTypes::eDRIVETANK:
		static_cast<PxVehicleDriveTank*>(trg)->setToRestState();
		break;
	case PxVehicleTypes::eNODRIVE:
		static_cast<PxVehicleNoDrive*>(trg)->setToRestState();
		break;
	default:
		break;
	}
	actorTrg->setLinearVelocity(linVel);
	actorTrg->setAngularVelocity(angVel);


	//Keep a track of the wheels on trg that have their dynamics data set as a copy from src.
	PxU8 setWheelsTrg[PX_MAX_NB_WHEELS];
	PxMemZero(setWheelsTrg, sizeof(PxU8) * PX_MAX_NB_WHEELS);

	//Keep a track of the average wheel rotation speed of all enabled wheels on src.
	PxU32 numEnabledWheelsSrc = 0;
	PxF32 accumulatedWheelRotationSpeedSrc = 0.0f;

	//Copy wheel dynamics data from src wheels to trg wheels.
	//Track the target wheels that have been given dynamics data from src wheels.
	//Compute the accumulated wheel rotation speed of all enabled src wheels.
	const PxU32 numMappedWheels = PxMin(numWheelsSrc, numWheelsTrg);
	for(PxU32 i = 0; i < numMappedWheels; i++)
	{
		const PxU32 srcWheelId = wheelMap.sourceWheelIds[i];
		const PxU32 trgWheelId = wheelMap.targetWheelIds[i];

		trg->mWheelsDynData.copy(src.mWheelsDynData, srcWheelId, trgWheelId);

		setWheelsTrg[trgWheelId] = 1;

		if(!src.mWheelsSimData.getIsWheelDisabled(srcWheelId))
		{
			numEnabledWheelsSrc++;
			accumulatedWheelRotationSpeedSrc += src.mWheelsDynData.getWheelRotationSpeed(srcWheelId);
		}
	}

	//Compute the average wheel rotation speed of src.
	PxF32 averageWheelRotationSpeedSrc = 0;
	if(numEnabledWheelsSrc > 0)
	{
		averageWheelRotationSpeedSrc = (accumulatedWheelRotationSpeedSrc/ (1.0f * numEnabledWheelsSrc));
	}

	//For wheels on trg that have not had their dynamics data copied from src just set
	//their wheel rotation speed to the average wheel rotation speed.
	for(PxU32 i = 0; i < numWheelsTrg; i++)
	{
		if(0 == setWheelsTrg[i] && !trg->mWheelsSimData.getIsWheelDisabled(i))
		{
			trg->mWheelsDynData.setWheelRotationSpeed(i, averageWheelRotationSpeedSrc);
		}
	}

	//Copy the engine rotation speed/gear states/autobox states/etc.
	switch(src.getVehicleType())
	{
	case PxVehicleTypes::eDRIVE4W:
	case PxVehicleTypes::eDRIVENW:
	case PxVehicleTypes::eDRIVETANK:
		{
			const PxVehicleDriveDynData& driveDynDataSrc = static_cast<const PxVehicleDrive&>(src).mDriveDynData;
			PxVehicleDriveDynData* driveDynDataTrg = &static_cast<PxVehicleDrive*>(trg)->mDriveDynData;
			*driveDynDataTrg = driveDynDataSrc;
		}
		break;
	default:
		break;
	}
}

bool areEqual(const PxQuat& q0, const PxQuat& q1)
{
	return ((q0.x == q1.x) && (q0.y == q1.y) && (q0.z == q1.z) && (q0.w == q1.w)); 
}

void PxVehicleUpdateCMassLocalPose(const PxTransform& oldCMassLocalPose, const PxTransform& newCMassLocalPose, const PxU32 gravityDirection, PxVehicleWheels* vehicle)
{
	PX_CHECK_AND_RETURN(areEqual(PxQuat(PxIdentity), oldCMassLocalPose.q), "Only center of mass poses with identity rotation are supported");
	PX_CHECK_AND_RETURN(areEqual(PxQuat(PxIdentity), newCMassLocalPose.q), "Only center of mass poses with identity rotation are supported");
	PX_CHECK_AND_RETURN(0==gravityDirection || 1==gravityDirection || 2==gravityDirection, "gravityDirection must be 0 or 1 or 2.");

	//Update the offsets from the rigid body center of mass.
	PxVec3 wheelCenterCMOffsets[PX_MAX_NB_WHEELS];
	const PxU32 nbWheels = vehicle->mWheelsSimData.getNbWheels();
	for(PxU32 i = 0; i < nbWheels; i++)
	{
		wheelCenterCMOffsets[i] = vehicle->mWheelsSimData.getWheelCentreOffset(i) + oldCMassLocalPose.p - newCMassLocalPose.p;
		vehicle->mWheelsSimData.setWheelCentreOffset(i, vehicle->mWheelsSimData.getWheelCentreOffset(i) + oldCMassLocalPose.p - newCMassLocalPose.p);
		vehicle->mWheelsSimData.setSuspForceAppPointOffset(i, vehicle->mWheelsSimData.getSuspForceAppPointOffset(i) + oldCMassLocalPose.p - newCMassLocalPose.p);
		vehicle->mWheelsSimData.setTireForceAppPointOffset(i, vehicle->mWheelsSimData.getTireForceAppPointOffset(i) + oldCMassLocalPose.p - newCMassLocalPose.p);
	}

	//Now update the sprung masses.
	PxF32 sprungMasses[PX_MAX_NB_WHEELS];
	PxVehicleComputeSprungMasses(nbWheels, wheelCenterCMOffsets, PxVec3(0,0,0), vehicle->getRigidDynamicActor()->getMass(), gravityDirection, sprungMasses);
	for(PxU32 i = 0; i < nbWheels; i++)
	{
		PxVehicleSuspensionData suspData = vehicle->mWheelsSimData.getSuspensionData(i);
		const PxF32 massRatio = sprungMasses[i]/suspData.mSprungMass;
		suspData.mSprungMass = sprungMasses[i];
		suspData.mSpringStrength *= massRatio;
		suspData.mSpringDamperRate *= massRatio;
		vehicle->mWheelsSimData.setSuspensionData(i, suspData);
	}
}

}//physx
