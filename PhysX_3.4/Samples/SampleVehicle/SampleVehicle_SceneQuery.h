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

#ifndef SAMPLEVEHICLE_UTILSCENEQUERY_H
#define SAMPLEVEHICLE_UTILSCENEQUERY_H

#include "common/PxPhysXCommonConfig.h"
#include "vehicle/PxVehicleSDK.h"
#include "foundation/PxPreprocessor.h"
#include "PxScene.h"
#include "PxBatchQueryDesc.h"

using namespace physx;

//Make sure that suspension raycasts only consider shapes flagged as drivable that don't belong to the owner vehicle.
enum
{
	SAMPLEVEHICLE_DRIVABLE_SURFACE = 0xffff0000,
	SAMPLEVEHICLE_UNDRIVABLE_SURFACE = 0x0000ffff
};

static PxQueryHitType::Enum SampleVehicleWheelRaycastPreFilter(	
	PxFilterData filterData0, 
	PxFilterData filterData1,
	const void* constantBlock, PxU32 constantBlockSize,
	PxHitFlags& queryFlags)
{
	//filterData0 is the vehicle suspension raycast.
	//filterData1 is the shape potentially hit by the raycast.
	PX_UNUSED(queryFlags);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);
	PX_UNUSED(filterData0);
	return ((0 == (filterData1.word3 & SAMPLEVEHICLE_DRIVABLE_SURFACE)) ? PxQueryHitType::eNONE : PxQueryHitType::eBLOCK);
}


//Set up query filter data so that vehicles can drive on shapes with this filter data.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void SampleVehicleSetupDrivableShapeQueryFilterData(PxFilterData* qryFilterData);

//Set up query filter data so that vehicles cannot drive on shapes with this filter data.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void SampleVehicleSetupNonDrivableShapeQueryFilterData(PxFilterData* qryFilterData);

//Set up query filter data for the shapes of a vehicle to ensure that vehicles cannot drive on themselves 
//but can drive on the shapes of other vehicles.
//Note that we have reserved word3 of the PxFilterData for vehicle raycast query filtering.
void SampleVehicleSetupVehicleShapeQueryFilterData(PxFilterData* qryFilterData);

//Data structure for quick setup of scene queries for suspension raycasts.
class SampleVehicleSceneQueryData
{
public:

	//Allocate scene query data for up to maxNumWheels suspension raycasts.
	static SampleVehicleSceneQueryData* allocate(const PxU32 maxNumWheels);

	//Free allocated buffer for scene queries of suspension raycasts.
	void free();

	//Create a PxBatchQuery instance that will be used as a single batched raycast of multiple suspension lines of multiple vehicles
	PxBatchQuery* setUpBatchedSceneQuery(PxScene* scene);

	//Get the buffer of scene query results that will be used by PxVehicleNWSuspensionRaycasts
	PxRaycastQueryResult* getRaycastQueryResultBuffer() {return mSqResults;}

	//Get the number of scene query results that have been allocated for use by PxVehicleNWSuspensionRaycasts
	PxU32 getRaycastQueryResultBufferSize() const {return mNumQueries;}

	//Set the pre-filter shader 
	void setPreFilterShader(PxBatchQueryPreFilterShader preFilterShader) {mPreFilterShader=preFilterShader;}

private:

	//One result for each wheel.
	PxRaycastQueryResult* mSqResults;
	PxU32 mNbSqResults;

	//One hit for each wheel.
	PxRaycastHit* mSqHitBuffer;

	//Filter shader used to filter drivable and non-drivable surfaces
	PxBatchQueryPreFilterShader mPreFilterShader;

	//Maximum number of suspension raycasts that can be supported by the allocated buffers 
	//assuming a single query and hit per suspension line.
	PxU32 mNumQueries;

	void init()
	{
		mPreFilterShader=SampleVehicleWheelRaycastPreFilter;
	}

	SampleVehicleSceneQueryData()
	{
		init();
	}

	~SampleVehicleSceneQueryData()
	{
	}
};


#endif //SAMPLEVEHICLE_UTILSCENEQUERY_H
