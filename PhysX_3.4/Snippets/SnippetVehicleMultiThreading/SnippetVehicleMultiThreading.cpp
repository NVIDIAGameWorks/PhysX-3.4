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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates multi-threaded vehicles.
//
// It creates multiple vehicles on a plane and then concurrently simulates them
// in parallel across multiple threads.

// The concurrent vehicle simulation is split into four steps:
// 1. Suspension raycasts
// 2. Vehicle updates
// 3. Vehicle post-updates
// 4. SDK simulate

// Steps 1 and 2 above both make use of concurrency to improve performance.

// Step 3 above is an extra step that is necessary when PxVehicleUpdates is performed concurrently.
// The vehicle post-update step must be performed sequentially. 

// The 4 steps above are timed with profile zones.  The total accumulated time spent in each of 
// these steps is recorded and printed at the end of the simulation.  With PVD attached, profiles 
// can be further analyzed in the "Profile Zones" view in PVD.  

// The number of threads can be set by changing NUM_WORKER_THREADS, while the number of 
// vehicles can be set by changing NUM_VEHICLES.  The number of vehicles processed per task 
// can be modified by setting RAYCAST_BATCH_SIZE and UPDATE_BATCH_SIZE.  It is worthwhile 
// experimenting with these parameters to get a feel for the performance gains possible in 
// different scenarios and across various platforms.

// To avoid the overhead of PVD affecting the integrity of the profiling data the default
// behavior is for PVD to only record profile data.  
// Visualizing the scene will reduce the integrity of the performance numbers. 

// Performance statistics are only generated in "profile" build.
// PVD is not active at all in "release" build.

// ****************************************************************************

#include <ctype.h>

#include "PxPhysicsAPI.h"

#include "vehicle/PxVehicleUtil.h"

#include "../SnippetVehicleCommon/SnippetVehicleCreate.h"
#include "../SnippetVehicleCommon/SnippetVehicleSceneQuery.h"
#include "../SnippetVehicleCommon/SnippetVehicleFilterShader.h"
#include "../SnippetVehicleCommon/SnippetVehicleTireFriction.h"
#include "../SnippetVehicleCommon/SnippetVehicleWheelQueryResult.h"
#include "../SnippetVehicleCommon/SnippetVehicleConcurrency.h"

#include "../SnippetUtils/SnippetUtils.h"
#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"

#include "foundation/PxProfiler.h"


using namespace physx;
using namespace snippetvehicle;

PxDefaultAllocator			gAllocator;
PxDefaultErrorCallback		gErrorCallback;

PxFoundation*				gFoundation = NULL;
PxPhysics*					gPhysics	= NULL;

PxDefaultCpuDispatcher*		gDispatcher = NULL;
PxScene*					gScene		= NULL;

PxCooking*					gCooking	= NULL;

PxMaterial*					gMaterial	= NULL;

PxPvd*                      gPvd        = NULL;

#if 1
PxPvdInstrumentationFlags gConnectionFlags = PxPvdInstrumentationFlag::ePROFILE;
#else
PxPvdInstrumentationFlags gConnectionFlags = PxPvdInstrumentationFlag::eALL;
#endif

PxTaskManager*				gTaskManager = NULL;

PxRigidStatic*				gGroundPlane = NULL;

#define NUM_VEHICLES		1024
PxVehicleWheels*			gVehicles[NUM_VEHICLES];
PxBatchQuery*				gBatchQueries[NUM_VEHICLES];
PxVehicleDrivableSurfaceToTireFrictionPairs* gFrictionPairs = NULL;
VehicleSceneQueryData*		gVehicleSceneQueryData = NULL;
VehicleWheelQueryResults*	gVehicleWheelQueryResults = NULL;
VehicleConcurrency*			gVehicleConcurrency = NULL;

static const int gNumNames = 4;
static const char* gNames[gNumNames] =
{
	"concurrentVehicleRaycasts",
	"concurrentVehicleUpdates",
	"concurrentVehiclePostUpdates",
	"Basic.simulate"
};

struct ProfilerCallback : public physx::PxProfilerCallback
{
	PxU64 times[gNumNames];

	ProfilerCallback()
	{
		for (int i = 0; i < gNumNames; ++i)
			times[i] = 0;
	}

