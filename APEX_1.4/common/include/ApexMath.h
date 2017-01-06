/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_MATH_H
#define APEX_MATH_H

#include "PxMat44.h"
#include "PsMathUtils.h"

#include "PsVecMath.h"
namespace nvidia
{

#define APEX_ALIGN_UP(offset, alignment) (((offset) + (alignment)-1) & ~((alignment)-1))

/**
 * computes weight * _origin + (1.0f - weight) * _target
 */
PX_INLINE PxMat44 interpolateMatrix(float weight, const PxMat44& _origin, const PxMat44& _target)
{
	// target: normalize, save scale, transform to quat
	PxMat33 target(_target.column0.getXYZ(),
					_target.column1.getXYZ(),
					_target.column2.getXYZ());
	PxVec3 axis0 = target.column0;
	PxVec3 axis1 = target.column1;
	PxVec3 axis2 = target.column2;
	const PxVec4 targetScale(axis0.normalize(), axis1.normalize(), axis2.normalize(), 1.0f);
	target.column0 = axis0;
	target.column1 = axis1;
	target.column2 = axis2;
	const PxQuat targetQ = PxQuat(target);

	// origin: normalize, save scale, transform to quat
	PxMat33 origin(_origin.column0.getXYZ(),
					_origin.column1.getXYZ(),
					_origin.column2.getXYZ());
	const PxVec4 originScale(axis0.normalize(), axis1.normalize(), axis2.normalize(), 1.0f);
	origin.column0 = axis0;
	origin.column1 = axis1;
	origin.column2 = axis2;
	const PxQuat originQ = PxQuat(origin);

	// interpolate
	PxQuat relativeQ = physx::shdfnd::slerp(1.0f - weight, originQ, targetQ);
	PxMat44 relative(relativeQ);
	relative.setPosition(weight * _origin.getPosition() + (1.0f - weight) * _target.getPosition());

	PxMat44 _relative = relative;
	const PxVec4 scale = weight * originScale + (1.0f - weight) * targetScale;
	_relative.scale(scale);

	return _relative;
}

bool operator != (const PxMat44& a, const PxMat44& b);

}


#endif // APEX_MATH_H
