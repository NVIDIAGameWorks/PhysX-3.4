/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_MANAGER_H__
#define __IOFX_MANAGER_H__

#include "PsArray.h"
#include "PsHashMap.h"
#include "SceneIntl.h"
#include "IofxManagerIntl.h"
#include "ApexActor.h"

#include "ModifierData.h"

#include "PxTask.h"
#include "ApexMirroredArray.h"

namespace nvidia
{
namespace iofx
{

class IofxScene;
class IofxManager;
class IosObjectBaseData;
class IofxAssetImpl;
class IofxActorImpl;
class RenderVolumeImpl;
class IofxSharedRenderData;

class TaskUpdateEffects : public PxTask
{
public:
	TaskUpdateEffects(IofxManager& owner) : mOwner(owner) {}
	const char* getName() const
	{
		return "IofxManager::UpdateEffects";
	}
	void run();
protected:
	IofxManager& mOwner;

private:
	TaskUpdateEffects& operator=(const TaskUpdateEffects&);
};

class IofxAssetImpl;

class IofxSceneInst : public UserAllocated
{
public:
	virtual ~IofxSceneInst() {}

	uint32_t getRefCount() const
	{
		return _refCount;
	}
	void addRef()
	{
		++_refCount;
	}
	bool removeRef()
	{
		PX_ASSERT(_refCount > 0);
		return (--_refCount == 0);
	}

protected:
	IofxSceneInst() : _refCount(0) {}

private:
	uint32_t		_refCount;
};

class IofxAssetSceneInst : public IofxSceneInst
{
public:
	IofxAssetSceneInst(IofxAssetImpl* asset, uint32_t semantics)
		: _asset(asset), _semantics(semantics)
	{
	}
	virtual ~IofxAssetSceneInst()
	{
	}

	PX_INLINE IofxAssetImpl* getAsset() const
	{
		return _asset;
	}
	PX_INLINE uint32_t getSemantics() const
	{
		return _semantics;
	}

protected:
	IofxAssetImpl*	_asset;
	uint32_t		_semantics;
};

class IofxManagerClient : public IofxManagerClientIntl, public UserAllocated
{
public:
	IofxManagerClient(IofxAssetSceneInst* assetSceneInst, uint32_t actorClassID, const IofxManagerClientIntl::Params& params)
		: _assetSceneInst(assetSceneInst), _actorClassID(actorClassID), _params(params)
	{
	}
	virtual ~IofxManagerClient()
	{
	}

	PX_INLINE IofxAssetSceneInst* getAssetSceneInst() const
	{
		PX_ASSERT(_assetSceneInst != NULL);
		return _assetSceneInst;
	}

	PX_INLINE uint32_t getActorClassID() const
	{
		return _actorClassID;
	}

	PX_INLINE const IofxManagerClientIntl::Params& getParams() const
	{
		return _params;
	}

	// IofxManagerClientIntl interface
	virtual void getParams(IofxManagerClientIntl::Params& params) const
	{
		params = _params;
	}
	virtual void setParams(const IofxManagerClientIntl::Params& params)
	{
		_params = params;
	}

protected:
	IofxAssetSceneInst*			_assetSceneInst;
	uint32_t						_actorClassID;
	IofxManagerClientIntl::Params	_params;
};

class IofxActorSceneInst : public IofxSceneInst
{
public:
	IofxActorSceneInst(ResID renderResID)
		: _renderResID(renderResID)
	{
	}
	virtual ~IofxActorSceneInst()
	{
	}

	PX_INLINE ResID getRenderResID() const
	{
		return _renderResID;
	}

