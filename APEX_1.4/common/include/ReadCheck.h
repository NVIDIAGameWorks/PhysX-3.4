/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef READ_CHECK_H
#define READ_CHECK_H

#include "PxSimpleTypes.h"
#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

class ApexRWLockable;

// RAII wrapper around the Scene::apexStartRead() method, note that this
// object does not acquire any scene locks, it is an error checking only mechanism
class ReadCheck
{
public: 
	ReadCheck(const ApexRWLockable* scene, const char* functionName);
	~ReadCheck();

private:
	const ApexRWLockable* mLockable;
	const char* mName;
	uint32_t mErrorCount;
};

#if (PX_DEBUG || PX_CHECKED)
	// Creates a scoped read check object that detects whether appropriate scene locks
	// have been acquired and checks if reads/writes overlap, this macro should typically
	// be placed at the beginning of any const API methods that are not multi-thread safe, 
	// the error conditions checked can be summarized as:
	
	// 1. Other threads were already writing, or began writing during the object lifetime
	#define READ_ZONE() ReadCheck __readCheck(static_cast<const ApexRWLockable*>(this), __FUNCTION__);
#else
	#define READ_ZONE()
#endif

}
}

#endif // READ_CHECK_H