	~ProfilerCallback()
	{
		for (int i = 0; i < gNumNames; ++i)
		{
			float ms = SnippetUtils::getElapsedTimeInMilliseconds(times[i]);
			printf("%s: %f ms\n", gNames[i], PxF64(ms));
		}
	}

	virtual void* zoneStart(const char* eventName, bool, uint64_t)
	{
		for (int i = 0; i < gNumNames; ++i)
		{
			if (!strcmp(gNames[i], eventName))
			{
				times[i] -= SnippetUtils::getCurrentTimeCounterValue();
				break;
			}
		}
		return NULL;
	}
	virtual void zoneEnd(void* /*profilerData*/, const char* eventName, bool, uint64_t)
	{
		PxU64 time = SnippetUtils::getCurrentTimeCounterValue();

		for (int i = 0; i < gNumNames; ++i)
		{
			if (!strcmp(gNames[i], eventName))
			{
				times[i] += time;
				break;
			}
		}
	}
};
ProfilerCallback gProfilerCallback;

#define NUM_WORKER_THREADS 1

#define RAYCAST_BATCH_SIZE 1

#define UPDATE_BATCH_SIZE 1

VehicleDesc initVehicleDesc()
{
	//Set up the chassis mass, dimensions, moment of inertia, and center of mass offset.
	//The moment of inertia is just the moment of inertia of a cuboid but modified for easier steering.
	//Center of mass offset is 0.65m above the base of the chassis and 0.25m towards the front.
	const PxF32 chassisMass = 1500.0f;
	const PxVec3 chassisDims(2.5f,2.0f,5.0f);
	const PxVec3 chassisMOI
		((chassisDims.y*chassisDims.y + chassisDims.z*chassisDims.z)*chassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.z*chassisDims.z)*0.8f*chassisMass/12.0f,
		 (chassisDims.x*chassisDims.x + chassisDims.y*chassisDims.y)*chassisMass/12.0f);
	const PxVec3 chassisCMOffset(0.0f, -chassisDims.y*0.5f + 0.65f, 0.25f);

	//Set up the wheel mass, radius, width, moment of inertia, and number of wheels.
	//Moment of inertia is just the moment of inertia of a cylinder.
	const PxF32 wheelMass = 20.0f;
	const PxF32 wheelRadius = 0.5f;
	const PxF32 wheelWidth = 0.4f;
	const PxF32 wheelMOI = 0.5f*wheelMass*wheelRadius*wheelRadius;
	const PxU32 nbWheels = 6;

	VehicleDesc vehicleDesc;

	vehicleDesc.chassisMass = chassisMass;
	vehicleDesc.chassisDims = chassisDims;
	vehicleDesc.chassisMOI = chassisMOI;
	vehicleDesc.chassisCMOffset = chassisCMOffset;
	vehicleDesc.chassisMaterial = gMaterial;
	vehicleDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_CHASSIS, COLLISION_FLAG_CHASSIS_AGAINST, 0, 0);

	vehicleDesc.wheelMass = wheelMass;
	vehicleDesc.wheelRadius = wheelRadius;
	vehicleDesc.wheelWidth = wheelWidth;
	vehicleDesc.wheelMOI = wheelMOI;
	vehicleDesc.numWheels = nbWheels;
	vehicleDesc.wheelMaterial = gMaterial;
	vehicleDesc.chassisSimFilterData = PxFilterData(COLLISION_FLAG_WHEEL, COLLISION_FLAG_WHEEL_AGAINST, 0, 0);

	return vehicleDesc;
}

