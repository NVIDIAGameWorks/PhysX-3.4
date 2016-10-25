/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_FS_ACTOR_H__
#define __BASIC_FS_ACTOR_H__

#include "Apex.h"

#include "ApexActor.h"
#include "FieldSamplerIntl.h"
#include "BasicFSAssetImpl.h"

#include "PxTask.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#endif


namespace nvidia
{
namespace apex
{
class RenderMeshActor;
}
namespace basicfs
{

class BasicFSScene;

class BasicFSActor : public ApexActor, public ApexResourceInterface, public ApexResource, public FieldSamplerIntl
{
public:
	BasicFSActor(BasicFSScene&);
	virtual ~BasicFSActor();

	/* ApexResourceInterface, ApexResource */
	uint32_t					getListIndex() const
	{
		return m_listIndex;
	}
	void					setListIndex(class ResourceList& list, uint32_t index)
	{
		m_list = &list;
		m_listIndex = index;
	}

	virtual void			visualize()
	{
	}

	virtual void			simulate(float dt)
	{
		PX_UNUSED(dt);
	}

	void					setPhysXScene(PxScene*);
	PxScene*				getPhysXScene() const;

	/* FieldSamplerIntl */
	virtual bool			updateFieldSampler(FieldShapeDescIntl& shapeDesc, bool& isEnabled) = 0;

protected:
	BasicFSScene*			mScene;

	PxTransform		mPose;
	float			mScale;

	bool					mFieldSamplerChanged;
	bool					mFieldSamplerEnabled;

	float			mFieldWeight;

	friend class BasicFSScene;
};

}
} // end namespace nvidia

#endif
