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


#ifndef PX_PHYSICS_COMMON_READERWRITERLOCK
#define PX_PHYSICS_COMMON_READERWRITERLOCK

//#define PX_RAYCAST_READWRITE_LOCK

/*
Implements a full reader writer lock, i.e. multiple readers or a single writer.

Below is an implementation using just a mutex for non win32 platforms.

Disable the full reader/writer lock by default. Using the full reader writer lock
results in better performance when a lot of parallel raycasts are performed but is
more costly when only a few are performed and slows down serial raycasts.
*/

namespace physx
{
namespace Cm
{

#ifdef PX_RAYCAST_READWRITE_LOCK
	class ReaderWriterLock
	{
	public:

		PX_INLINE ReaderWriterLock()
		{
			hReaderEvent=CreateEvent(NULL,TRUE,FALSE,NULL);
			PX_ASSERT(hReaderEvent!=NULL);

			hMutex = CreateEvent(NULL,FALSE,TRUE,NULL);
			PX_ASSERT(hMutex!=NULL);

			InitializeCriticalSection(&writerMutex);
			counter = -1;
			recursionCounter=0;
		}
		PX_INLINE ~ReaderWriterLock()
		{
			if(hReaderEvent!=NULL)
				CloseHandle(hReaderEvent);
			if(hMutex!=NULL)
				CloseHandle(hMutex);
			DeleteCriticalSection(&writerMutex);
		}

		PX_INLINE void lockReader()
		{
			if(InterlockedIncrement(&counter) == 0)
			{
				WaitForSingleObject(hMutex, INFINITE);
				SetEvent(hReaderEvent);
			}

			WaitForSingleObject(hReaderEvent,INFINITE);
		}

		PX_INLINE void lockWriter()
		{
			EnterCriticalSection(&writerMutex);

			//we may already have the global mutex(really an event so we have to handle recursion ourselves)
			recursionCounter++;
			if(recursionCounter==1)
				WaitForSingleObject(hMutex, INFINITE);
		}
		PX_INLINE void unlockReader()
		{
			if(InterlockedDecrement(&counter) < 0)
			{
				ResetEvent(hReaderEvent);
				SetEvent(hMutex);
			}
		}
		PX_INLINE void unlockWriter()
		{
			recursionCounter--;
			if(recursionCounter==0)
				SetEvent(hMutex);

			LeaveCriticalSection(&writerMutex);
		}

	private:

		HANDLE hReaderEvent;
		HANDLE hMutex;
		CRITICAL_SECTION writerMutex;
		LONG counter;//count the number of readers in the lock.
		LONG recursionCounter;//handle recursive writer locking
	};
#else
	/*
	TODO: implement proper reader writer lock for other platforms, eg cell os has a native reader writer lock.
	*/

	class ReaderWriterLock
	{
		PX_NOCOPY(ReaderWriterLock)
	public:

		PX_INLINE void lockReader()
		{
			//lock.lock();
			lock.lockReader();
		}
		PX_INLINE void lockWriter()
		{
			//lock.lock();
			lock.lockWriter();
		}
		PX_INLINE void unlockReader()
		{
			//lock.unlock();
			lock.unlockReader();
		}
		PX_INLINE void unlockWriter()
		{
			//lock.unlock();
			lock.unlockWriter();
		}

	private:

		//Ps::Mutex lock;

		Ps::AtomicRwLock lock;
	};
#endif

	class TakeReaderLock
	{
	public:

		TakeReaderLock(ReaderWriterLock &l) : lock(l)
		{
			lock.lockReader();
		}
		~TakeReaderLock()
		{
			lock.unlockReader();
		}

	private:
		TakeReaderLock& operator=(const TakeReaderLock&);
		ReaderWriterLock &lock;
	};

	class TakeWriterLock
	{
	public:

		TakeWriterLock(ReaderWriterLock &l) : lock(l)
		{
			lock.lockWriter();
		}
		~TakeWriterLock()
		{
			lock.unlockWriter();
		}

	private:
		TakeWriterLock& operator=(const TakeWriterLock&);
		ReaderWriterLock &lock;
	};


} // namespace Cm

}

#endif