void initPhysics()
{
	/////////////////////////////////////////////
	//Initialise the sdk and scene
	/////////////////////////////////////////////

	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
	
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, gConnectionFlags);

	// PVD sets itself up as the profiler during the "connect" call above. We override this with
	// our own callback. If we wanted both our profiling and PVD's at the same time, we would
	// just call the PVD functions (available in PxPvd's base class) from our own profiler callback.
	PxSetProfilerCallback(&gProfilerCallback);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	
	gDispatcher = PxDefaultCpuDispatcherCreate(NUM_WORKER_THREADS);

	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= VehicleFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, false);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, false);
	}
	gCooking = 	PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));	

	/////////////////////////////////////////////
	//Create a task manager that will be used to 
	//update the vehicles concurrently across 
	//multiple threads.
	/////////////////////////////////////////////

	gTaskManager = PxTaskManager::createTaskManager(gFoundation->getErrorCallback(), gDispatcher);

	/////////////////////////////////////////////
	//Initialise the vehicle sdk and create 
	//vehicles that will drive on a plane
	/////////////////////////////////////////////

	PxInitVehicleSDK(*gPhysics);
	PxVehicleSetBasisVectors(PxVec3(0,1,0), PxVec3(0,0,1));
	PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

	//Create the batched scene queries for the suspension raycasts.
	gVehicleSceneQueryData = VehicleSceneQueryData::allocate(NUM_VEHICLES, PX_MAX_NB_WHEELS, 1, 1, WheelSceneQueryPreFilterBlocking , NULL, gAllocator);
	for(PxU32 i = 0; i < NUM_VEHICLES; i++)
	{
		gBatchQueries[i] = VehicleSceneQueryData::setUpBatchedSceneQuery(i, *gVehicleSceneQueryData, gScene);
	}

	//Create the friction table for each combination of tire and surface type.
	//For simplicity we only have a single surface type.
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	gFrictionPairs = createFrictionPairs(gMaterial);
	
	//Create a plane to drive on.
	PxFilterData groundPlaneSimFilterData(COLLISION_FLAG_GROUND, COLLISION_FLAG_GROUND_AGAINST, 0, 0);
	gGroundPlane = createDrivablePlane(groundPlaneSimFilterData, gMaterial, gPhysics);
	gScene->addActor(*gGroundPlane);

	//Create vehicles that will drive on the plane.
	for(PxU32 i = 0; i < NUM_VEHICLES; i++)
	{
		VehicleDesc vehicleDesc = initVehicleDesc();
		PxVehicleDrive4W* vehicle = createVehicle4W(vehicleDesc, gPhysics, gCooking);
		PxTransform startTransform(PxVec3(vehicleDesc.chassisDims.x*3.0f*i, (vehicleDesc.chassisDims.y*0.5f + vehicleDesc.wheelRadius + 1.0f), 0), PxQuat(PxIdentity));
		vehicle->getRigidDynamicActor()->setGlobalPose(startTransform);
		gScene->addActor(*vehicle->getRigidDynamicActor());

		//Set the vehicle to rest in first gear.
		//Set the vehicle to use auto-gears.
		vehicle->setToRestState();
		vehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		vehicle->mDriveDynData.setUseAutoGears(true);
	
		//Set each car to accelerate forwards
		vehicle->mDriveDynData.setAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL, 1.0f);

		gVehicles[i] = vehicle;
	}

	//Set up the wheel query results that are used to query the state of the vehicle after calling PxVehicleUpdates
	gVehicleWheelQueryResults = VehicleWheelQueryResults::allocate(NUM_VEHICLES, PX_MAX_NB_WHEELS, gAllocator);

	//Set up the data required for concurrent calls to PxVehicleUpdates
	gVehicleConcurrency = VehicleConcurrency::allocate(NUM_VEHICLES, PX_MAX_NB_WHEELS, gAllocator);

	//Set up the profile zones so that the advantages of parallelism can be measured in pvd.
}

//TaskVehicleRaycasts allows vehicle suspension raycasts to be performed concurrently across
//multiple threads.
class TaskVehicleRaycasts: public PxLightCpuTask
{
public:

	TaskVehicleRaycasts()
		: mThreadId(0xffffffff)
	{
	}

	void setThreadId(const PxU32 threadId)
	{
		mThreadId = threadId;
	}

	virtual void run()
	{
		PxU32 vehicleId = mThreadId*RAYCAST_BATCH_SIZE;
		while(vehicleId < NUM_VEHICLES)
		{
			const PxU32 numToRaycast = PxMin(NUM_VEHICLES - vehicleId, static_cast<PxU32>(RAYCAST_BATCH_SIZE));
			for(PxU32 i = 0; i < numToRaycast; i++)
			{
				PxVehicleWheels* vehicles[1] = {gVehicles[vehicleId + i]};
				PxBatchQuery* batchQuery = gBatchQueries[vehicleId + i];
				const PxU32 raycastQueryResultsSize = gVehicleSceneQueryData->getQueryResultBufferSize();
				PxRaycastQueryResult* raycastQueryResults = gVehicleSceneQueryData->getRaycastQueryResultBuffer(vehicleId + i);
				PxVehicleSuspensionRaycasts(batchQuery, 1, vehicles, raycastQueryResultsSize, raycastQueryResults);
			}
			vehicleId += NUM_WORKER_THREADS*RAYCAST_BATCH_SIZE;
		}
	}

