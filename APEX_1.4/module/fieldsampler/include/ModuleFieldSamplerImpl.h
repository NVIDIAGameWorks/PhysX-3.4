/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODULE_FIELD_SAMPLER_IMPL_H__
#define __MODULE_FIELD_SAMPLER_IMPL_H__

#include "Apex.h"
#include "ModuleFieldSampler.h"
#include "ApexSDKIntl.h"
#include "ModuleIntl.h"
#include "ModuleBase.h"

#include "ApexSDKHelpers.h"
#include "ApexRWLockable.h"
#include "ModuleFieldSamplerRegistration.h"

#include "ModuleFieldSamplerIntl.h"

namespace nvidia
{
namespace apex
{
class SceneIntl;
}

namespace fieldsampler
{

class FieldSamplerScene;

class ModuleFieldSamplerImpl : public ModuleFieldSampler, public ModuleFieldSamplerIntl, public ModuleBase, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ModuleFieldSamplerImpl(ApexSDKIntl* sdk);
	~ModuleFieldSamplerImpl();

	FieldSamplerManagerIntl*		getInternalFieldSamplerManager(const Scene& apexScene);

	bool setFieldSamplerWeightedCollisionFilterCallback(const Scene& apexScene,FieldSamplerWeightedCollisionFilterCallback *callback);

#if PX_PHYSICS_VERSION_MAJOR == 3
	void						enablePhysXMonitor(const Scene& apexScene, bool enable);
	void						setPhysXMonitorFilterData(const Scene& apexScene, physx::PxFilterData filterData);
#endif

	uint32_t				createForceSampleBatch(const Scene& apexScene, uint32_t maxCount, const physx::PxFilterData filterData);
	void						releaseForceSampleBatch(const Scene& apexScene, uint32_t batchId);
	void						submitForceSampleBatch(const Scene& apexScene, uint32_t batchId,
													   PxVec4* forces, const uint32_t forcesStride,
													   const PxVec3* positions, const uint32_t positionsStride,
													   const PxVec3* velocities, const uint32_t velocitiesStride,
													   const float* mass, const uint32_t massStride,
													   const uint32_t* indices, const uint32_t numIndices);


	// base class methods
	void						init(NvParameterized::Interface&);
	NvParameterized::Interface* getDefaultModuleDesc();
	void release()
	{
		ModuleBase::release();
	}
	void destroy();
	const char* getName() const
	{
		return ModuleBase::getName();
	}
	
	ModuleSceneIntl* 				createInternalModuleScene(SceneIntl&, RenderDebugInterface*);
	void						releaseModuleSceneIntl(ModuleSceneIntl&);
	AuthObjTypeID				getModuleID() const;
	RenderableIterator* 	createRenderableIterator(const Scene&);


protected:
	FieldSamplerScene* 	getFieldSamplerScene(const Scene& apexScene) const;

	ResourceList				mFieldSamplerScenes;

private:

	FieldSamplerModuleParameters* 			mModuleParams;
};

}
} // end namespace nvidia::apex

#endif // __MODULE_FIELD_SAMPLER_IMPL_H__
