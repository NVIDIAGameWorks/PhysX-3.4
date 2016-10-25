/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_SCENE_CPU_H__
#define __BASIC_IOS_SCENE_CPU_H__

#if ENABLE_TEST
#include "BasicIosTestScene.h"
#endif
#include "BasicIosScene.h"

namespace nvidia
{
namespace basicios
{

#if ENABLE_TEST
#define BASIC_IOS_SCENE BasicIosTestScene
#else
#define BASIC_IOS_SCENE BasicIosScene
#endif

class BasicIosSceneCPU : public BASIC_IOS_SCENE
{
	class TimerCallback : public FieldSamplerCallbackIntl, public IofxManagerCallbackIntl, public UserAllocated
	{
		shdfnd::Time mTimer;
		float mMinTime, mMaxTime;
	public:
		TimerCallback() {}		
		void operator()(void* stream = NULL);
		void reset();
		float getElapsedTime() const;
	};
public:
	BasicIosSceneCPU(ModuleBasicIosImpl& module, SceneIntl& scene, RenderDebugInterface* debugRender, ResourceList& list);
	~BasicIosSceneCPU();

	virtual BasicIosActorImpl*		createIosActor(ResourceList& list, BasicIosAssetImpl& asset, nvidia::apex::IofxAsset& iofxAsset);

	virtual void				fetchInjectorParams(uint32_t injectorID, InjectorParams& injParams)
	{
		PX_ASSERT(injectorID < mInjectorParamsArray.size());
		injParams = mInjectorParamsArray[ injectorID ];
	}
	virtual void				updateInjectorParams(uint32_t injectorID, const InjectorParams& injParams)
	{
		PX_ASSERT(injectorID < mInjectorParamsArray.size());
		mInjectorParamsArray[ injectorID ] = injParams;
	}

	void							fetchResults();

protected:
	virtual void setCallbacks(BasicIosActorCPU* actor);
	virtual bool growInjectorStorage(uint32_t newSize)
	{
		mInjectorParamsArray.resize(newSize);
		return true;
	}

private:
	physx::Array<InjectorParams> mInjectorParamsArray;
	TimerCallback					mTimerCallback;

	friend class BasicIosActorCPU;
};

}
} // namespace nvidia

#endif // __BASIC_IOS_SCENE_H__
