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
#include "PsFPU.h"
#include "float.h"
#include "PsIntrinsics.h"

#if PX_X64
#define _MCW_ALL _MCW_DN | _MCW_EM | _MCW_RC
#else
#define _MCW_ALL _MCW_DN | _MCW_EM | _MCW_IC | _MCW_RC | _MCW_PC
#endif

physx::shdfnd::FPUGuard::FPUGuard()
{
// default plus FTZ and DAZ
#if PX_X64
	// query current control word state
	_controlfp_s(mControlWords, 0, 0);

	// set both x87 and sse units to default + DAZ
	unsigned int cw;
	_controlfp_s(&cw, _CW_DEFAULT | _DN_FLUSH, _MCW_ALL);
#else
	// query current control word state
	__control87_2(0, 0, mControlWords, mControlWords + 1);

	// set both x87 and sse units to default + DAZ
	unsigned int x87, sse;
	__control87_2(_CW_DEFAULT | _DN_FLUSH, _MCW_ALL, &x87, &sse);
#endif
}

physx::shdfnd::FPUGuard::~FPUGuard()
{
	_clearfp();

#if PX_X64
	// reset FP state
	unsigned int cw;
	_controlfp_s(&cw, *mControlWords, _MCW_ALL);
#else

	// reset FP state
	unsigned int x87, sse;
	__control87_2(mControlWords[0], _MCW_ALL, &x87, 0);
	__control87_2(mControlWords[1], _MCW_ALL, 0, &sse);
#endif
}

void physx::shdfnd::enableFPExceptions()
{
	// clear any pending exceptions
	_clearfp();

	// enable all fp exceptions except inexact and underflow (common, benign)
	_controlfp_s(NULL, uint32_t(~_MCW_EM) | _EM_INEXACT | _EM_UNDERFLOW, _MCW_EM);
}

void physx::shdfnd::disableFPExceptions()
{
	_controlfp_s(NULL, _MCW_EM, _MCW_EM);
}
