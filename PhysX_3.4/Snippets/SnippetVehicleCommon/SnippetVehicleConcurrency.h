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

#ifndef SNIPPET_VEHICLE_CONCURRENCY_H
#define SNIPPET_VEHICLE_CONCURRENCY_H

#include "PxPhysicsAPI.h"
#include <new>

namespace snippetvehicle
{
using namespace physx;

//Data structure for quick setup of wheel query data structures.
class VehicleConcurrency
{
public:

	VehicleConcurrency()
		: mMaxNumVehicles(0),
		  mMaxNumWheelsPerVehicle(0),
		  mVehicleConcurrentUpdates(NULL)
	{
	}

	~VehicleConcurrency()
	{
	}

	static VehicleConcurrency* allocate(const PxU32 maxNumVehicles, const PxU32 maxNumWheelsPerVehicle, PxAllocatorCallback& allocator)
	{
		const PxU32 byteSize = 
			sizeof(VehicleConcurrency) + 
			sizeof(PxVehicleConcurrentUpdateData)*maxNumVehicles + 
			sizeof(PxVehicleWheelConcurrentUpdateData)*maxNumWheelsPerVehicle*maxNumVehicles;

		PxU8* buffer = static_cast<PxU8*>(allocator.allocate(byteSize, NULL, NULL, 0));

		VehicleConcurrency* vc = reinterpret_cast<VehicleConcurrency*>(buffer);
		new(vc) VehicleConcurrency();
		buffer += sizeof(VehicleConcurrency);

		vc->mMaxNumVehicles = maxNumVehicles;
		vc->mMaxNumWheelsPerVehicle = maxNumWheelsPerVehicle;

		vc->mVehicleConcurrentUpdates = reinterpret_cast<PxVehicleConcurrentUpdateData*>(buffer);
		buffer += sizeof(PxVehicleConcurrentUpdateData)*maxNumVehicles;

		for(PxU32 i=0;i<maxNumVehicles;i++)
		{
			new(vc->mVehicleConcurrentUpdates + i) PxVehicleConcurrentUpdateData();
			
			vc->mVehicleConcurrentUpdates[i].nbConcurrentWheelUpdates = maxNumWheelsPerVehicle;
			
			vc->mVehicleConcurrentUpdates[i].concurrentWheelUpdates = reinterpret_cast<PxVehicleWheelConcurrentUpdateData*>(buffer);
			buffer += sizeof(PxVehicleWheelConcurrentUpdateData)*maxNumWheelsPerVehicle;

			for(PxU32 j = 0; j < maxNumWheelsPerVehicle; j++)
			{
				new(vc->mVehicleConcurrentUpdates[i].concurrentWheelUpdates + j) PxVehicleWheelConcurrentUpdateData();
			}

		}


		return vc;
	}

	//Free allocated buffer for scene queries of suspension raycasts.
	void free(PxAllocatorCallback& allocator)
	{
		allocator.deallocate(this);
	}

	//Return the PxVehicleConcurrentUpdate for a vehicle specified by an index.
	PxVehicleConcurrentUpdateData* getVehicleConcurrentUpdate(const PxU32 id)
	{
		return (mVehicleConcurrentUpdates + id);
	}

	//Return the entire array of PxVehicleConcurrentUpdates
	PxVehicleConcurrentUpdateData* getVehicleConcurrentUpdateBuffer()
	{
		return mVehicleConcurrentUpdates;
	}

private:

	PxU32 mMaxNumVehicles;
	PxU32 mMaxNumWheelsPerVehicle;
	PxVehicleConcurrentUpdateData* mVehicleConcurrentUpdates;
};

} // namespace snippetvehicle

#endif //SNIPPET_VEHICLE_CONCURRENCY_H
