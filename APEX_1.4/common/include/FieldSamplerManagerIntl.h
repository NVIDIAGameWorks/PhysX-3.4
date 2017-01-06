/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FIELD_SAMPLER_MANAGER_INTL_H
#define FIELD_SAMPLER_MANAGER_INTL_H

#include "Apex.h"
#include "ApexSDKHelpers.h"

namespace nvidia
{
namespace apex
{

struct FieldSamplerQueryDescIntl;
class FieldSamplerQueryIntl;

class FieldSamplerSceneIntl;

struct FieldSamplerDescIntl;
class FieldSamplerIntl;

struct FieldBoundaryDescIntl;
class FieldBoundaryIntl;

class FieldSamplerManagerIntl
{
public:
	virtual FieldSamplerQueryIntl*	createFieldSamplerQuery(const FieldSamplerQueryDescIntl&) = 0;

	virtual void					registerFieldSampler(FieldSamplerIntl* , const FieldSamplerDescIntl& , FieldSamplerSceneIntl*) = 0;
	virtual void					unregisterFieldSampler(FieldSamplerIntl*) = 0;

	virtual void					registerFieldBoundary(FieldBoundaryIntl* , const FieldBoundaryDescIntl&) = 0;
	virtual void					unregisterFieldBoundary(FieldBoundaryIntl*) = 0;

#if PX_PHYSICS_VERSION_MAJOR == 3
	virtual void					registerUnhandledParticleSystem(PxActor*) = 0;
	virtual void					unregisterUnhandledParticleSystem(PxActor*) = 0;
	virtual bool					isUnhandledParticleSystem(PxActor*) = 0;

	virtual bool					doFieldSamplerFiltering(const PxFilterData &o1, const PxFilterData &o2, float &weight) const = 0;
#endif
};

}
} // end namespace nvidia::apex

#endif // #ifndef FIELD_SAMPLER_MANAGER_INTL_H
