/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "Apex.h"
#include "PxAssert.h"
#include "nvparameterized/NvParameterized.h"
#include "Curve.h"
#include "CurveImpl.h"

namespace nvidia
{
namespace apex
{

/**
	Linear interpolation. "in" and "out" stands here for X and Y coordinates of the control points
*/
inline float lerp(float inCurrent, float inMin, float inMax, float outMin, float outMax)
{
	if (inMin == inMax)
	{
		return outMin;
	}

	return ((inCurrent - inMin) / (inMax - inMin)) * (outMax - outMin) + outMin;
}

/**
	The CurveImpl is a class for storing control points on a curve and evaluating the results later.
*/

/**
	Retrieve the output Y for the specified input x, based on the properties of the stored curve described
	by mControlPoints.
*/
float CurveImpl::evaluate(float x) const
{
	Vec2R xPoints, yPoints;
	if (calculateControlPoints(x, xPoints, yPoints))
	{
		return lerp(x, xPoints[0], xPoints[1], yPoints[0], yPoints[1]);
	}
	else if (mControlPoints.size() == 1)
	{
		return mControlPoints[0].y;
	}
	else
	{
		// This is too noisy for editors...
		//PX_ASSERT(!"Unable to find control points that contained the specified curve parameter");
		return 0;
	}
}

/**
	Add a control point to the list of control points, returning the index of the new point.
*/
uint32_t CurveImpl::addControlPoint(const Vec2R& controlPoint)
{
	uint32_t index = calculateFollowingControlPoint(controlPoint.x);

	if (index == mControlPoints.size())
	{
		// add element to the end
		Vec2R& v2 = mControlPoints.insert();
		v2 = controlPoint;
	}
	else
	{
		// memmove all elements from index - end to index+1 - new_end
		uint32_t oldSize = mControlPoints.size();
		mControlPoints.insert();
		memmove(&mControlPoints[index + 1], &mControlPoints[index], sizeof(Vec2R) * (oldSize - index));
		mControlPoints[index] = controlPoint;
	}
	return index;
}

/**
	Add a control points to the list of control points.  Assuming the
	hPoints points to a list of vec2s
*/
void CurveImpl::addControlPoints(::NvParameterized::Interface* param, ::NvParameterized::Handle& hPoints)
{
	::NvParameterized::Handle ih(*param), hMember(*param);
	int arraySize = 0;
	PX_ASSERT(hPoints.getConstInterface() == param);
	hPoints.getArraySize(arraySize);
	for (int i = 0; i < arraySize; i++)
	{
		hPoints.getChildHandle(i, ih);
		Vec2R tmpVec2;
		ih.getChildHandle(0, hMember);
		hMember.getParamF32(tmpVec2.x);
		ih.getChildHandle(1, hMember);
		hMember.getParamF32(tmpVec2.y);

		addControlPoint(tmpVec2);
	}
}

/**
	Locates the control points that contain x, placing the resulting control points in the two
	out parameters. Returns true if the points were found, false otherwise. If the points were not
	found, the output variables are untouched
*/
bool CurveImpl::calculateControlPoints(float x, Vec2R& outXPoints, Vec2R& outYPoints) const
{
	uint32_t controlPointSize = mControlPoints.size();
	if (controlPointSize < 2)
	{
		return false;
	}

	uint32_t followControlPoint = calculateFollowingControlPoint(x);
	if (followControlPoint == 0)
	{
		outXPoints[0] = outXPoints[1] = mControlPoints[0].x;
		outYPoints[0] = outYPoints[1] = mControlPoints[0].y;
		return true;
	}
	else if (followControlPoint == controlPointSize)
	{
		outXPoints[0] = outXPoints[1] = mControlPoints[followControlPoint - 1].x;
		outYPoints[0] = outYPoints[1] = mControlPoints[followControlPoint - 1].y;
		return true;
	}

	outXPoints[0] = mControlPoints[followControlPoint - 1].x;
	outXPoints[1] = mControlPoints[followControlPoint].x;

	outYPoints[0] = mControlPoints[followControlPoint - 1].y;
	outYPoints[1] = mControlPoints[followControlPoint].y;

	return true;
}

/**
	Locates the first control point with x larger than xValue or the nimber of control points if such point doesn't exist
*/
uint32_t CurveImpl::calculateFollowingControlPoint(float xValue) const
{
	// TODO: This could be made O(log(N)), but I think there should
	// be so few entries that it's not worth the code complexity.
	uint32_t cpSize = mControlPoints.size();

	for (uint32_t u = 0; u < cpSize; ++u)
	{
		if (xValue <= mControlPoints[u].x)
		{
			return u;
		}
	}

	return cpSize;
}

///get the array of control points
const Vec2R* CurveImpl::getControlPoints(uint32_t& outCount) const
{
	outCount = mControlPoints.size();
	if (outCount)
	{
		return &mControlPoints.front();
	}
	else
	{
		// LRR: there's more to this, chase this down later
		return NULL;
	}
}

}
} // namespace nvidia::apex
