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

#include "foundation/PxErrorCallback.h"
#include "SnConvX.h"
#include <stdarg.h>
#include "PsString.h"
#include "PsFoundation.h"

#define MAX_DISPLAYED_ISSUES   10

using namespace physx;

void Sn::ConvX::resetNbErrors()
{
	mNbErrors = 0;
	mNbWarnings = 0;
}

int Sn::ConvX::getNbErrors() const
{
	return mNbErrors;
}

void Sn::ConvX::displayMessage(PxErrorCode::Enum code, const char* format, ...)
{
	if(silentMode())
		return;
		
	int sum = mNbWarnings + mNbErrors;
	if(sum >= MAX_DISPLAYED_ISSUES)
		return;

	bool display = false;

	if(code==PxErrorCode::eINTERNAL_ERROR || code==PxErrorCode::eINVALID_OPERATION || code==PxErrorCode::eINVALID_PARAMETER)
	{
		mNbErrors++;
		display = true;
	}
	else if(code == PxErrorCode::eDEBUG_WARNING)
	{
		mNbWarnings++;	
		display = true;
	}

	if(display || ((sum == 0) && verboseMode()) )
	{
		va_list va;
		va_start(va, format);
		Ps::getFoundation().errorImpl(code, __FILE__, __LINE__, format, va);
		va_end(va); 
	}

	if(display)
	{
		if( sum == 0)
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__, "Hit warnings or errors: skipping further verbose output.\n");		    
		}
		else if(sum == MAX_DISPLAYED_ISSUES-1)
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__, "Exceeding 10 warnings or errors: skipping further output.\n");		    
		}
	}

	return;
}