	void addAssetSceneInst(IofxAssetSceneInst* value)
	{
		_assetSceneInstArray.pushBack(value);
	}
	bool removeAssetSceneInst(IofxAssetSceneInst* value)
	{
		return _assetSceneInstArray.findAndReplaceWithLast(value);
	}
	const physx::Array<IofxAssetSceneInst*>& getAssetSceneInstArray() const
	{
		return _assetSceneInstArray;
	}

protected:
	ResID _renderResID;
	physx::Array<IofxAssetSceneInst*> _assetSceneInstArray;
};

class CudaPipeline
{
public:
	virtual ~CudaPipeline() {}
	virtual void release() = 0;
	virtual void fetchResults() = 0;
	virtual void submitTasks() = 0;

	virtual PxTaskID launchGpuTasks() = 0;
	virtual void launchPrep() = 0;

	virtual IofxManagerClient* createClient(IofxAssetSceneInst* assetSceneInst, uint32_t actorClassID, const IofxManagerClientIntl::Params& params) = 0;
	virtual IofxAssetSceneInst* createAssetSceneInst(IofxAssetImpl* asset, uint32_t semantics) = 0;
};


class IofxManager : public IofxManagerIntl, public ApexResourceInterface, public ApexResource, public ApexContext
{
public:
	IofxManager(IofxScene& scene, const IofxManagerDescIntl& desc, bool isMesh);
	~IofxManager();

	void destroy();

	/* Over-ride this ApexContext method to capture IofxActorImpl deletion events */
	void removeActorAtIndex(uint32_t index);

	void createSimulationBuffers(IosBufferDescIntl& outDesc);
	void setSimulationParameters(float radius, const PxVec3& up, float gravity, float restDensity);
	void updateEffectsData(float deltaTime, uint32_t numObjects, uint32_t maxInputID, uint32_t maxStateID, void* extraData);
	virtual void submitTasks();
	virtual void fetchResults();
	void release();
	void outputHostToDevice(PxGpuCopyDescQueue& copyQueue);
	PxTaskID getUpdateEffectsTaskID(PxTaskID);
	void cpuModifiers();
	PxBounds3 getBounds() const;
	void swapStates();

	uint32_t getActorID(IofxAssetSceneInst* assetSceneInst, uint16_t meshID);
	void releaseActorID(IofxAssetSceneInst* assetSceneInst, uint32_t actorID);

	uint16_t getActorClassID(IofxManagerClientIntl* client, uint16_t meshID);

	IofxManagerClientIntl* createClient(nvidia::apex::IofxAsset* asset, const IofxManagerClientIntl::Params& params);
	void releaseClient(IofxManagerClientIntl* client);

	uint16_t	getVolumeID(RenderVolume* vol);
	PX_INLINE uint32_t	getSimulatedParticlesCount() const
	{
		return mLastNumObjects;
	}

	PX_INLINE void setOnStartCallback(IofxManagerCallbackIntl* callback)
	{
		if (mOnStartCallback) 
		{
			PX_DELETE(mOnStartCallback);
		}
		mOnStartCallback = callback;
	}
	PX_INLINE void setOnFinishCallback(IofxManagerCallbackIntl* callback)
	{
		if (mOnFinishCallback) 
		{
			PX_DELETE(mOnFinishCallback);
		}
		mOnFinishCallback = callback;
	}

	PX_INLINE bool isMesh()
	{
		return mIsMesh;
	}

	uint32_t	    getListIndex() const
	{
		return m_listIndex;
	}
	void	            setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}

	float	getObjectRadius() const;

	IofxAssetSceneInst* 	createAssetSceneInst(IofxAsset* asset);

	void initIofxActor(IofxActorImpl* iofxActor, uint32_t mActorID, RenderVolumeImpl* renderVolume);


	typedef HashMap<IofxAssetImpl*, IofxAssetSceneInst*> AssetHashMap_t;
	AssetHashMap_t				mAssetHashMap;

	physx::Array<IofxActorSceneInst*> mActorTable;

	struct ActorClassData
	{
		IofxManagerClient*		client;  // NULL for empty rows
		uint16_t				meshid;
		uint16_t				count;
		uint32_t				actorID;
	};
	physx::Array<ActorClassData>	mActorClassTable;

