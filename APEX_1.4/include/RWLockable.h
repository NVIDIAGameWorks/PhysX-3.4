/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RWLOCKABLE_H
#define RWLOCKABLE_H

/**
\file
\brief The include containing the interface for any rw-lockable object in the APEX SDK
*/

#include "foundation/PxSimpleTypes.h"

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

/**
 * \brief Base class for any rw-lockable object implemented by APEX SDK
 */
class RWLockable
{
public:
	/**
	\brief Acquire RW lock for read access.
	
	The APEX 1.3.3 SDK (and higher) provides a multiple-reader single writer mutex lock to coordinate	
	access to the APEX SDK API from multiple concurrent threads.  This method will in turn invoke the
	lockRead call on the APEX Scene.  The source code fileName and line number are provided for debugging
	purposes.
	*/
	virtual void acquireReadLock(const char *fileName, const uint32_t lineno) const = 0;

	/**
	\brief Acquire RW lock for write access.
	
	The APEX 1.3.3 SDK (and higher) provides a multiple-reader single writer mutex lock to coordinate	
	access to the APEX SDK API from multiple concurrent threads.  This method will in turn invoke the
	lockRead call on the APEX Scene.  The source code fileName and line number are provided for debugging
	purposes.
	*/
	virtual void acquireWriteLock(const char *fileName, const uint32_t lineno) const = 0;

	/**
	\brief Release the RW read lock
	*/
	virtual void releaseReadLock(void) const = 0;

	/**
	\brief Release the RW write lock
	*/
	virtual void releaseWriteLock(void) const = 0;
};

}
}
#endif // RWLOCKABLE_H
