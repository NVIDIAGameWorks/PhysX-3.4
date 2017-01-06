/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SCOPED_PHYS_XLOCK_H

#define SCOPED_PHYS_XLOCK_H

#include "foundation/PxPreprocessor.h"
#include "ApexDefs.h"


#include "PxScene.h"
#include "Scene.h"

namespace nvidia
{
	namespace apex
	{

		/**
		\brief This helper class creates a scoped read access to the PhysX SDK API

		This helper class is used to create a scoped read lock/unlock pair around a section of code
		which is trying to do read access against the PhysX SDK.
		*/

		class ScopedPhysXLockRead
		{
		public:

			/**
			\brief Constructor for ScopedPhysXLockRead
			\param[in] scene the APEX scene
			\param[in] fileName used to determine what file called the lock for debugging purposes
			\param[in] lineno used to determine what line number called the lock for debugging purposes
			*/
			ScopedPhysXLockRead(nvidia::Scene* scene, const char *fileName, int lineno) : mApexScene(scene), mPhysXScene(0)
			{
				if (mApexScene)
				{
					mApexScene->lockRead(fileName, (uint32_t)lineno);
				}
			}
			
			/**
			\brief Constructor for ScopedPhysXLockRead
			\param[in] scene the PhysX scene
			\param[in] fileName used to determine what file called the lock for debugging purposes
			\param[in] lineno used to determine what line number called the lock for debugging purposes
			*/
			ScopedPhysXLockRead(physx::PxScene* scene, const char *fileName, int lineno) : mPhysXScene(scene), mApexScene(0)
			{
				if (mPhysXScene)
				{
					mPhysXScene->lockRead(fileName, (uint32_t)lineno);
				}
			}
			
			~ScopedPhysXLockRead()
			{
				if (mApexScene)
				{
					mApexScene->unlockRead();
				}
				if (mPhysXScene)
				{
					mPhysXScene->unlockRead();
				}
			}
		private:
			nvidia::Scene* mApexScene;
			physx::PxScene* mPhysXScene;
		};

		/**
		\brief This helper class creates a scoped write access to the PhysX SDK API

		This helper class is used to create a scoped write lock/unlock pair around a section of code
		which is trying to do read access against the PhysX SDK.
		*/
		class ScopedPhysXLockWrite
		{
		public:
			/**
			\brief Constructor for ScopedPhysXLockWrite
			\param[in] scene the APEX scene
			\param[in] fileName used to determine what file called the lock for debugging purposes
			\param[in] lineno used to determine what line number called the lock for debugging purposes
			*/
			ScopedPhysXLockWrite(nvidia::Scene *scene, const char *fileName, int lineno) : mApexScene(scene), mPhysXScene(0)
			{
				if (mApexScene)
				{
					mApexScene->lockWrite(fileName, (uint32_t)lineno);
				}
			}
			
			/**
			\brief Constructor for ScopedPhysXLockWrite
			\param[in] scene the PhysX scene
			\param[in] fileName used to determine what file called the lock for debugging purposes
			\param[in] lineno used to determine what line number called the lock for debugging purposes
			*/
			ScopedPhysXLockWrite(physx::PxScene *scene, const char *fileName, int lineno) : mPhysXScene(scene), mApexScene(0)
			{
				if (mPhysXScene)
				{
					mPhysXScene->lockWrite(fileName, (uint32_t)lineno);
				}
			}
			
			~ScopedPhysXLockWrite()
			{
				if (mApexScene)
				{
					mApexScene->unlockWrite();
				}
				if (mPhysXScene)
				{
					mPhysXScene->unlockWrite();
				}
			}
		private:
			nvidia::Scene* mApexScene;
			physx::PxScene* mPhysXScene;
		};
	}
}


#if defined(_DEBUG) || PX_CHECKED
/**
\brief This macro creates a scoped write lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_WRITE(x) nvidia::apex::ScopedPhysXLockWrite _wlock(x,__FILE__,__LINE__);
#else
/**
\brief This macro creates a scoped write lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_WRITE(x) nvidia::apex::ScopedPhysXLockWrite _wlock(x,"",0);
#endif

#if defined(_DEBUG) || PX_CHECKED
/**
\brief This macro creates a scoped read lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_READ(x) nvidia::apex::ScopedPhysXLockRead _rlock(x,__FILE__,__LINE__);
#else
/**
\brief This macro creates a scoped read lock/unlock pair
*/
#define SCOPED_PHYSX_LOCK_READ(x) nvidia::apex::ScopedPhysXLockRead _rlock(x,"",0);
#endif



#endif
