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

#ifndef SAMPLE_VEHICLE_VEHICLE_MANAGER_H
#define SAMPLE_VEHICLE_VEHICLE_MANAGER_H

#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleDrive4W.h"
#include "vehicle/PxVehicleDriveNW.h"
#include "vehicle/PxVehicleDriveTank.h"
#include "vehicle/PxVehicleUpdate.h"
#include "foundation/PxFoundation.h"

#if PX_DEBUG_VEHICLE_ON
#include "vehicle/PxVehicleUtilTelemetry.h"
#endif

using namespace physx;

namespace physx
{
	class PxScene;
	class PxBatchQuery;
	class PxCooking;
	class PxMaterial;
	class PxConvexMesh;
	struct PxVehicleDrivableSurfaceType;
}

class SampleVehicleSceneQueryData;
class SampleVehicleWheelQueryResults;

class PxVehicle4WAlloc;

//Collision types and flags describing collision interactions of each collision type.
enum
{
	COLLISION_FLAG_GROUND			=	1 << 0,
	COLLISION_FLAG_WHEEL			=	1 << 1,
	COLLISION_FLAG_CHASSIS			=	1 << 2,
	COLLISION_FLAG_OBSTACLE			=	1 << 3,
	COLLISION_FLAG_DRIVABLE_OBSTACLE=	1 << 4,

	COLLISION_FLAG_GROUND_AGAINST	=															COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_WHEEL_AGAINST	=									COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_CHASSIS_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_OBSTACLE_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST=	COLLISION_FLAG_GROUND 						 |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};

//Id of drivable surface (used by suspension raycast filtering).
enum
{
	DRIVABLE_SURFACE_ID=0xffffffff
};

//Drivable surface types.
enum
{
	SURFACE_TYPE_MUD=0,
	SURFACE_TYPE_TARMAC,
	SURFACE_TYPE_SNOW,
	SURFACE_TYPE_GRASS,
	MAX_NUM_SURFACE_TYPES
};

struct SurfaceTypeNames
{
	static const char* getName(PxU32 type)
	{
		static char surfaceTypes[MAX_NUM_SURFACE_TYPES+1][64]=
		{
			"mud",
			"tarmac",
			"ice",
			"grass",
		};
		
		return surfaceTypes[type];
	}
};

//Tire types.
enum
{
	TIRE_TYPE_WETS=0,
	TIRE_TYPE_SLICKS,
	TIRE_TYPE_ICE,
	TIRE_TYPE_MUD,
	MAX_NUM_TIRE_TYPES
};

struct TireFrictionMultipliers
{
	static float getValue(PxU32 surfaceType, PxU32 tireType)
	{
		//Tire model friction for each combination of drivable surface type and tire type.
		static PxF32 tireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES]=
		{
			//WETS	SLICKS	ICE		MUD
			{0.95f,	0.95f,	0.95f,	0.95f},		//MUD
			{1.10f,	1.15f,	1.10f,	1.10f},		//TARMAC
			{0.70f,	0.70f,	0.70f,	0.70f},		//ICE
			{0.80f,	0.80f,	0.80f,	0.80f}		//GRASS
		};
		return tireFrictionMultipliers[surfaceType][tireType];
	}
};

class SampleVehicle_VehicleManager
{
public:

	enum
	{
		MAX_NUM_4W_VEHICLES=8,
		MAX_NUM_6W_VEHICLES=2,
		MAX_NUM_4W_TANKS=2
	};

	SampleVehicle_VehicleManager();
	~SampleVehicle_VehicleManager();

	void init(PxPhysics& physics, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes);

	void shutdown();

	//Create a vehicle ready to drive.
	void create4WVehicle
		(PxScene& scene, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
		 const PxF32 chassisMass, const PxVec3* wheelCentreOffsets4, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
		 const PxTransform& startTransform, const bool useAutoGearFlag);
	void create6WVehicle
		(PxScene& scene, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
		const PxF32 chassisMass, const PxVec3* wheelCentreOffsets4, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
		const PxTransform& startTransform, const bool useAutoGearFlag);
	void create4WTank
		(PxScene& scene, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
		 const PxF32 chassisMass, const PxVec3* wheelCentreOffsets4, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
		 const PxTransform& startTransform, const bool useAutoGearFlag, const PxVehicleDriveTankControlModel::Enum tankDriveModel);
	void create6WTank
		(PxScene& scene, PxPhysics& physics, PxCooking& cooking, const PxMaterial& material,
		 const PxF32 chassisMass, const PxVec3* wheelCentreOffsets4, PxConvexMesh* chassisConvexMesh, PxConvexMesh** wheelConvexMeshes4,
		 const PxTransform& startTransform, const bool useAutoGearFlag, const PxVehicleDriveTankControlModel::Enum tankDriveModel);

	PX_FORCE_INLINE	PxU32						getNbVehicles()					const	{ return mNumVehicles;		}
	PX_FORCE_INLINE	PxVehicleWheels*			getVehicle(const PxU32 i)				{ return mVehicles[i];		}
	PX_FORCE_INLINE const PxVehicleWheelQueryResult& getVehicleWheelQueryResults(const PxU32 i) const { return mVehicleWheelQueryResults[i]; }
					void						addVehicle(const PxU32 i, PxVehicleWheels* vehicle);		

	//Start the suspension raycasts (always call before calling update)
	void suspensionRaycasts(PxScene* scene);

	//Update vehicle dynamics and compute forces/torques to apply to sdk rigid bodies.
#if PX_DEBUG_VEHICLE_ON
	void updateAndRecordTelemetryData(const PxF32 timestep, const PxVec3& gravity, PxVehicleWheels* focusVehicleNW, PxVehicleTelemetryData* telemetryDataNW);
#else
	void update(const PxF32 timestep, const PxVec3& gravity);
#endif

	//Reset the car back to its rest state with a specified transform.
	void resetNWCar(const PxTransform& transform, const PxU32 vehicleId);

	//Switch the player's vehicle to 3-wheeled modes and back to 4-wheeled mode.
	void switchTo3WDeltaMode(const PxU32 vehicleId);
	void switchTo3WTadpoleMode(const PxU32 vehicleId);
	void switchTo4WMode(const PxU32 vehicleId);
	
	PxSerializationRegistry* getSerializationRegistry() { return mSerializationRegistry; }
private:

	//Array of all cars and report data for each car.
	PxVehicleWheels* mVehicles[MAX_NUM_4W_VEHICLES+MAX_NUM_6W_VEHICLES];
	PxVehicleWheelQueryResult mVehicleWheelQueryResults[MAX_NUM_4W_VEHICLES+MAX_NUM_6W_VEHICLES];
	PxU32 mNumVehicles;

	//sdk raycasts (for the suspension lines).
	SampleVehicleSceneQueryData* mSqData;
	PxBatchQuery* mSqWheelRaycastBatchQuery;

	//Reports for each wheel.
	SampleVehicleWheelQueryResults* mWheelQueryResults;

	//Cached simulation data of focus vehicle in 4W mode.
	PxVehicleWheelsSimData* mWheelsSimData4W;
	PxVehicleDriveSimData4W mDriveSimData4W;
	bool mIsIn3WMode;

	//Friction from combinations of tire and surface types.
	PxVehicleDrivableSurfaceToTireFrictionPairs* mSurfaceTirePairs;


	//Initialise a car back to its start transform and state.
	void resetNWCar(const PxTransform& startTransform, PxVehicleWheels* car);

	//Serialization
	PxSerializationRegistry*		mSerializationRegistry;
};

#endif //SAMPLE_VEHICLE_VEHICLE_MANAGER_H
