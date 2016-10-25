/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __FIELD_BOUNDARY_WRAPPER_H__
#define __FIELD_BOUNDARY_WRAPPER_H__

#include "Apex.h"
#include "ApexSDKHelpers.h"
#include "ApexActor.h"
#include "FieldBoundaryIntl.h"

#if APEX_CUDA_SUPPORT
#include "ApexCudaWrapper.h"
#endif

#include "FieldSamplerCommon.h"


namespace nvidia
{
namespace fieldsampler
{

class FieldSamplerManager;


class FieldBoundaryWrapper : public ApexResourceInterface, public ApexResource
{
public:
	// ApexResourceInterface methods
	void			release();
	void			setListIndex(ResourceList& list, uint32_t index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	uint32_t	getListIndex() const
	{
		return m_listIndex;
	}

	FieldBoundaryWrapper(ResourceList& list, FieldSamplerManager* manager, FieldBoundaryIntl* fieldBoundary, const FieldBoundaryDescIntl& fieldBoundaryDesc);

	FieldBoundaryIntl* getInternalFieldBoundary() const
	{
		return mFieldBoundary;
	}
	PX_INLINE const FieldBoundaryDescIntl& getInternalFieldBoundaryDesc() const
	{
		return mFieldBoundaryDesc;
	}

	void update();

	const nvidia::Array<FieldShapeDescIntl>&	getFieldShapes() const
	{
		return mFieldShapes;
	}
	bool									getFieldShapesChanged() const
	{
		return mFieldShapesChanged;
	}

protected:
	FieldSamplerManager*			mManager;

	FieldBoundaryIntl*				mFieldBoundary;
	FieldBoundaryDescIntl				mFieldBoundaryDesc;

	nvidia::Array<FieldShapeDescIntl>	mFieldShapes;
	bool							mFieldShapesChanged;
};

}
} // end namespace nvidia::apex

#endif
