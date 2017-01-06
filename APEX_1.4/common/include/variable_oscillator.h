/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef VARIABLE_OSCILLATOR_H
#define VARIABLE_OSCILLATOR_H

#include "ApexUsingNamespace.h"
#include "PsUserAllocated.h"

namespace nvidia
{
namespace apex
{

class variableOscillator : public UserAllocated
{
public:
	variableOscillator(float min, float max, float initial, float period);
	~variableOscillator();
	float updateVariableOscillator(float deltaTime);

private:
	float computeEndVal(float current, float max_or_min);

private:
	float	mMin;
	float	mMax;
	float	mPeriod;

	float	mCumTime;
	float	mStartVal;
	float	mEndVal;
	float	mLastVal;
	bool	mGoingUp;
};

}
} // namespace nvidia::apex

#endif
