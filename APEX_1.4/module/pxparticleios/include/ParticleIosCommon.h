/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __PARTICLE_IOS_COMMON_H__
#define __PARTICLE_IOS_COMMON_H__

#include "PxBounds3.h"
#include "PxVec3.h"
#include "InplaceTypes.h"

const unsigned int INVALID_PARTICLE_INDEX		= (unsigned int)-1;

namespace nvidia
{
namespace pxparticleios
{

//struct Px3InjectorParams
#define INPLACE_TYPE_STRUCT_NAME Px3InjectorParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	mLODMaxDistance) \
	INPLACE_TYPE_FIELD(float,	mLODDistanceWeight) \
	INPLACE_TYPE_FIELD(float,	mLODSpeedWeight) \
	INPLACE_TYPE_FIELD(float,	mLODLifeWeight) \
	INPLACE_TYPE_FIELD(float,	mLODBias) \
	INPLACE_TYPE_FIELD(uint32_t,	mLocalIndex)
#include INPLACE_TYPE_BUILD()

typedef InplaceArray<Px3InjectorParams> InjectorParamsArray;


struct GridDensityParams
{
	bool Enabled;
	float GridSize;
	uint32_t GridMaxCellCount;
	uint32_t GridResolution;
	PxVec3 DensityOrigin;
	GridDensityParams(): Enabled(false) {}
};

struct GridDensityFrustumParams
{
	float nearDimX;
	float farDimX;
	float nearDimY;
	float farDimY;
	float dimZ; 
};

}
} // namespace nvidia

#endif
