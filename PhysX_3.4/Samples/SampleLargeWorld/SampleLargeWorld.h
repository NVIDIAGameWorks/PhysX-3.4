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


#ifndef SAMPLE_LARGE_WORLD_H
#define SAMPLE_LARGE_WORLD_H

#include "PhysXSample.h"
#include "ChunkLoader.h"
#include "characterkinematic/PxController.h"
#include "characterkinematic/PxControllerBehavior.h"
#include "PxTkRandom.h"


namespace physx
{
	class PxControllerManager;
}

class SampleCCTCameraController;
class ControlledActor;
class RenderBaseActor;
class BackgroundLoader;

typedef Ps::HashMap<const char*, RAWMesh, Ps::Hash<const char*>, Ps::RawAllocator> ObjMeshMap;

class SampleLargeWorld : public PhysXSample
	, public PxUserControllerHitReport, public PxControllerBehaviorCallback, public PxQueryFilterCallback
{
	friend class BackgroundLoader;
public:
											SampleLargeWorld(PhysXSampleApplication& app);
	virtual									~SampleLargeWorld();

	///////////////////////////////////////////////////////////////////////////////

	// Implements SampleApplication
	virtual	void							onInit();
    virtual	void						    onInit(bool restart) { onInit(); }
	virtual	void							onShutdown();
	virtual	void							onTickPreRender(PxF32 dtime);
	virtual void							onDigitalInputEvent(const SampleFramework::InputEvent& , bool val);

	//virtual void							onRelease(const PxBase* , void* , PxDeletionEventFlag::Enum ) {}
	///////////////////////////////////////////////////////////////////////////////

	// Implements PhysXSampleApplication	
	virtual	void							customizeSceneDesc(PxSceneDesc&);
	virtual	void							customizeSample(SampleSetup&);
	virtual	void							onSubstep(PxF32 dtime);	
	virtual void							collectInputEvents(std::vector<const SampleFramework::InputEvent*>& inputEvents);
	virtual void							helpRender(PxU32 x, PxU32 y, PxU8 textAlpha);
	virtual	void							descriptionRender(PxU32 x, PxU32 y, PxU8 textAlpha);
	virtual	void							customizeRender();
	virtual	void							newMesh(const RAWMesh&);

	///////////////////////////////////////////////////////////////////////////////

	// Implements PxUserControllerHitReport
	virtual void							onShapeHit(const PxControllerShapeHit& hit);
	virtual void							onControllerHit(const PxControllersHit& hit)		{}
	virtual void							onObstacleHit(const PxControllerObstacleHit& hit)	{}

	///////////////////////////////////////////////////////////////////////////////

	// Implements PxControllerBehaviorCallback
	virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxShape& shape, const PxActor& actor);
	virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxController& controller);
	virtual PxControllerBehaviorFlags		getBehaviorFlags(const PxObstacle& obstacle);

	///////////////////////////////////////////////////////////////////////////////

	// Implements PxQueryFilterCallback
	virtual	PxQueryHitType::Enum		preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags);
	virtual	PxQueryHitType::Enum		postFilter(const PxFilterData& filterData, const PxQueryHit& hit);

	///////////////////////////////////////////////////////////////////////////////

	// Implements PxSimulationFilterShader
	enum
	{
											CCD_FLAG = 1<<29,
	};

	static PxFilterFlags					filter(PxFilterObjectAttributes attributes0, 
														PxFilterData filterData0, 
														PxFilterObjectAttributes attributes1, 
														PxFilterData filterData1,
														PxPairFlags& pairFlags,
														const void* constantBlock,
														PxU32 constantBlockSize);

	///////////////////////////////////////////////////////////////////////////////
	
	//Create chunk_i_j file
	void									createChunk(CoordType coordX, CoordType coordY, PxCollection& outCollection);
	

private:
	RAWMesh*								createRawMeshFromMeshGeom(const PxTriangleMeshGeometry& mesh, RAWMesh &rawMesh);
	RAWMesh*								createRAWMeshFromObjMesh(const char* inObjFileName, const PxTransform& inPos, PxU32 inMaterialID, RAWMesh &outRawMesh);

	//Deep cope 
	void									cloneMesh(const RAWMesh& inSrc, RAWMesh& outDst);

	void									renderProgressBar(PxF32 ratio, bool needFlush = false);
	void									releaseJoints();
	bool									readyToSyncCCT();
	void									attachNearestObjectsToCCT();

	PxRigidDynamic*							createBox(const PxVec3& pos, const PxVec3& dims, const PxVec3* linVel,
														RenderMaterial* material, PxReal density);
	void									addWindMills(const PxTriangleMeshGeometry& meshGeom, PxToolkit::BasicRandom& random, 
														const PxBounds3& bound, PxCollection& outCollection);
	void									addTrees(const PxTriangleMeshGeometry& meshGeom, const PxBounds3& bound, PxCollection& outCollection);
	void									addCity(SampleArray<PxVec3>& terrainVertices, const PxBounds3& bound, PxCollection& outCollection);
	void									addFarm(SampleArray<PxVec3>& terrainVertices, const PxBounds3& bound, PxCollection& outCollection);
	void									addFortress(SampleArray<PxVec3>& terrainVertices, const PxBounds3& bound, PxCollection& outCollection);
	
	static void								setCCDActive(PxShape& shape);
	static bool								isCCDActive(PxFilterData& filterData);

	PxTriangleMesh*							generateTriMesh(const SampleArray<PxVec3>* vertices, const SampleArray<PxU32>* indices);

	void									setCollisionGroup(PxRigidActor* actor, PxU32 group);

	//Terrain template data from Terrain.bin
	struct BinData
	{
		SampleArray<SampleArray<PxVec3> >	mTerrainVertices;
		SampleArray<SampleArray<PxU32> >	mTerrainIndices;	

		CoordType							mDim;

		//Need parent to render progress bar
		void								serialize(SampleLargeWorld* parent, const char* terrainFile);

	}binData;

	ObjMeshMap								mRenderMeshCache;
	
	BackgroundLoader*						mBGLoader;
	SampleCCTCameraController*				mCCTCamera;
	ControlledActor*						mActor;
	PxControllerManager*					mControllerManager;
	PxExtendedVec3							mControllerInitialPosition;
	PxExtendedVec3							mLastCCTPosition;
	std::vector<PxFixedJoint*>				mFixedJoints;

	//Sky box
	RenderBaseActor*						mSkybox[5];

	//For progress bar renderering
	PxReal									mProgressBarRatio;
	Ps::Time::Second						mDiskIOTime;
	Ps::Time::Second						mPhysxStreaming;
	Ps::Time::Second						mGraphicStreaming;
	
	//Bound of the whole world
	PxBounds3								mWorldBound;

	bool									mReadyToSyncCCT;
	bool									mAddRenderActor;
	bool									mPick;
	bool									mKeyShiftDown;

	PxReal									mDefaultCameraSpeed;

public:
	static const PxU32						DEFAULT_COLLISION_GROUP = 0;
	static const PxU32						FAN_COLLISION_GROUP		= 1;
	static const PxU32						PICKING_COLLISION_GROUP = 2;

	PxStringTable*							mStringTable;
};

#endif
