/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __IOFX_ACTOR_CPU_H__
#define __IOFX_ACTOR_CPU_H__

#include "Apex.h"
#include "IofxActor.h"
#include "InstancedObjectSimulationIntl.h"
#include "ResourceProviderIntl.h"
#include "ApexActor.h"
#include "IofxActorImpl.h"
#include "IofxSceneCPU.h"

#include "ModifierData.h"

namespace nvidia
{
namespace iofx
{

class Modifier;
class IofxAssetImpl;
class IofxManager;
class IofxActorCPU;
class IosObjectCpuData;

class TaskModifiers : public PxLightCpuTask
{
public:
	TaskModifiers(IofxActorCPU& owner) : mOwner(owner) {}
	const char* getName() const
	{
		return "IofxActorCPU::Modifiers";
	}
	void run();

protected:
	IofxActorCPU& mOwner;

private:
	TaskModifiers& operator=(const TaskModifiers&);
};

class IofxActorCPU : public IofxActorImpl
{
public:
	IofxActorCPU(ResID, IofxScene*, IofxManager& );
	~IofxActorCPU() {}

	void updateBounds();

	void runModifiers();


	template <typename Input, typename PublicState, typename PrivateState, typename OutputLayout>
	void updateParticles(const IosObjectCpuData& objData, const OutputLayout& outputLayout, const uint8_t* ptr);


	TaskModifiers	            mModifierTask;

