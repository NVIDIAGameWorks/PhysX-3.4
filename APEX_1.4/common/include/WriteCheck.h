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


#ifndef WRITE_CHECK_H
#define WRITE_CHECK_H

#include "PxSimpleTypes.h"
#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

class ApexRWLockable;

// RAII wrapper around the Scene::startWrite() method, note that this
// object does not acquire any scene locks, it is an error checking only mechanism
class WriteCheck
{
public: 
	WriteCheck(ApexRWLockable* scene, const char* functionName, bool allowReentry=true);
	~WriteCheck();

private:
	ApexRWLockable* mLockable;
	const char* mName;
	bool mAllowReentry;
	uint32_t mErrorCount;
};

#if PX_DEBUG || PX_CHECKED
	// Creates a scoped write check object that detects whether appropriate scene locks
	// have been acquired and checks if reads/writes overlap, this macro should typically
	// be placed at the beginning of any non-const API methods that are not multi-thread safe. 
	// By default re-entrant  write calls by the same thread are allowed, the error conditions 
	// checked can be summarized as:
	
	// 1. Other threads were already reading, or began reading during the object lifetime
	// 2. Other threads were already writing, or began writing during the object lifetime
	#define WRITE_ZONE() WriteCheck __writeCheck(static_cast<ApexRWLockable*>(this), __FUNCTION__);

	// Creates a scoped write check object that disallows re-entrant writes, this is used by
	// the Scene::simulate method to detect when callbacks make write calls to the API
	#define WRITE_ZONE_NOREENTRY() WriteCheck __writeCheck(static_cast<ApexRWLockable*>(this), __FUNCTION__, false);
#else
	#define WRITE_ZONE()
	#define WRITE_ZONE_NOREENTRY()
#endif

}
}

#endif // WRITE_CHECK_H
