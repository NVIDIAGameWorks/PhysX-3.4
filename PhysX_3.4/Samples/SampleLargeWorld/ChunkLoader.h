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


#ifndef CHUNKLOADER_H
#define CHUNKLOADER_H

#include "PsMutex.h"
#include "PsThread.h"
#include "PhysXSample.h"

#include "PsTime.h"

#define ENABLE_PROGRESS_BAR 1

typedef PxU16 CoordType;
typedef PxU32 IDType;

#define N1			7
//The terrain has 16*16 grids 
#define	N2			256

//The width of each terrain chunk, this is the hardcoded number accroding to Terrain.bin
#define CHUNK_WIDTH 256.0f

#define MATERIAL_ROAD_GRASS		1003
#define MATERIAL_BRICKS			1004
#define MATERIAL_BUILDING		1005
#define MATERIAL_FARM			1006
#define MATERIAL_TREE			1007

PX_FORCE_INLINE PxShape* getShape(const PxRigidActor& actor)
{
	PX_ASSERT(actor.getNbShapes() >= 1);
	PxShape* shape = NULL;
	actor.getShapes(&shape, 1);
	return shape;
}

typedef union 
{
	struct{ 
		CoordType x, y;
	} coord;
	IDType id;

}ChunkID;

static PX_INLINE bool operator < (const ChunkID& _Left, const ChunkID& _Right) 
{
	return _Left.id < _Right.id;
}
static PX_INLINE bool operator == (const ChunkID& _Left, PxU32 _Right) 
{
	return _Left.id == _Right;
}

static PX_INLINE bool operator == (const ChunkID& _Left, ChunkID _Right) 
{
	return _Left.id == _Right.id;
}

struct ChunkCommandType
{
	enum Enum
	{
		eAdd,
		eRemove
	};
};

struct ChunkCommand
{
	ChunkCommand(ChunkCommandType::Enum _type, const ChunkID& _id) 
	{ 
		type = _type;
		id = _id;
	} 

	ChunkCommandType::Enum	type;
	ChunkID id;
};

struct DynamicObjects
{
	DynamicObjects(PxRigidDynamic* _actor, bool _isImportant)
	{
		actor = _actor;
		isImportant = _isImportant;
		id.id = 0;
	}

	PxRigidDynamic*		actor;
	bool				isImportant;
	ChunkID				id;
};

class SampleLargeWorld;
class BackgroundLoader : public SampleAllocateable
{
public:
	struct DeferredLoadedRenderData : public RAWMesh
	{
		DeferredLoadedRenderData():shape(NULL){}		
		//the shape to link with
		PxShape* shape;
	};
public:

	BackgroundLoader(SampleLargeWorld& sample, CoordType halfRange, CoordType terrainRange, PxF32 chunkWidth);
	~BackgroundLoader();
	
	void addDynamicObject(PxRigidDynamic* inActor, bool inIsImportant) 
	{
		mDyncActors.push_back(new DynamicObjects(inActor, inIsImportant)); 
		updateDynamicChunkId();
	}

	void onTick(); 
	void updateChunk(const PxVec3& cameraPos);
	
	//The lock for accessing streaming progress and timing status
	Ps::ReadWriteLock				mLoaderStatusLock;


#ifdef ENABLE_PROGRESS_BAR
	volatile PxU32					mQueryLength;
	volatile PxF32					mQueryProgress;
#endif

	volatile Ps::Time::Second		mDiskIOTime;
	volatile Ps::Time::Second		mPhyXStreamTime;
	volatile Ps::Time::Second		mGraphicStreamTime;

private:
	const char* getPathname( ChunkID id );
	
	void	serialize(PxCollection* collection, ChunkID id);
	
	void	addReadyChunkToScene(ChunkID id);
	void	destroyChunk(ChunkID id);
	
	void	updateDynamicChunkId();

	//Return NULL means already loaded or cannot load
	PxCollection* loadChunk(ChunkID id);

	void	createRenderObjectsFromCollection(PxCollection* collection);
	void	deleteLoadedRenderQueue();
	void	deleteCollections();

	void*	loaderThread();	
	static void* loaderThread(void* loader);
	
	DeferredLoadedRenderData* createRawMeshFromObjMesh(const char* name, const PxTransform& pos, DeferredLoadedRenderData& rawMesh);

private:

	struct CollectionMemory
	{
		CollectionMemory()
			: collection(NULL)
			, memory(NULL)
			, addToScene(false)
		{
			
		}

		PxCollection*	collection;
		void*			memory;
		bool			addToScene;
	};

	typedef std::map<ChunkID, CollectionMemory>	CollectionIdMap;
	CollectionIdMap					mCollectionIdMap;
	 
	std::vector<ChunkID>			mRequestQueue;
	std::vector<DynamicObjects*>	mDyncActors;
	std::vector<PxActor*>			mRemovingActors;

	//We calculate terrain's normals and store here
	std::vector<DeferredLoadedRenderData>	mLoadedRenderDataQueue;

	physx::shdfnd::Thread*			mLoaderThread;

	//The lock to access ChunkQueue arrays
	physx::shdfnd::Mutex			mQueueLock;

	//The lock to access mCollectionIdMap
	physx::shdfnd::Mutex			mCollectionLock;
	
	//A signal to inform background thread new jobs
	shdfnd::Sync		mRequestReady;

	SampleLargeWorld*	mSampleLargeWorld;
	PxPhysics&			mPhysics;
	PxScene&			mScene;
	PxMaterial&			mMaterial;
	
	ChunkID				mCurChunkId;
	CoordType			mHalfRange;
	CoordType			mTerrainRange;
	PxF32				mChunkWidth;
	
	//Internal Timing varialbe
	Ps::Time::Second	mDiskIOTimeCounter;
	Ps::Time::Second	mPhyXStreamTimeCounter;
	Ps::Time::Second	mGraphicStreamTimeCounter;
	PxBounds3			mLocalBounds;

	PxSerializationRegistry*		mSr;
	std::vector<ChunkCommand>		mChunkCmds;
}; 

#endif

