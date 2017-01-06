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


#ifndef SAMPLE_VEHICLE_H
#define SAMPLE_VEHICLE_H

#include "PhysXSample.h"
#include "SampleVehicle_ControlInputs.h"
#include "SampleVehicle_CameraController.h"
#include "SampleVehicle_VehicleController.h"
#include "SampleVehicle_VehicleManager.h"
#include "SampleVehicle_GameLogic.h"
#include "vehicle/PxVehicleTireFriction.h"

class SampleVehicle : public PhysXSample
{

public:

											SampleVehicle(PhysXSampleApplication& app);
	virtual									~SampleVehicle();


	///////////////////////////////////////////////////////////////////////////////

	// Implements RAWImportCallback
	virtual	void							newMesh(const RAWMesh&);

	///////////////////////////////////////////////////////////////////////////////

	// Implements SampleApplication
	virtual	void							onInit();
    virtual	void						    onInit(bool restart) { onInit(); }
	virtual	void							onShutdown();

	virtual	void							onTickPreRender(PxF32 dtime);
	virtual	void							onTickPostRender(PxF32 dtime);

	virtual void							onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);
	virtual void							onAnalogInputEvent(const SampleFramework::InputEvent& , float val);

	///////////////////////////////////////////////////////////////////////////////

	// Implements PhysXSampleApplication
	virtual	void							helpRender(PxU32 x, PxU32 y, PxU8 textAlpha);
	virtual	void							descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha);
	virtual	void							customizeSample(SampleSetup&);
	virtual	void							customizeSceneDesc(PxSceneDesc&);
	virtual	void							customizeRender();
	virtual	void							onSubstep(PxF32 dtime);	
	virtual void							collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);

private:

	SampleVehicle_ControlInputs		mControlInputs;
	SampleVehicle_CameraController	mCameraController;
	SampleVehicle_VehicleController	mVehicleController;


	//Terrain

	PxF32*							mTerrainVB;
	PxU32							mNbTerrainVerts;

	enum
	{
		MAX_NUM_INDEX_BUFFERS = 16
	};
	PxU32							mNbIB;
	PxU32*							mIB[MAX_NUM_INDEX_BUFFERS];
	PxU32							mNbTriangles[MAX_NUM_INDEX_BUFFERS];
	PxU32							mRenderMaterial[MAX_NUM_INDEX_BUFFERS];

	//Materials

	PxVehicleDrivableSurfaceType	mVehicleDrivableSurfaceTypes[MAX_NUM_INDEX_BUFFERS];
	PxMaterial*						mStandardMaterials[MAX_NUM_INDEX_BUFFERS];
	PxMaterial*						mChassisMaterialDrivable;
	PxMaterial*						mChassisMaterialNonDrivable;


	RenderMaterial*					mTerrainMaterial;
	RenderMaterial*					mRoadMaterial;
	RenderMaterial*					mRoadIceMaterial;
	RenderMaterial*					mRoadGravelMaterial;

	void							createStandardMaterials();

	enum eFocusVehicleType
	{
		ePLAYER_VEHICLE_TYPE_VEHICLE4W=0,
		ePLAYER_VEHICLE_TYPE_VEHICLE6W,
		ePLAYER_VEHICLE_TYPE_TANK4W,
		ePLAYER_VEHICLE_TYPE_TANK6W,
		eMAX_NUM_FOCUS_VEHICLE_TYPES
	};

	// Vehicles
	SampleVehicle_VehicleManager	mVehicleManager;
	std::vector<RenderMeshActor*>	mVehicleGraphics;
	PxU32							mPlayerVehicle;
	eFocusVehicleType				mPlayerVehicleType;
	PxVehicleDriveTankControlModel::Enum mTankDriveModel;

	const char*						getFocusVehicleName();
	void							createVehicles();

	PxU32							mTerrainSize;
	PxF32							mTerrainWidth;
	PxRigidActor*					mHFActor;

	void							createTrack(PxU32 size, PxF32 width, PxF32 chaos);
	void							createTerrain(PxU32 size, PxF32 width, PxF32 chaos);
	void							addRenderMesh(PxF32* verts, PxU32 nVerts, PxU32* indices, PxU32 mIndices, PxU32 matID);
	void							addMesh(PxRigidActor* actor, PxF32* verts, PxU32 nVerts, PxU32* indices, PxU32 mIndices, PxU32 materialIndex, const char* filename);
	void							createLandscapeMesh();

	//Obstacles

	void							createObstacles();
	PxRigidStatic* 					addStaticObstacle(const PxTransform& transform, const PxU32 numShapes, PxTransform* shapeTransforms, PxGeometry** shapeGeometries, PxMaterial** shapeMaterials);
	PxRigidDynamic*					addDynamicObstacle(const PxTransform& transform, const PxF32 mass, const PxU32 numShapes, PxTransform* transforms,  PxGeometry** geometries, PxMaterial** materials);
	PxRigidDynamic* 				addDynamicDrivableObstacle(const PxTransform& transform, const PxF32 mass, const PxU32 numShapes, PxTransform* transforms,  PxGeometry** geometries, PxMaterial** materials);
	void							createStack(PxU32 size, PxF32 boxSize, const PxVec3& pos, const PxQuat& quat);
	void							createWall(const PxU32 numHorizontalBoxes, const PxU32 numVerticalBoxes, const PxF32 boxSize, const PxVec3& pos, const PxQuat& quat);

	//Debug render

	bool							mHideScreenText;
	bool							mDebugRenderFlag;
#if PX_DEBUG_VEHICLE_ON
	PxU32							mDebugRenderActiveGraphChannelWheel;
	PxU32							mDebugRenderActiveGraphChannelEngine;
	PxVehicleTelemetryData*			mTelemetryData4W;
	PxVehicleTelemetryData*			mTelemetryData6W;
#endif
	void							setupTelemetryData();
	void							clearTelemetryData();

	void							drawWheels();
	void							drawVehicleDebug();
	void							drawHud();
	void							drawGraphsAndPrintTireSurfaceTypes(const PxVehicleWheels& focusVehicle, const PxVehicleWheelQueryResult& focusVehicleWheelQueryResults);
	void							drawFocusVehicleGraphsAndPrintTireSurfaces();

	//Waypoints
	SampleVehicleWayPoints			mWayPoints;
	bool							mFixCar;
	bool							mBackToStart;

	//3W and 4W modes
	bool							m3WModeIncremented;
	PxU32							m3WMode;

	PxF32							mForwardSpeedHud;	

#if defined(SERIALIZE_VEHICLE_BINARY)
	void*							mMemory;
#endif

	void							updateCameraController(const PxF32 dtime, PxScene& scene);
	void							updateVehicleController(const PxF32 dtime);
	void							updateVehicleManager(const PxF32 dtime, const PxVec3& gravity);

	void							resetFocusVehicleAtWaypoint();
	PxRigidDynamic*					getFocusVehicleRigidDynamicActor();
	bool							getFocusVehicleUsesAutoGears();
	char							mVehicleFilePath[256];
};

#endif
