/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "SceneIntl.h"
#include "Modifier.h"
#include "IofxActor.h"
#include "IofxActorCPU.h"
#include "IofxAssetImpl.h"
#include "IofxSceneCPU.h"
#include "IosObjectData.h"

#include "ModuleIofxImpl.h"

#include "ARLSort.h"


namespace nvidia
{
namespace iofx
{

void TaskModifiers::run()
{
	setProfileStat((uint16_t) mOwner.mWorkingRange.objectCount);
	mOwner.updateBounds();
	mOwner.runModifiers();
}


#pragma warning(disable: 4355) // 'this' : used in base member initializer list

IofxActorCPU::IofxActorCPU(ResID renderResID, IofxScene* iscene, IofxManager& mgr)
	: IofxActorImpl(renderResID, iscene, mgr)
	, mModifierTask(*this)
{
}

void IofxActorCPU::updateBounds()
{
	mWorkingBounds.setEmpty();

	const IosObjectCpuData& objData = *static_cast<const IosObjectCpuData*>(mMgr.mWorkingIosData);

	for (uint32_t id = 0; id < mWorkingRange.objectCount; id++)
	{
		uint32_t outputID = mWorkingRange.startIndex + id;

		uint32_t stateID = objData.outputToState[ outputID ];
		PX_ASSERT(stateID != IosBufferDescIntl::NOT_A_PARTICLE);
		PX_ASSERT(stateID < objData.maxStateID);

		uint32_t inputID = objData.pmaInStateToInput->get(stateID);
		PX_ASSERT(inputID != IosBufferDescIntl::NOT_A_PARTICLE);
		inputID &= ~IosBufferDescIntl::NEW_PARTICLE_FLAG;
		PX_ASSERT(inputID < objData.maxInputCount);

		// include particle in renderable bounds
		mWorkingBounds.include(objData.pmaPositionMass->get(inputID).getXYZ());
	}
}

PX_INLINE void MeshInput::load(const IosObjectBaseData& objData, uint32_t pos)
{
	position		= objData.pmaPositionMass->get(pos).getXYZ();
	mass			= objData.pmaPositionMass->get(pos).w;
	velocity		= objData.pmaVelocityLife->get(pos).getXYZ();
	liferemain		= objData.pmaVelocityLife->get(pos).w;
	density			= objData.pmaDensity ? objData.pmaDensity->get(pos) : 0;
	collisionNormal	= objData.pmaCollisionNormalFlags ? objData.pmaCollisionNormalFlags->get(pos).getXYZ() : PxVec3(0.0f);
	collisionFlags	= objData.pmaCollisionNormalFlags ? *(const uint32_t*)(&objData.pmaCollisionNormalFlags->get(pos).w) : 0;

	userData		= objData.pmaUserData ? objData.pmaUserData->get(pos) : 0;
}

PX_INLINE void SpriteInput::load(const IosObjectBaseData& objData, uint32_t pos)
{
	position		= objData.pmaPositionMass->get(pos).getXYZ();
	mass			= objData.pmaPositionMass->get(pos).w;
	velocity		= objData.pmaVelocityLife->get(pos).getXYZ();
	liferemain		= objData.pmaVelocityLife->get(pos).w;
	density			= objData.pmaDensity ? objData.pmaDensity->get(pos) : 0;

	userData		= objData.pmaUserData ? objData.pmaUserData->get(pos) : 0;
}


void IofxActorCPU::runModifiers()
{
	mWorkingVisibleCount = mWorkingRange.objectCount;

	IosObjectCpuData* obj = DYNAMIC_CAST(IosObjectCpuData*)(mMgr.mWorkingIosData);
	if (mMgr.mIsMesh)
	{
		IofxSharedRenderDataMeshImpl* meshRenderData = DYNAMIC_CAST(IofxSharedRenderDataMeshImpl*)(obj->renderData);
		const IofxMeshRenderLayout& meshRenderLayout = meshRenderData->getRenderLayout();

		MeshOutputLayout outputLayout;
		PX_COMPILE_TIME_ASSERT(sizeof(outputLayout.offsets) == sizeof(meshRenderLayout.offsets));
		::memcpy(outputLayout.offsets, meshRenderLayout.offsets, sizeof(outputLayout.offsets));	
		outputLayout.stride = meshRenderLayout.stride;

		updateParticles<MeshInput, MeshPublicState, MeshPrivateState>(*obj, 
																	outputLayout,
																	static_cast<uint8_t*>(meshRenderData->getBufferMappedPtr())
																	);
	}
	else
	{
		if (mDistanceSortingEnabled)
		{
			const ObjectRange& range = mWorkingRange;

			mWorkingVisibleCount = 0;
			PX_ASSERT(obj->sortingKeys != NULL);
			for (uint32_t outputId = range.startIndex; outputId < range.startIndex + range.objectCount ; outputId++)
			{
				uint32_t stateId = obj->outputToState[ outputId ];
				PX_ASSERT(stateId != IosBufferDescIntl::NOT_A_PARTICLE);
				uint32_t inputId = obj->pmaInStateToInput->get(stateId);
				inputId &= ~IosBufferDescIntl::NEW_PARTICLE_FLAG;

				const PxVec3 position = obj->pmaPositionMass->get(inputId).getXYZ();
				float sortDistance = obj->zNear + (obj->eyePosition - position).dot(obj->eyeDirection);

				//build uint key for radix sorting (flip float)
				uint32_t key = *reinterpret_cast<const uint32_t*>(&sortDistance);
				uint32_t mask = -int32_t(key >> 31) | 0x80000000;
				key ^= mask;

				obj->sortingKeys[ outputId ] = key;
				if ((key & 0x80000000) == 0) ++mWorkingVisibleCount;
			}

			uint32_t* keys = obj->sortingKeys + range.startIndex;
			uint32_t* values = obj->outputToState + range.startIndex;
			iofx::ARLSort<uint32_t, uint32_t>::sort(keys, values, (int32_t)range.objectCount);

			if (range.objectCount > 0)
			{
				PX_ASSERT(values[range.objectCount - 1] < obj->maxStateID);
			}
		}

		IofxSharedRenderDataSpriteImpl* spriteRenderData = DYNAMIC_CAST(IofxSharedRenderDataSpriteImpl*)(obj->renderData);
		const IofxSpriteRenderLayout& spriteRenderLayout = spriteRenderData->getRenderLayout();
		if (spriteRenderLayout.surfaceCount > 0)
		{
			SpriteTextureOutputLayout outputLayout;
			bool result = true;
			for (uint32_t i = 0; i < spriteRenderLayout.surfaceCount; ++i)
			{
				UserRenderSurface::MappedInfo mappedInfo;
				if (spriteRenderData->getSurfaceMappedInfo(i, mappedInfo))
				{
					const UserRenderSurfaceDesc& surfaceDesc = spriteRenderLayout.surfaceDescs[i];
					outputLayout.textureData[i].layout = static_cast<uint16_t>(spriteRenderLayout.surfaceElements[i]);

					outputLayout.texturePtr[i] = static_cast<uint8_t*>(mappedInfo.pData);

					const uint32_t surfaceWidth = uint32_t(surfaceDesc.width);
					const uint32_t surfacePitch = mappedInfo.rowPitch;

					PX_ASSERT(isPowerOfTwo(surfaceWidth));
					outputLayout.textureData[i].widthShift = static_cast<uint8_t>(highestSetBit(surfaceWidth));
					PX_ASSERT(isPowerOfTwo(surfacePitch));
					outputLayout.textureData[i].pitchShift = static_cast<uint8_t>(highestSetBit(surfacePitch));
				}
				else
				{
					result = false;
					break;
				}
			}
			outputLayout.textureCount = result ? spriteRenderLayout.surfaceCount : 0;

			updateParticles<SpriteInput, SpritePublicState, SpritePrivateState>(*obj, outputLayout, NULL);
		}
		else
		{
			SpriteOutputLayout outputLayout;
			/* Copy user-defined semantics offsets to output offsets */
			PX_COMPILE_TIME_ASSERT(sizeof(outputLayout.offsets) == sizeof(spriteRenderLayout.offsets));
			::memcpy(outputLayout.offsets, spriteRenderLayout.offsets, sizeof(outputLayout.offsets));
			outputLayout.stride = spriteRenderLayout.stride;

			updateParticles<SpriteInput, SpritePublicState, SpritePrivateState>(*obj, 
																				outputLayout,
																				static_cast<uint8_t*>(spriteRenderData->getBufferMappedPtr())
																				);
		}
	}
}

class ModifierParamsMapperCPUimpl : public ModifierParamsMapperCPU
{
	template <typename T>
	void _mapValue(size_t offset, T value)
	{
		if (mData != 0)
		{
			*(T*)(mData + mParamsOffset + offset) = value;
		}
	}

public:
	virtual void beginParams(void* , size_t size, size_t align, uint32_t)
	{
		mParamsOffset = (mTotalSize + (uint32_t)align - 1) & ~((uint32_t)align - 1);
		mTotalSize = mParamsOffset + (uint32_t)size;
	}
	virtual void endParams()
	{
	}

