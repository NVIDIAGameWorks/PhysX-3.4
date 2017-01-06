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
#include "variable_oscillator.h"
#include "PxMath.h"

namespace nvidia
{
namespace apex
{

variableOscillator::variableOscillator(float min, float max, float initial, float period) :
	mMin(min),
	mMax(max),
	mPeriod(period),
	mStartVal(initial),
	mLastVal(initial)
{
	mCumTime = 0.0f;
	mGoingUp = true;
	mEndVal = computeEndVal(mMin, mMax);
}


variableOscillator::~variableOscillator()
{
}

float variableOscillator::computeEndVal(float current, float maxOrMin)
{
	float target;
	float maxDelta;
	float quarterVal;

	// compute the max range of the oscillator
	maxDelta = maxOrMin - current;
	// find the 'lower bound' of the oscillator peak
	quarterVal = current + (maxDelta / 4.0f);
	// get a rand between 0 and 1
	target = (float) ::rand() / (float) RAND_MAX;
	// scale the rand to the range we want
	target = target * PxAbs(quarterVal - maxOrMin);
	// add the offset to the scaled random number.
	if (current < maxOrMin)
	{
		target = target + quarterVal;
	}
	else
	{
		target = quarterVal - target;
	}
	return(target);
}

float variableOscillator::updateVariableOscillator(float deltaTime)
{
	float returnVal;
	float halfRange;

	mCumTime += deltaTime;

	// has the function crossed a max or a min?
	if ((mGoingUp  && (mCumTime > (mPeriod / 2.0f))) ||
	        (!mGoingUp && (mCumTime > mPeriod)))
	{
		mStartVal = mLastVal;
		if (mGoingUp)
		{
			mEndVal = computeEndVal(mStartVal, mMin);
		}
		else
		{
			mEndVal = computeEndVal(mStartVal, mMax);
			mCumTime = mCumTime - mPeriod;
		}
		mGoingUp = !mGoingUp;
	}
	halfRange = 0.5f * PxAbs(mEndVal - mStartVal);
	returnVal = -halfRange * PxCos(mCumTime * PxTwoPi / mPeriod) + halfRange + PxMin(mStartVal, mEndVal);
	mLastVal = returnVal;

	return(returnVal);
}

}
} // namespace nvidia::apex
