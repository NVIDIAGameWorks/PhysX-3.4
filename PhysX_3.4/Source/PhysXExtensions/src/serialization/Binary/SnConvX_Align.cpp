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

#include "SnConvX.h"
#include "SnConvX_Align.h"
#include <assert.h>
using namespace physx;

void Sn::ConvX::alignTarget(int alignment)
{
	const int outputSize = getCurrentOutputSize();
	const PxU32 outPadding = getPadding(size_t(outputSize), PxU32(alignment));
	if(outPadding)
	{
		assert(outPadding<CONVX_ZERO_BUFFER_SIZE);
		output(mZeros, int(outPadding));
	}
}

const char* Sn::ConvX::alignStream(const char* buffer, int alignment)
{
	const PxU32 padding = getPadding(size_t(buffer), PxU32(alignment));
	assert(!getPadding(size_t(buffer + padding), PxU32(alignment)));

	const int outputSize = getCurrentOutputSize();
	const PxU32 outPadding = getPadding(size_t(outputSize), PxU32(alignment));
	if(outPadding==padding)
	{
		assert(outPadding<CONVX_ZERO_BUFFER_SIZE);
		output(mZeros, int(outPadding));
	}
	else if(outPadding)
	{
		assert(outPadding<CONVX_ZERO_BUFFER_SIZE);
		output(mZeros, int(outPadding));
	}

	return buffer + padding;
}
