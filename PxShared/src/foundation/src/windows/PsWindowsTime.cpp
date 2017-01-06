// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "PsTime.h"
#include "windows/PsWindowsInclude.h"

namespace
{
int64_t getTimeTicks()
{
	LARGE_INTEGER a;
	QueryPerformanceCounter(&a);
	return a.QuadPart;
}

double getTickDuration()
{
	LARGE_INTEGER a;
	QueryPerformanceFrequency(&a);
	return 1.0f / double(a.QuadPart);
}

double sTickDuration = getTickDuration();
} // namespace

namespace physx
{
namespace shdfnd
{

static const CounterFrequencyToTensOfNanos gCounterFreq = Time::getCounterFrequency();

const CounterFrequencyToTensOfNanos& Time::getBootCounterFrequency()
{
	return gCounterFreq;
}

CounterFrequencyToTensOfNanos Time::getCounterFrequency()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return CounterFrequencyToTensOfNanos(Time::sNumTensOfNanoSecondsInASecond, (uint64_t)freq.QuadPart);
}

uint64_t Time::getCurrentCounterValue()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return (uint64_t)ticks.QuadPart;
}

Time::Time() : mTickCount(0)
{
	getElapsedSeconds();
}

Time::Second Time::getElapsedSeconds()
{
	int64_t lastTickCount = mTickCount;
	mTickCount = getTimeTicks();
	return (mTickCount - lastTickCount) * sTickDuration;
}

Time::Second Time::peekElapsedSeconds()
{
	return (getTimeTicks() - mTickCount) * sTickDuration;
}

Time::Second Time::getLastTime() const
{
	return mTickCount * sTickDuration;
}

} // namespace shdfnd
} // namespace physx
