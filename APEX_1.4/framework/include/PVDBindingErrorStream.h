/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PVDBINDING_ERROR_STREAM_H
#define PVDBINDING_ERROR_STREAM_H

#include "PxErrorCallback.h"
#include "PxProfileBase.h"

#include <stdio.h>

namespace physx { namespace profile {

inline void printInfo(const char* format, ...)
{
	PX_UNUSED(format); 
#if PRINT_TEST_INFO
	va_list va;
	va_start(va, format);
	vprintf(format, va);
	va_end(va); 
#endif
}

class PVDBindingErrorStream : public PxErrorCallback
{
public:

	PVDBindingErrorStream() {}
	void reportError(PxErrorCode::Enum e, const char* message, const char* file, int line)
	{
		PX_UNUSED(line); 
		PX_UNUSED(file); 
		switch (e)
		{
		case PxErrorCode::eINVALID_PARAMETER:
			printf( "on invalid parameter: %s", message );
			break;
		case PxErrorCode::eINVALID_OPERATION:
			printf( "on invalid operation: %s", message );
			break;
		case PxErrorCode::eOUT_OF_MEMORY:
			printf( "on out of memory: %s", message );
			break;
		case PxErrorCode::eDEBUG_INFO:
			printf( "on debug info: %s", message );
			break;
		case PxErrorCode::eDEBUG_WARNING:
			printf( "on debug warning: %s", message );
			break;
		default:
			printf( "on unknown error: %s", message );
			break;
		}
	}
};

}}

#endif // PVDBINDING_ERROR_STREAM_H
