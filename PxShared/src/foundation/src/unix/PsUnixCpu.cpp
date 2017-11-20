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

#include "foundation/PxSimpleTypes.h"
#include "PsCpu.h"

#if PX_X86 && !PX_EMSCRIPTEN
#define cpuid(op, reg)                                                                                                 \
	__asm__ __volatile__("pushl %%ebx      \n\t" /* save %ebx */                                                       \
	                     "cpuid            \n\t"                                                                       \
	                     "movl %%ebx, %1   \n\t" /* save what cpuid just put in %ebx */                                \
	                     "popl %%ebx       \n\t" /* restore the old %ebx */                                            \
	                     : "=a"(reg[0]), "=r"(reg[1]), "=c"(reg[2]), "=d"(reg[3])                                      \
	                     : "a"(op)                                                                                     \
	                     : "cc")
#else
#define cpuid(op, reg) reg[0] = reg[1] = reg[2] = reg[3] = 0;
#endif

namespace physx
{
namespace shdfnd
{

uint8_t Cpu::getCpuId()
{
	uint32_t cpuInfo[4];
	cpuid(1, cpuInfo);
	return static_cast<uint8_t>(cpuInfo[1] >> 24); // APIC Physical ID
}
}
}