	virtual const char* getName() const { return "TaskVehicleRaycasts"; }

private:

	PxU32 mThreadId;
};

//TaskVehicleUpdates allows vehicle updates to be performed concurrently across
//multiple threads.
class TaskVehicleUpdates: public PxLightCpuTask
{
public:

	TaskVehicleUpdates()
		: PxLightCpuTask(), 
		  mTimestep(0),
		  mGravity(PxVec3(0,0,0)),
		  mThreadId(0xffffffff)
	{
	}

	void setThreadId(const PxU32 threadId)
	{
		mThreadId = threadId;
	}

	void setTimestep(const PxF32 timestep)
	{
		mTimestep = timestep;
	}

	void setGravity(const PxVec3& gravity)
	{
		mGravity = gravity;
	}

	virtual void run()
	{
		PxU32 vehicleId = mThreadId*UPDATE_BATCH_SIZE;
		while(vehicleId < NUM_VEHICLES)
		{
			const PxU32 numToUpdate = PxMin(NUM_VEHICLES - vehicleId, static_cast<PxU32>(UPDATE_BATCH_SIZE));
			for(PxU32 i = 0; i < numToUpdate; i++)
			{
				PxVehicleWheels* vehicles[1] = {gVehicles[vehicleId +i]};
				PxVehicleWheelQueryResult* vehicleWheelQueryResults = gVehicleWheelQueryResults->getVehicleWheelQueryResults(vehicleId + i);
				PxVehicleConcurrentUpdateData* concurrentUpdates = gVehicleConcurrency->getVehicleConcurrentUpdate(vehicleId + i);
				PxVehicleUpdates(mTimestep, mGravity, *gFrictionPairs, 1, vehicles, vehicleWheelQueryResults, concurrentUpdates);
			}
			vehicleId += NUM_WORKER_THREADS*UPDATE_BATCH_SIZE;
		}
	}

	virtual const char* getName() const { return "TaskVehicleUpdates"; }

private:

	PxF32 mTimestep;
	PxVec3 mGravity;

	PxU32 mThreadId;
};

//TaskWait runs after all concurrent raycasts and updates have completed.
class TaskWait: public PxLightCpuTask
{
public:

	TaskWait(SnippetUtils::Sync* syncHandle)
		: PxLightCpuTask(),
		  mSyncHandle(syncHandle)
	{
	}

	virtual void run()
	{
	}

	PX_INLINE void release()
	{
		PxLightCpuTask::release();
		SnippetUtils::syncSet(mSyncHandle);
	}

	virtual const char* getName() const { return "TaskWait"; }

private:

	SnippetUtils::Sync* mSyncHandle;
};

void concurrentVehicleRaycasts()
{
	SnippetUtils::Sync* vehicleRaycastsComplete = SnippetUtils::syncCreate();
	SnippetUtils::syncReset(vehicleRaycastsComplete);

	//Create tasks that will update the vehicles concurrently then wait until all vehicles 
	//have completed their update.
	TaskWait taskWait(vehicleRaycastsComplete);
	TaskVehicleRaycasts taskVehicleRaycasts[NUM_WORKER_THREADS];
	for(PxU32 i = 0; i < NUM_WORKER_THREADS; i++)
	{
		taskVehicleRaycasts[i].setThreadId(i);
	}

	//Start the task manager.
	gTaskManager->resetDependencies();
	gTaskManager->startSimulation();

	//Start the profiler.
	PX_PROFILE_ZONE("concurrentVehicleRaycasts",0);

	//Update the raycasts concurrently then wait until all vehicles 
	//have completed their raycasts.
	taskWait.setContinuation(*gTaskManager, NULL);
	for(PxU32 i = 0; i < NUM_WORKER_THREADS; i++)
	{
		taskVehicleRaycasts[i].setContinuation(&taskWait);
	}
	taskWait.removeReference();
	for(PxU32 i = 0; i < NUM_WORKER_THREADS; i++)
	{
		taskVehicleRaycasts[i].removeReference();
	}

	//Wait for the signal that the work has been completed.
	SnippetUtils::syncWait(vehicleRaycastsComplete);

	//Release the sync handle
	SnippetUtils::syncRelease(vehicleRaycastsComplete);

}