	struct VolumeData
	{
		RenderVolumeImpl*			vol; // NULL for empty rows
		PxBounds3					mBounds;
		uint32_t					mPri;
		uint32_t					mFlags;
		physx::Array<IofxActorImpl*>	mActors; // Indexed by actorClassID
	};
	physx::Array<VolumeData>			mVolumeTable;
	physx::Array<uint32_t>				mCountPerActor;
	physx::Array<uint32_t>				mStartPerActor;
	physx::Array<uint32_t>				mBuildPerActor;
	physx::Array<uint32_t>				mOutputToState;
	PxTaskID						mPostUpdateTaskID;

	physx::Array<uint32_t>				mSortingKeys;

	physx::Array<uint32_t>				mVolumeActorClassBitmap;

	IofxScene*                          mIofxScene;
	physx::Array<IosObjectBaseData*>	mObjData;
	IosBufferDescIntl						mSimBuffers;
	ApexSimpleString					mIosAssetName;

	// reference pointers for IOFX actors, so they know which buffer
	// in in which mode.
	IosObjectBaseData* 					mWorkingIosData;
	IosObjectBaseData* 					mResultIosData;
	IosObjectBaseData*					mStagingIosData;

	enum RenderState
	{
		RENDER_WAIT_FOR_DATA_ALLOC = 0,
		RENDER_WAIT_FOR_FETCH_RESULT,
		RENDER_WAIT_FOR_NEW,
		RENDER_READY
	};
	RenderState							mRenderState;

	RenderInteropFlags::Enum			mInteropFlags;

	volatile uint32_t					mTargetSemantics;

	// Simulation storage, for CPU/GPU IOS
	ApexMirroredArray<PxVec4>			positionMass;
	ApexMirroredArray<PxVec4>			velocityLife;
	ApexMirroredArray<PxVec4>			collisionNormalFlags;
	ApexMirroredArray<float>			density;
	ApexMirroredArray<IofxActorIDIntl>	actorIdentifiers;
	ApexMirroredArray<uint32_t>			inStateToInput;
	ApexMirroredArray<uint32_t>			outStateToInput;

	ApexMirroredArray<uint32_t>			userData;

	IofxManagerCallbackIntl*				mOnStartCallback;
	IofxManagerCallbackIntl*				mOnFinishCallback;

	// Assets that were added on this frame (prior to simulate)
	physx::Array<const IofxAssetImpl*>		addedAssets;

	// Max size of public/private states over active (simulated) assets
	uint32_t							pubStateSize, privStateSize;

	// State data (CPU only)

	typedef ApexMirroredArray<IofxSlice> SliceArray;

	struct State
	{
		physx::Array<SliceArray*> slices; // Slices
		physx::Array<IofxSlice*> a, b; // Pointers to slices' halves
	};

	State								pubState;
	State								privState;

	uint32_t							mInStateOffset;
	uint32_t							mOutStateOffset;
	bool								mStateSwap;

	float                               mTotalElapsedTime;
	bool                                mIsMesh;
	bool                                mDistanceSortingEnabled;
	bool                                mCudaIos;
	bool                                mCudaModifiers;

	void								prepareRenderables();

	void								fillMapUnmapArraysForInterop(physx::Array<CUgraphicsResource> &, physx::Array<CUgraphicsResource> &);
	void								mapBufferResultsForInterop(bool, bool);

	CudaPipeline*                       mCudaPipeline;
	TaskUpdateEffects					mSimulateTask;

	PxBounds3							mBounds;

#if APEX_CUDA_SUPPORT
	PxGpuCopyDescQueue					mCopyQueue;
#endif

	uint32_t							mLastNumObjects;
	uint32_t							mLastMaxInputID;
};

#define DEFERRED_IOFX_ACTOR ((IofxActorImpl*)(1))

}
} // end namespace nvidia::apex

#endif // __IOFX_MANAGER_H__