	virtual void mapValue(size_t offset, int32_t value)
	{
		_mapValue(offset, value);
	}
	virtual void mapValue(size_t offset, float value)
	{
		_mapValue(offset, value);
	}
	virtual void mapCurve(size_t offset, const nvidia::apex::Curve* curve)
	{
		_mapValue(offset, curve);
	}

	ModifierParamsMapperCPUimpl()
	{
		reset(0);
	}

	void reset(uint8_t* data)
	{
		mData = data;
		mTotalSize = 0;
		mParamsOffset = 0;
	}

	uint32_t getTotalSize() const
	{
		return mTotalSize;
	}
	uint32_t getParamsOffset() const
	{
		return mParamsOffset;
	}

private:
	uint8_t*	mData;

	uint32_t	mTotalSize;
	uint32_t	mParamsOffset;
};


uint32_t IofxAssetSceneInstCPU::introspectModifiers(ModifierParamsMapperCPUimpl& introspector, ModifierInfo* list, const ModifierStack& stack, ModifierStage stage, uint32_t usageClass)
{
	uint32_t usageStage = ModifierUsageFromStage(stage);
	uint32_t count = 0;
	for (ModifierStack::ConstIterator it = stack.begin(); it != stack.end(); ++it)
	{
		//uint32_t type = (*it)->getModifierType();
		uint32_t usage = (*it)->getModifierUsage();
		if ((usage & usageStage) == usageStage && (usage & usageClass) == usageClass)
		{
			const ModifierImpl* modifier = ModifierImpl::castFrom(*it);
			modifier->mapParamsCPU(introspector);
			if (list != 0)
			{
				if (usageClass == ModifierUsage_Sprite)
				{
					list[count].updateSpriteFunc = modifier->getUpdateSpriteFunc(stage);
				}
				else
				{
					list[count].updateMeshFunc = modifier->getUpdateMeshFunc(stage);
				}

				list[count].paramsOffset = introspector.getParamsOffset();
			}
			++count;
		}
	}
	return count;
}

IofxAssetSceneInstCPU::IofxAssetSceneInstCPU(IofxAssetImpl* asset, uint32_t semantics, IofxScene* scene)
	: IofxAssetSceneInst(asset, semantics)
{
	PX_UNUSED(scene);

	const ModifierStack& spawnModifiers = asset->getModifierStack(ModifierStage_Spawn);
	const ModifierStack& continuousModifiers = asset->getModifierStack(ModifierStage_Continuous);

	// set the particle buffer type here
	uint32_t usageClass = (asset->getMeshAssetCount() > 0) ? ModifierUsage_Mesh : ModifierUsage_Sprite;

	//collect params
	ModifierParamsMapperCPUimpl paramsMapper;

	uint32_t spawnModifierCount = introspectModifiers(paramsMapper, 0, spawnModifiers, ModifierStage_Spawn, usageClass);
	uint32_t continuousModifierCount = introspectModifiers(paramsMapper, 0, continuousModifiers, ModifierStage_Continuous, usageClass);

	mSpawnModifiersList.resize(spawnModifierCount);
	mContinuousModifiersList.resize(continuousModifierCount);

	mModifiersParamsBuffer.resize(paramsMapper.getTotalSize());

	paramsMapper.reset(mModifiersParamsBuffer.begin());

	introspectModifiers(paramsMapper, mSpawnModifiersList.begin(), spawnModifiers, ModifierStage_Spawn, usageClass);
	introspectModifiers(paramsMapper, mContinuousModifiersList.begin(), continuousModifiers, ModifierStage_Continuous, usageClass);
}

}
} // namespace nvidia