	PxBounds3                   mWorkingBounds;
	ObjectRange					mWorkingRange;
	uint32_t						mWorkingVisibleCount;
};

template <typename T>
void FromSlices(IofxSlice** slices, uint32_t idx, T& val)
{
	size_t i = 0;
	IofxSlice* data = (IofxSlice*)&val;
	for (; i != sizeof(T) / sizeof(IofxSlice); ++i, ++data)
	{
		*data = slices[i][idx];
	}

	uint32_t tail = sizeof(T) % sizeof(IofxSlice);
	if (!tail)
	{
		return;
	}

	uint32_t* u32s = (uint32_t*)data;

	if (4 == tail)
	{
		u32s[0] = slices[i][idx].x;
	}
	else if (8 == tail)
	{
		u32s[0] = slices[i][idx].x;
		u32s[1] = slices[i][idx].y;
	}
	else if (12 == tail)
	{
		u32s[0] = slices[i][idx].x;
		u32s[1] = slices[i][idx].y;
		u32s[2] = slices[i][idx].z;
	}
	else
	{
		PX_ALWAYS_ASSERT();
	}
}

template <typename T>
void ToSlices(IofxSlice** slices, uint32_t idx, const T& val)
{
	size_t i = 0;
	const IofxSlice* data = (const IofxSlice*)&val;
	for (; i != sizeof(T) / sizeof(IofxSlice); ++i, ++data)
	{
		slices[i][idx] = *data;
	}

	uint32_t tail = sizeof(T) % sizeof(IofxSlice);
	if (!tail)
	{
		return;
	}

	const uint32_t* u32s = (const uint32_t*)data;

	if (4 == tail)
	{
		slices[i][idx].x = u32s[0];
	}
	else if (8 == tail)
	{
		slices[i][idx].x = u32s[0];
		slices[i][idx].y = u32s[1];
	}
	else if (12 == tail)
	{
		slices[i][idx].x = u32s[0];
		slices[i][idx].y = u32s[1];
		slices[i][idx].z = u32s[2];
	}
	else
	{
		PX_ALWAYS_ASSERT();
	}
}

template <typename Input, typename PublicState, typename PrivateState, typename OutputLayout>
void IofxActorCPU::updateParticles(const IosObjectCpuData& objData, const OutputLayout& outputLayout, const uint8_t* ptr)
{
	IofxSlice** inPubState = objData.inPubState;
	IofxSlice** inPrivState = objData.inPrivState;
	IofxSlice** outPubState = objData.outPubState;
	IofxSlice** outPrivState = objData.outPrivState;

	ModifierCommonParams common = objData.getCommonParams();

	RandState randState(mIofxScene->mApexScene->getSeed());

	for (uint32_t id = 0; id < mWorkingRange.objectCount; id++)
	{
		uint32_t outputID = mWorkingRange.startIndex + id;
		uint32_t stateID = objData.outputToState[ outputID ];
		PX_ASSERT(stateID != IosBufferDescIntl::NOT_A_PARTICLE);
		PX_ASSERT(stateID < objData.maxStateID);
		uint32_t inputID = objData.pmaInStateToInput->get(stateID);

		bool newParticle = false;
		if (inputID & IosBufferDescIntl::NEW_PARTICLE_FLAG)
		{
			newParticle = true;
			inputID &= ~IosBufferDescIntl::NEW_PARTICLE_FLAG;
		}

		const IofxActorIDIntl actorId = objData.pmaActorIdentifiers->get(inputID);
		const uint16_t actorClassId = actorId.getActorClassID();
		PX_ASSERT(actorClassId < mMgr.mActorClassTable.size());

		IofxManagerClient* iofxClient = mMgr.mActorClassTable[ actorClassId ].client;
		PX_ASSERT(iofxClient != NULL);
		IofxAssetSceneInstCPU* iofxAssetSceneInst = static_cast<IofxAssetSceneInstCPU*>(iofxClient->getAssetSceneInst());
		PX_ASSERT(iofxAssetSceneInst != NULL);

		Input input;
		input.load(objData, inputID);

		PublicState pubState;
		PrivateState privState;

		if (newParticle)
		{
			PublicState::initDefault(pubState, iofxClient->getParams().objectScale);
			PrivateState::initDefault(privState);

			for (uint32_t j = 0; j < iofxAssetSceneInst->mSpawnModifiersList.size(); j++)
			{
				const void* params = &iofxAssetSceneInst->mModifiersParamsBuffer[ iofxAssetSceneInst->mSpawnModifiersList[j].paramsOffset ];
				iofxAssetSceneInst->mSpawnModifiersList[j].updateFunc(params, input, pubState, privState, common, randState);
			}
		}
		else
		{
			PX_ASSERT(stateID < objData.maxObjectCount);

			FromSlices(inPubState, stateID, pubState);
			FromSlices(inPrivState, stateID, privState);
		}

		ToSlices(outPubState, outputID, pubState);

		for (uint32_t j = 0; j < iofxAssetSceneInst->mContinuousModifiersList.size(); j++)
		{
			const void* params = &iofxAssetSceneInst->mModifiersParamsBuffer[ iofxAssetSceneInst->mContinuousModifiersList[j].paramsOffset ];
			iofxAssetSceneInst->mContinuousModifiersList[j].updateFunc(params, input, pubState, privState, common, randState);
		}

		objData.pmaOutStateToInput->get(outputID) = inputID;
		ToSlices(outPrivState, outputID, privState);

		outputLayout.write(outputID, input, pubState, ptr);
	}
}

class ModifierParamsMapperCPUimpl;

class IofxAssetSceneInstCPU : public IofxAssetSceneInst
{
public:
	IofxAssetSceneInstCPU(IofxAssetImpl* asset, uint32_t semantics, IofxScene* scene);
	virtual ~IofxAssetSceneInstCPU() {}

	struct ModifierInfo
	{
		union
		{
			ModifierImpl::updateSpriteFunc updateSpriteFunc;
			ModifierImpl::updateMeshFunc updateMeshFunc;
		};
		void updateFunc(const void* params, const SpriteInput& input, SpritePublicState& pubState, SpritePrivateState& privState, const ModifierCommonParams& common, RandState& randState)
		{
			updateSpriteFunc(params, input, pubState, privState, common, randState);
		}
		void updateFunc(const void* params, const MeshInput& input, MeshPublicState& pubState, MeshPrivateState& privState, const ModifierCommonParams& common, RandState& randState)
		{
			updateMeshFunc(params, input, pubState, privState, common, randState);
		}

		uint32_t paramsOffset;
	};

	uint32_t introspectModifiers(ModifierParamsMapperCPUimpl& introspector, ModifierInfo* list, const ModifierStack& stack, ModifierStage stage, uint32_t usageClass);

	physx::Array<ModifierInfo>	mSpawnModifiersList;
	physx::Array<ModifierInfo>	mContinuousModifiersList;
	physx::Array<uint8_t>			mModifiersParamsBuffer;
};


}
} // namespace nvidia

#endif // __IOFX_ACTOR_CPU_H__
