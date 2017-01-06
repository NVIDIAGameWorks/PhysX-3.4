/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "PxErrorCallback.h"

#include <ApexDefs.h>

class SimplePxErrorStream : public physx::PxErrorCallback
{
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorCode = NULL;
		switch (code)
		{
		case physx::PxErrorCode::eNO_ERROR :
			return;
		case physx::PxErrorCode::eINVALID_PARAMETER:
			errorCode = "Invalid Parameter";
			break;
		case physx::PxErrorCode::eINVALID_OPERATION:
			errorCode = "Invalid Operation";
			break;
		case physx::PxErrorCode::eOUT_OF_MEMORY:
			errorCode = "Out of Memory";
			break;
		case physx::PxErrorCode::eINTERNAL_ERROR :
			errorCode = "Internal Error";
			break;
//		case physx::PxErrorCode::eASSERTION:
//			errorCode = "Assertion";
//			break;
		case physx::PxErrorCode::eDEBUG_INFO:
			errorCode = "Debug Info";
			break;
		case physx::PxErrorCode::eDEBUG_WARNING:
			errorCode = "Debug Warning";
			break;
//		case physx::PxErrorCode::eSERIALIZATION_ERROR:
//			errorCode = "Serialization Error";
//			break;
		default:
			errorCode = "Unknown Error Code";
		}

		if (errorCode != NULL)
		{
			printf("PhysX error: %s %s:%d\n%s\n", errorCode, file, line, message);
		}
		else
		{
			printf("PhysX error: physx::PxErrorCode is %d in %s:%d\n%s\n", code, file, line, message);
		}
	}

	virtual void print(const char* message)
	{
		printf("%s", message);
	}
};
