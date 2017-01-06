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

#include "SceneVehicleSceneQuery.h"
#include "vehicle/PxVehicleSDK.h"
#include "PxFiltering.h"
#include "PsFoundation.h"
#include "PsUtilities.h"

#define CHECK_MSG(exp, msg) (!!(exp) || (physx::shdfnd::getFoundation().error(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, msg), 0) )
#define SIZEALIGN16(size) (((unsigned)(size)+15)&((unsigned)(~15)));

void VehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
	CHECK_MSG(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
	qryFilterData->word3 = (PxU32)VEHICLE_DRIVABLE_SURFACE;
}

void VehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData)
{
	CHECK_MSG(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
	qryFilterData->word3 = (PxU32)VEHICLE_UNDRIVABLE_SURFACE;
}

void VehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData)
{
	CHECK_MSG(0 == qryFilterData->word3, "word3 is reserved for filter data for vehicle raycast queries");
	qryFilterData->word3 = (PxU32)VEHICLE_UNDRIVABLE_SURFACE;
}

VehicleSceneQueryData* VehicleSceneQueryData::allocate(const PxU32 maxNumWheels)
{
	
	VehicleSceneQueryData* sqData = (VehicleSceneQueryData*)PX_ALLOC(sizeof(VehicleSceneQueryData), "PxVehicleNWSceneQueryData");
	sqData->init();
	sqData->mSqResults = (PxRaycastQueryResult*)PX_ALLOC(sizeof(PxRaycastQueryResult)*maxNumWheels, "PxRaycastQueryResult");
	sqData->mNbSqResults = maxNumWheels;
	sqData->mSqHitBuffer = (PxRaycastHit*)PX_ALLOC(sizeof(PxRaycastHit)*maxNumWheels, "PxRaycastHit");
	sqData->mNumQueries = maxNumWheels;

	sqData->mSqSweepResults = (PxSweepQueryResult*)PX_ALLOC(sizeof(PxSweepQueryResult) * maxNumWheels, "PxSweepQueryResult");
	sqData->mNbSqSweepResults = maxNumWheels;
	sqData->mSqSweepHitBuffer = (PxSweepHit*)PX_ALLOC(sizeof(PxSweepHit) * maxNumWheels, "PxSweepHit");
	sqData->mNumSweepQueries = maxNumWheels;

	return sqData;
}

void VehicleSceneQueryData::free()
{
	PX_FREE(this);
}

PxBatchQuery* VehicleSceneQueryData::setUpBatchedSceneQuery(PxScene* scene)
{
	PxBatchQueryDesc sqDesc(mNbSqResults, 0, 0);
	sqDesc.queryMemory.userRaycastResultBuffer = mSqResults;
	sqDesc.queryMemory.userRaycastTouchBuffer = mSqHitBuffer;
	sqDesc.queryMemory.raycastTouchBufferSize = mNumQueries;
	sqDesc.preFilterShader = mPreFilterShader;
	return scene->createBatchQuery(sqDesc);
}

PxBatchQuery* VehicleSceneQueryData::setUpBatchedSceneQuerySweep(PxScene* scene)
{
	PxBatchQueryDesc sqDesc(0, mNbSqResults, 0);
	sqDesc.queryMemory.userSweepResultBuffer = mSqSweepResults;
	sqDesc.queryMemory.userSweepTouchBuffer = mSqSweepHitBuffer;
	sqDesc.queryMemory.sweepTouchBufferSize = mNumQueries;
	sqDesc.preFilterShader = mPreFilterShader;
	return scene->createBatchQuery(sqDesc);
}


