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
#include "FieldBoundaryWrapper.h"
#include "FieldSamplerManager.h"


namespace nvidia
{
namespace fieldsampler
{

FieldBoundaryWrapper::FieldBoundaryWrapper(ResourceList& list, FieldSamplerManager* manager, FieldBoundaryIntl* fieldBoundary, const FieldBoundaryDescIntl& fieldBoundaryDesc)
	: mManager(manager)
	, mFieldBoundary(fieldBoundary)
	, mFieldBoundaryDesc(fieldBoundaryDesc)
	, mFieldShapesChanged(false)
{
	list.add(*this);

}

void FieldBoundaryWrapper::release()
{
	delete this;
}

void FieldBoundaryWrapper::update()
{
	mFieldShapesChanged = mFieldBoundary->updateFieldBoundary(mFieldShapes);
}

}
} // end namespace nvidia::apex

