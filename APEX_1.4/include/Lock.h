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


#ifndef LOCK_H
#define LOCK_H

/*!
\file
\brief classes PxSceneReadLock, PxSceneWriteLock
*/

#include "ApexInterface.h"

namespace nvidia
{
namespace apex
{

/**
\brief RAII wrapper for the Scene read lock.

Use this class as follows to lock the scene for reading by the current thread 
for the duration of the enclosing scope:

	ReadLock lock(sceneRef);

\see Scene::apexacquireReadLock(), Scene::apexUnacquireReadLock(), SceneDesc::useRWLock
*/
class ReadLock
{
	ReadLock(const ReadLock&);
	ReadLock& operator=(const ReadLock&);

public:
	
	/**
	\brief Constructor
	\param lockable The object to lock for reading
	\param file Optional string for debugging purposes
	\param line Optional line number for debugging purposes
	*/
	ReadLock(const ApexInterface& lockable, const char* file=NULL, uint32_t line=0)
		: mLockable(lockable)
	{
		mLockable.acquireReadLock(file, line);
	}

	~ReadLock()
	{
		mLockable.releaseReadLock();
	}

private:

	const ApexInterface& mLockable;
};

/**
\brief RAII wrapper for the Scene write lock.

Use this class as follows to lock the scene for writing by the current thread 
for the duration of the enclosing scope:

	WriteLock lock(sceneRef);

\see Scene::apexacquireWriteLock(), Scene::apexUnacquireWriteLock(), SceneDesc::useRWLock
*/
class WriteLock
{
	WriteLock(const WriteLock&);
	WriteLock& operator=(const WriteLock&);

public:

	/**
	\brief Constructor
	\param lockable The object to lock for writing
	\param file Optional string for debugging purposes
	\param line Optional line number for debugging purposes
	*/
	WriteLock(const ApexInterface& lockable, const char* file=NULL, uint32_t line=0)
		: mLockable(lockable)
	{
		mLockable.acquireWriteLock(file, line);
	}

	~WriteLock()
	{
		mLockable.releaseWriteLock();
	}

private:
	const ApexInterface& mLockable;
};


} // namespace apex
} // namespace nvidia

/**
\brief Lock an object for writing by the current thread for the duration of the enclosing scope.
*/
#define WRITE_LOCK(LOCKABLE) nvidia::apex::WriteLock __WriteLock(LOCKABLE, __FILE__, __LINE__);
/**
\brief Lock an object for reading by the current thread for the duration of the enclosing scope.
*/
#define READ_LOCK(LOCKABLE) nvidia::apex::ReadLock __ReadLock(LOCKABLE, __FILE__, __LINE__);

/** @} */
#endif
