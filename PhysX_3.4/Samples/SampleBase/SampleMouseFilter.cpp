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

// Mouse filter from ICE, refactored a bit

#include "SampleMouseFilter.h"

MouseFilter::MouseFilter() :
	mWeightModifier			(0.0f),
	mHistoryBufferLength	(0),
	mHistoryBufferX			(NULL),
	mHistoryBufferY			(NULL)
{
}

MouseFilter::MouseFilter(PxU32 length, PxReal weightModifier)
{
	mWeightModifier			= weightModifier;
	mHistoryBufferLength	= 0;
	mHistoryBufferX			= NULL;
	mHistoryBufferY			= NULL;
	SetHistoryBufferLength(length);
}

MouseFilter::~MouseFilter()
{
	SAMPLE_FREE(mHistoryBufferX);
	SAMPLE_FREE(mHistoryBufferY);
}

bool MouseFilter::SetHistoryBufferLength(PxU32 length)
{
	SAMPLE_FREE(mHistoryBufferX);
	SAMPLE_FREE(mHistoryBufferY);

	mHistoryBufferLength = length;
	if(length)
	{
		mHistoryBufferX = (float*)SAMPLE_ALLOC(sizeof(float)*length);
        memset(mHistoryBufferX, 0, length*sizeof(float)); 
		mHistoryBufferY = (float*)SAMPLE_ALLOC(sizeof(float)*length);
        memset(mHistoryBufferY, 0, length*sizeof(float));		
	}
	return true;
}

void MouseFilter::Apply(PxReal& deltaMouseX, PxReal& deltaMouseY)
{
	// Checkings
	if(!mHistoryBufferX || !mHistoryBufferY)	return;

	// Shift the buffer around. If you want performance from this, be sure
	// to use a circular buffer than these slow memmove()s.
	memmove(mHistoryBufferX+1, mHistoryBufferX, (mHistoryBufferLength-1)*sizeof(PxReal));
	memmove(mHistoryBufferY+1, mHistoryBufferY, (mHistoryBufferLength-1)*sizeof(PxReal));

	// Put the current values at the front of the history buffer
	*mHistoryBufferX = deltaMouseX;
	*mHistoryBufferY = deltaMouseY;

	// Filter the mouse
	PxReal CurAverageX	= 0.0f;
	PxReal CurAverageY	= 0.0f;
	PxReal AverageTot	= 0.0f;
	PxReal CurrentWeight	= 1.0f;
	for(PxU32 i=0;i<mHistoryBufferLength;i++)
	{
		CurAverageX += mHistoryBufferX[i] * CurrentWeight;
		CurAverageY += mHistoryBufferY[i] * CurrentWeight;

		// Note! Our total is also weighted
		AverageTot += 1.0f * CurrentWeight;

		// The weight for the next entry in the history buffer
		CurrentWeight *= mWeightModifier;
	}

	// Calculate the final weighted value
	deltaMouseX = CurAverageX / AverageTot;
	deltaMouseY = CurAverageY / AverageTot;
}
