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

#ifndef PSFOUNDATION_PSTIME_H
#define PSFOUNDATION_PSTIME_H

#include "Ps.h"

#if PX_LINUX || PX_ANDROID
#include <time.h>
#endif

namespace physx
{
namespace shdfnd
{

struct CounterFrequencyToTensOfNanos
{
	uint64_t mNumerator;
	uint64_t mDenominator;
	CounterFrequencyToTensOfNanos(uint64_t inNum, uint64_t inDenom) : mNumerator(inNum), mDenominator(inDenom)
	{
	}

	// quite slow.
	uint64_t toTensOfNanos(uint64_t inCounter) const
	{
		return (inCounter * mNumerator) / mDenominator;
	}
};

class PX_FOUNDATION_API Time
{
  public:
	typedef double Second;
	static const uint64_t sNumTensOfNanoSecondsInASecond = 100000000;
	// This is supposedly guaranteed to not change after system boot
	// regardless of processors, speedstep, etc.
	static const CounterFrequencyToTensOfNanos& getBootCounterFrequency();

	static CounterFrequencyToTensOfNanos getCounterFrequency();

	static uint64_t getCurrentCounterValue();

	// SLOW!!
	// Thar be a 64 bit divide in thar!
	static uint64_t getCurrentTimeInTensOfNanoSeconds()
	{
		uint64_t ticks = getCurrentCounterValue();
		return getBootCounterFrequency().toTensOfNanos(ticks);
	}

	Time();
	Second getElapsedSeconds();
	Second peekElapsedSeconds();
	Second getLastTime() const;

  private:
#if PX_LINUX || PX_ANDROID || PX_APPLE_FAMILY || PX_PS4
	Second mLastTime;
#else
	int64_t mTickCount;
#endif
};
} // namespace shdfnd
} // namespace physx

#endif // #ifndef PSFOUNDATION_PSTIME_H
