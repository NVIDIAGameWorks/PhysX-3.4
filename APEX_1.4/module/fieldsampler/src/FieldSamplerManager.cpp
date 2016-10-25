/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"
#include "Apex.h"

#include "FieldSamplerManager.h"

#include "FieldSamplerQuery.h"
#include "FieldSamplerSceneWrapper.h"
#include "FieldSamplerWrapper.h"
#include "FieldBoundaryWrapper.h"

#include "SceneIntl.h"

namespace nvidia
{
namespace fieldsampler
{

FieldSamplerManager::FieldSamplerManager(FieldSamplerScene* scene)
	: mScene(scene)
	, mFieldSamplerGroupsFilteringChanged(true)
{
	mWeightedCallback = NULL;
}


PX_INLINE void FieldSamplerManager::addFieldSamplerToQuery(FieldSamplerWrapper* fieldSamplerWrapper, FieldSamplerQuery* query)
{
	if (query->addFieldSampler(fieldSamplerWrapper))
	{
		fieldSamplerWrapper->mQueryRefCount += 1;
	}
}

void FieldSamplerManager::addAllFieldSamplersToQuery(FieldSamplerQuery* query) const
{
	for (uint32_t i = 0; i < mFieldSamplerWrapperList.getSize(); ++i)
	{
		addFieldSamplerToQuery(static_cast<FieldSamplerWrapper*>(mFieldSamplerWrapperList.getResource(i)), query);
	}
}

void FieldSamplerManager::submitTasks()
{
	if (mFieldSamplerGroupsFilteringChanged)
	{
		mFieldSamplerGroupsFilteringChanged = false;

		//clear queryRefCounts
		for (uint32_t i = 0; i < mFieldSamplerWrapperList.getSize(); ++i)
		{
			FieldSamplerWrapper* fieldSamplerWrapper = static_cast<FieldSamplerWrapper*>(mFieldSamplerWrapperList.getResource(i));
			fieldSamplerWrapper->mQueryRefCount = 0;
		}

		//rebuild all connection based on changed collision checking
		for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
		{
			FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
			query->clearAllFieldSamplers();
			addAllFieldSamplersToQuery(query);
		}
	}

	for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
	{
		FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
		query->submitTasks();
	}
}

void FieldSamplerManager::setTaskDependencies()
{
	for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
	{
		FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
		query->setTaskDependencies();
	}

	//update
	for (uint32_t i = 0; i < mFieldBoundaryWrapperList.getSize(); ++i)
	{
		FieldBoundaryWrapper* wrapper = DYNAMIC_CAST(FieldBoundaryWrapper*)(mFieldBoundaryWrapperList.getResource(i));
		wrapper->update();
	}
	for (uint32_t i = 0; i < mFieldSamplerWrapperList.getSize(); ++i)
	{
		FieldSamplerWrapper* wrapper = DYNAMIC_CAST(FieldSamplerWrapper*)(mFieldSamplerWrapperList.getResource(i));
		wrapper->update();
	}
	for (uint32_t i = 0; i < mFieldSamplerSceneWrapperList.getSize(); ++i)
	{
		FieldSamplerSceneWrapper* wrapper = DYNAMIC_CAST(FieldSamplerSceneWrapper*)(mFieldSamplerSceneWrapperList.getResource(i));
		wrapper->update();
	}
	for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
	{
		FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
		query->update();
	}

	//postUpdate
	for (uint32_t i = 0; i < mFieldSamplerSceneWrapperList.getSize(); ++i)
	{
		FieldSamplerSceneWrapper* wrapper = DYNAMIC_CAST(FieldSamplerSceneWrapper*)(mFieldSamplerSceneWrapperList.getResource(i));
		wrapper->postUpdate();
	}
}

void FieldSamplerManager::fetchResults()
{
	for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
	{
		FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
		query->fetchResults();
	}
}


FieldSamplerQueryIntl* FieldSamplerManager::createFieldSamplerQuery(const FieldSamplerQueryDescIntl& desc)
{
	FieldSamplerQuery* query = allocateFieldSamplerQuery(desc);
	if (query)
	{
		addAllFieldSamplersToQuery(query);
	}
	return query;
}

void FieldSamplerManager::registerFieldSampler(FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneIntl* fieldSamplerScene)
{
	FieldSamplerSceneWrapper* fieldSamplerSceneWrapper = NULL;
	//find FieldSamplerSceneWrapper
	for (uint32_t i = 0; i < mFieldSamplerSceneWrapperList.getSize(); ++i)
	{
		FieldSamplerSceneWrapper* wrapper = DYNAMIC_CAST(FieldSamplerSceneWrapper*)(mFieldSamplerSceneWrapperList.getResource(i));
		if (wrapper->getInternalFieldSamplerScene() == fieldSamplerScene)
		{
			fieldSamplerSceneWrapper = wrapper;
			break;
		}
	}
	if (fieldSamplerSceneWrapper == NULL)
	{
		fieldSamplerSceneWrapper = allocateFieldSamplerSceneWrapper(fieldSamplerScene);
	}
	PX_ASSERT(fieldSamplerSceneWrapper != NULL);

	FieldSamplerWrapper* fieldSamplerWrapper = allocateFieldSamplerWrapper(fieldSampler, fieldSamplerDesc, fieldSamplerSceneWrapper);
	PX_ASSERT(fieldSamplerWrapper != NULL);

	// add all mFieldBoundaryWrapperList
	for (uint32_t i = 0; i < mFieldBoundaryWrapperList.getSize(); ++i)
	{
		FieldBoundaryWrapper* wrapper = DYNAMIC_CAST(FieldBoundaryWrapper*)(mFieldBoundaryWrapperList.getResource(i));
		fieldSamplerWrapper->addFieldBoundary(wrapper);
	}

	for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
	{
		FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
		addFieldSamplerToQuery(fieldSamplerWrapper, query);
	}
}

void FieldSamplerManager::unregisterFieldSampler(FieldSamplerIntl* fieldSampler)
{
	FieldSamplerWrapper* fieldSamplerWrapper = NULL;
	//find FieldSamplerWrapper
	for (uint32_t i = 0; i < mFieldSamplerWrapperList.getSize(); ++i)
	{
		FieldSamplerWrapper* wrapper = static_cast<FieldSamplerWrapper*>(mFieldSamplerWrapperList.getResource(i));
		if (wrapper->getInternalFieldSampler() == fieldSampler)
		{
			fieldSamplerWrapper = wrapper;
			break;
		}
	}
	if (fieldSamplerWrapper != NULL)
	{
		for (uint32_t i = 0; i < mFieldSamplerQueryList.getSize(); ++i)
		{
			FieldSamplerQuery* query = DYNAMIC_CAST(FieldSamplerQuery*)(mFieldSamplerQueryList.getResource(i));
			query->removeFieldSampler(fieldSamplerWrapper);
		}
		fieldSamplerWrapper->release();
	}
}

void FieldSamplerManager::registerFieldBoundary(FieldBoundaryIntl* fieldBoundary, const FieldBoundaryDescIntl& fieldBoundaryDesc)
{
	FieldBoundaryWrapper* fieldBoundaryWrapper = PX_NEW(FieldBoundaryWrapper)(mFieldBoundaryWrapperList, this, fieldBoundary, fieldBoundaryDesc);
	if (fieldBoundaryWrapper)
	{
		for (uint32_t i = 0; i < mFieldSamplerWrapperList.getSize(); ++i)
		{
			static_cast<FieldSamplerWrapper*>(mFieldSamplerWrapperList.getResource(i))->addFieldBoundary(fieldBoundaryWrapper);
		}
	}
}
void FieldSamplerManager::unregisterFieldBoundary(FieldBoundaryIntl* fieldBoundary)
{
	FieldBoundaryWrapper* fieldBoundaryWrapper = 0;
	for (uint32_t i = 0; i < mFieldBoundaryWrapperList.getSize(); ++i)
	{
		FieldBoundaryWrapper* wrapper = static_cast<FieldBoundaryWrapper*>(mFieldBoundaryWrapperList.getResource(i));
		if (wrapper->getInternalFieldBoundary() == fieldBoundary)
		{
			fieldBoundaryWrapper = wrapper;
			break;
		}
	}
	if (fieldBoundaryWrapper != 0)
	{
		for (uint32_t i = 0; i < mFieldSamplerWrapperList.getSize(); ++i)
		{
			static_cast<FieldSamplerWrapper*>(mFieldSamplerWrapperList.getResource(i))->removeFieldBoundary(fieldBoundaryWrapper);
		}
		fieldBoundaryWrapper->release();
	}
}

void FieldSamplerManager::registerUnhandledParticleSystem(physx::PxActor* actor)
{
	if (!isUnhandledParticleSystem(actor))
	{
		mUnhandledParticleSystems.pushBack(actor);
	}
}

void FieldSamplerManager::unregisterUnhandledParticleSystem(physx::PxActor* actor)
{
	mUnhandledParticleSystems.findAndReplaceWithLast(actor);
}

bool FieldSamplerManager::isUnhandledParticleSystem(physx::PxActor* actor)
{
	for (uint32_t i = 0; i < mUnhandledParticleSystems.size(); i++)
	if (mUnhandledParticleSystems[i] == actor)
	{
		return true;
	}
	return false;
}

bool FieldSamplerManager::getFieldSamplerGroupsFiltering(const PxFilterData &o1,const PxFilterData &o2,float &weight) const
{
	bool ret = true;

	weight = 1.0f;
	if ( mScene )
	{
		if ( mWeightedCallback )
		{
			ret = mWeightedCallback->fieldSamplerWeightedCollisionFilter(o1,o2,weight);
		}
		else
		{
			physx::PxScene *scene = mScene->getModulePhysXScene();
			scene->lockRead(__FILE__,__LINE__);
			physx::PxSimulationFilterShader shader = scene->getFilterShader();
			scene->unlockRead();
			if ( shader )
			{
				physx::PxFilterObjectAttributes atr0 = 0;
				physx::PxFilterObjectAttributes atr1 = 0;
				physx::PxPairFlags pairFlags;
				physx::PxFilterFlags result = (*shader)(atr0,o1,atr1,o2,pairFlags,NULL,0);
				if ( result & (physx::PxFilterFlag::eKILL | physx::PxFilterFlag::eSUPPRESS) )
				{
					ret = false;
				}
			}
		}
	}
	return ret;
}




/******************************** CPU Version ********************************/

FieldSamplerQuery* FieldSamplerManagerCPU::allocateFieldSamplerQuery(const FieldSamplerQueryDescIntl& desc)
{
	return PX_NEW(FieldSamplerQueryCPU)(desc, mFieldSamplerQueryList, this);
}
FieldSamplerSceneWrapper* FieldSamplerManagerCPU::allocateFieldSamplerSceneWrapper(FieldSamplerSceneIntl* fieldSamplerScene)
{
	return PX_NEW(FieldSamplerSceneWrapperCPU)(mFieldSamplerSceneWrapperList, this, fieldSamplerScene);
}
FieldSamplerWrapper* FieldSamplerManagerCPU::allocateFieldSamplerWrapper(FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper)
{
	return PX_NEW(FieldSamplerWrapperCPU)(mFieldSamplerWrapperList, this, fieldSampler, fieldSamplerDesc, fieldSamplerSceneWrapper);
}

/******************************** GPU Version ********************************/

#if APEX_CUDA_SUPPORT

FieldSamplerQuery* FieldSamplerManagerGPU::allocateFieldSamplerQuery(const FieldSamplerQueryDescIntl& desc)
{
	return PX_NEW(FieldSamplerQueryGPU)(desc, mFieldSamplerQueryList, this);
}
FieldSamplerSceneWrapper* FieldSamplerManagerGPU::allocateFieldSamplerSceneWrapper(FieldSamplerSceneIntl* fieldSamplerScene)
{
	return PX_NEW(FieldSamplerSceneWrapperGPU)(mFieldSamplerSceneWrapperList, this, fieldSamplerScene);
}
FieldSamplerWrapper* FieldSamplerManagerGPU::allocateFieldSamplerWrapper(FieldSamplerIntl* fieldSampler, const FieldSamplerDescIntl& fieldSamplerDesc, FieldSamplerSceneWrapper* fieldSamplerSceneWrapper)
{
	return PX_NEW(FieldSamplerWrapperGPU)(mFieldSamplerWrapperList, this, fieldSampler, fieldSamplerDesc, fieldSamplerSceneWrapper);
}

#endif // APEX_CUDA_SUPPORT

}
} // end namespace nvidia::apex