void concurrentVehicleUpdates(const PxReal timestep)
{
	SnippetUtils::Sync* vehicleUpdatesComplete = SnippetUtils::syncCreate();
	SnippetUtils::syncReset(vehicleUpdatesComplete);

	//Create tasks that will update the vehicles concurrently then wait until all vehicles 
	//have completed their update.
	TaskWait taskWait(vehicleUpdatesComplete);
	TaskVehicleUpdates taskVehicleUpdates[NUM_WORKER_THREADS];
	for(PxU32 i = 0; i < NUM_WORKER_THREADS; i++)
	{
		taskVehicleUpdates[i].setThreadId(i);
		taskVehicleUpdates[i].setTimestep(timestep);
		taskVehicleUpdates[i].setGravity(gScene->getGravity());
	}

	//Start the task manager.
	gTaskManager->resetDependencies();
	gTaskManager->startSimulation();

	//Start the profiler.
	{
		PX_PROFILE_ZONE("concurrentVehicleUpdates",0);

		//Update the vehicles concurrently then wait until all vehicles 
		//have completed their update.
		taskWait.setContinuation(*gTaskManager, NULL);
		for(PxU32 i = 0; i < NUM_WORKER_THREADS; i++)
		{
			taskVehicleUpdates[i].setContinuation(&taskWait);
		}
		taskWait.removeReference();
		for(PxU32 i = 0; i < NUM_WORKER_THREADS; i++)
		{
			taskVehicleUpdates[i].removeReference();
		}

		//Wait for the signal that the work has been completed.
		SnippetUtils::syncWait(vehicleUpdatesComplete);

		//Release the sync handle
		SnippetUtils::syncRelease(vehicleUpdatesComplete);

		//End the profiler
	}

	//When PxVehicleUpdates is executed concurrently a secondary step is required to complete the 
	//update of the vehicles.
	PX_PROFILE_ZONE("concurrentVehiclePostUpdates",0);
	PxVehiclePostUpdates(gVehicleConcurrency->getVehicleConcurrentUpdateBuffer(), NUM_VEHICLES, gVehicles);
}


void stepPhysics()
{
	const PxF32 timestep = 1.0f/60.0f;

	//Concurrent vehicle raycasts.
	concurrentVehicleRaycasts();

	//Concurrent vehicle updates.
	concurrentVehicleUpdates(timestep);

	//Scene update.
	PX_PROFILE_ZONE("VehicleStepPhysics",0);
	gScene->simulate(timestep);
	gScene->fetchResults(true);
	}
	
void cleanupPhysics()
{
	//Clean up the vehicles and scene objects
	gVehicleConcurrency->free(gAllocator);
	gVehicleWheelQueryResults->free(gAllocator);
	for(PxU32 i = 0; i < NUM_VEHICLES;	i++)
	{
		gVehicles[i]->getRigidDynamicActor()->release();
		static_cast<PxVehicleDrive4W*>(gVehicles[i])->free();
	}
	gGroundPlane->release();
	gFrictionPairs->release();
	for(PxU32 i = 0; i < NUM_VEHICLES;	i++)
	{
		gBatchQueries[i]->release();
	}
	gVehicleSceneQueryData->free(gAllocator);
	PxCloseVehicleSDK();

	//Clean up the task manager used for concurrent vehicle updates.
	gTaskManager->release();

	//Clean up the scene and sdk.
	gMaterial->release();
	gCooking->release();
	gScene->release();
	gDispatcher->release();
	gPhysics->release();
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	gFoundation->release();

	printf("SnippetVehicleMultiThreading done.\n");
}

int snippetMain(int, const char*const*)
{
	printf("Initialising ... \n");
	initPhysics();

	printf("Simulating ... \n");
	for(PxU32 i = 0; i < 256; i++)
	{
		stepPhysics();
	}

	cleanupPhysics();

	return 0;
}
