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

#include "foundation/PxAssert.h"

#include <stdio.h>
#include "PsString.h"

#if PX_WINDOWS_FAMILY
#include <crtdbg.h>
#elif PX_SWITCH
#include "switch/PsSwitchAbort.h"
#endif

namespace
{
class DefaultAssertHandler : public physx::PxAssertHandler
{
	virtual void operator()(const char* expr, const char* file, int line, bool& ignore)
	{
		PX_UNUSED(ignore); // is used only in debug windows config
		char buffer[1024];
#if PX_WINDOWS_FAMILY
		sprintf_s(buffer, "%s(%d) : Assertion failed: %s\n", file, line, expr);
#else
		sprintf(buffer, "%s(%d) : Assertion failed: %s\n", file, line, expr);
#endif
		physx::shdfnd::printString(buffer);
#if PX_WINDOWS_FAMILY&& PX_DEBUG
		// _CrtDbgReport returns -1 on error, 1 on 'retry', 0 otherwise including 'ignore'.
		// Hitting 'abort' will terminate the process immediately.
		int result = _CrtDbgReport(_CRT_ASSERT, file, line, NULL, "%s", buffer);
		int mode = _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_REPORT_MODE);
		ignore = _CRTDBG_MODE_WNDW == mode && result == 0;
		if(ignore)
			return;
		__debugbreak();
#elif PX_WINDOWS_FAMILY&& PX_CHECKED
		__debugbreak();
#elif PX_SWITCH
		abort(buffer);
#else
		abort();
#endif
	}
};

DefaultAssertHandler sAssertHandler;
physx::PxAssertHandler* sAssertHandlerPtr = &sAssertHandler;
}

namespace physx
{

PxAssertHandler& PxGetAssertHandler()
{
	return *sAssertHandlerPtr;
}

void PxSetAssertHandler(PxAssertHandler& handler)
{
	sAssertHandlerPtr = &handler;
}
} // end of physx namespace
