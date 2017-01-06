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

#include "Ps.h"
#include "PsTime.h"

#include <time.h>
#include <sys/time.h>

#if PX_APPLE_FAMILY
#include <mach/mach_time.h>
#endif

// Use real-time high-precision timer.
#if !PX_APPLE_FAMILY
#define CLOCKID CLOCK_REALTIME
#endif

namespace physx
{
namespace shdfnd
{

static const CounterFrequencyToTensOfNanos gCounterFreq = Time::getCounterFrequency();

const CounterFrequencyToTensOfNanos& Time::getBootCounterFrequency()
{
	return gCounterFreq;
}

static Time::Second getTimeSeconds()
{
	static struct timeval _tv;
	gettimeofday(&_tv, NULL);
	return double(_tv.tv_sec) + double(_tv.tv_usec) * 0.000001;
}

Time::Time()
{
	mLastTime = getTimeSeconds();
}

Time::Second Time::getElapsedSeconds()
{
	Time::Second curTime = getTimeSeconds();
	Time::Second diff = curTime - mLastTime;
	mLastTime = curTime;
	return diff;
}

Time::Second Time::peekElapsedSeconds()
{
	Time::Second curTime = getTimeSeconds();
	Time::Second diff = curTime - mLastTime;
	return diff;
}

Time::Second Time::getLastTime() const
{
	return mLastTime;
}

#if PX_APPLE_FAMILY
CounterFrequencyToTensOfNanos Time::getCounterFrequency()
{
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	// mach_absolute_time * (info.numer/info.denom) is in units of nano seconds
	return CounterFrequencyToTensOfNanos(info.numer, info.denom * 10);
}

uint64_t Time::getCurrentCounterValue()
{
	return mach_absolute_time();
}

#else

CounterFrequencyToTensOfNanos Time::getCounterFrequency()
{
	return CounterFrequencyToTensOfNanos(1, 10);
}

uint64_t Time::getCurrentCounterValue()
{
	struct timespec mCurrTimeInt;
	clock_gettime(CLOCKID, &mCurrTimeInt);
	// Convert to nanos as this doesn't cause a large divide here
	return (static_cast<uint64_t>(mCurrTimeInt.tv_sec) * 1000000000) + (static_cast<uint64_t>(mCurrTimeInt.tv_nsec));
}
#endif

} // namespace shdfnd
} // namespace physx
