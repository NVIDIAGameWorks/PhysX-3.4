/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_SAMPLER_MANAGER_H__
#define __FIELD_SAMPLER_MANAGER_H__

#include "FieldSamplerManagerIntl.h"
#include "FieldSamplerScene.h"
#include "ApexGroupsFiltering.h"

#if APEX_CUDA_SUPPORT
#define CUDA_OBJ(name) SCENE_CUDA_OBJ(mManager->getScene(), name)
#endif

namespace nvidia
{
namespace fieldsampler
{

PX_INLINE bool operator != (const PxFilterData& d1, const PxFilterData& d2)
{
	return d1.word0 != d2.word0 || d1.word1 != d2.word1 || d1.word2 != d2.word2 || d1.word3 != d2.word3;
}

class FieldSamplerScene;

class FieldSamplerQuery;
class FieldSamplerSceneWrapper;
class FieldSamplerWrapper;
class FieldBoundaryWrapper;

class FieldSamplerManager : public FieldSamplerManagerIntl,  public UserAllocated
{
public:
	FieldSamplerManager(FieldSamplerScene* scene);
	virtual ~FieldSamplerManager() {}

	void		submitTasks();
	void		setTaskDependencies();
	void		fetchResults();

	/* FieldSamplerManager */
	FieldSamplerQueryIntl* createFieldSamplerQuery(const FieldSamplerQueryDescIntl&);

	void registerFieldSampler(FieldSamplerIntl* , const FieldSamplerDescIntl& , FieldSamplerSceneIntl*);
	void unregisterFieldSampler(FieldSamplerIntl*);

	void registerFieldBoundary(FieldBoundaryIntl* , const FieldBoundaryDescIntl&);
	void unregisterFieldBoundary(FieldBoundaryIntl*);

	void registerUnhandledParticleSystem(physx::PxActor*);
	void unregisterUnhandledParticleSystem(physx::PxActor*);
	bool isUnhandledParticleSystem(physx::PxActor*);

	FieldSamplerScene*	getScene() const
	{
		return mScene;
	}
	SceneIntl&		getApexScene() const
	{
		return *mScene->mApexScene;
	}

	bool getFieldSamplerGroupsFiltering(const PxFilterData &o1,const PxFilterData &o2,float &weight) const;
	void setFieldSamplerWeightedCollisionFilterCallback(FieldSamplerWeightedCollisionFilterCallback *callback)
	{
		mWeightedCallback = callback;
	}

	virtual bool doFieldSamplerFiltering(const PxFilterData &o1, const PxFilterData &o2, float &weight) const
	{
		return getFieldSamplerGroupsFiltering(o1, o2, weight);
	}

protected:
	virtual FieldSamplerQuery* allocateFieldSamplerQuery(const FieldSamplerQueryDescIntl&) = 0;
	virtual FieldSamplerSceneWrapper* allocateFieldSamplerSceneWrapper(FieldSamplerSceneIntl*) = 0;
	virtual FieldSamplerWrapper* allocateFieldSamplerWrapper(FieldSamplerIntl* , const FieldSamplerDescIntl& , FieldSamplerSceneWrapper*) = 0;

	static PX_INLINE void addFieldSamplerToQuery(FieldSamplerWrapper* fieldSamplerWrapper, FieldSamplerQuery* query);
	void addAllFieldSamplersToQuery(FieldSamplerQuery*) const;

	FieldSamplerScene*	mScene;

	ResourceList		mFieldSamplerQueryList;
	ResourceList		mFieldSamplerSceneWrapperList;
	ResourceList		mFieldSamplerWrapperList;
	ResourceList		mFieldBoundaryWrapperList;

	nvidia::Array<physx::PxActor*>		mUnhandledParticleSystems;
	FieldSamplerWeightedCollisionFilterCallback *mWeightedCallback;

	bool								mFieldSamplerGroupsFilteringChanged;

	friend class FieldSamplerSceneWrapperGPU;
};

class FieldSamplerManagerCPU : public FieldSamplerManager
{
public:
	FieldSamplerManagerCPU(FieldSamplerScene* scene) : FieldSamplerManager(scene) {}

protected:
	FieldSamplerQuery* allocateFieldSamplerQuery(const FieldSamplerQueryDescIntl&);
	FieldSamplerSceneWrapper* allocateFieldSamplerSceneWrapper(FieldSamplerSceneIntl*);
	FieldSamplerWrapper* allocateFieldSamplerWrapper(FieldSamplerIntl* , const FieldSamplerDescIntl& , FieldSamplerSceneWrapper*);
};

#if APEX_CUDA_SUPPORT
class FieldSamplerManagerGPU : public FieldSamplerManager
{
public:
	FieldSamplerManagerGPU(FieldSamplerScene* scene) : FieldSamplerManager(scene) {}

protected:
	FieldSamplerQuery* allocateFieldSamplerQuery(const FieldSamplerQueryDescIntl&);
	FieldSamplerSceneWrapper* allocateFieldSamplerSceneWrapper(FieldSamplerSceneIntl*);
	FieldSamplerWrapper* allocateFieldSamplerWrapper(FieldSamplerIntl* , const FieldSamplerDescIntl& , FieldSamplerSceneWrapper*);
};
#endif // APEX_CUDA_SUPPORT


} // end namespace nvidia::apex
}
#endif // __FIELD_SAMPLER_MANAGER_H__
