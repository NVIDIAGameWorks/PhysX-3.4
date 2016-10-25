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
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

#include <assert.h>
#include <stdio.h>
#include <new>
#include <windows.h>
#include <PxAllocatorCallback.h>
#include <PxErrorCallback.h>
#include <PsFoundation.h>
#include "PxUserProfilerCallback.h"
#include "PxSimpleTypes.h"
#include "PsHashMap.h"
#include "SimpleHash.h"
#include "PsMutex.h"
#include "PxUserProfilerParent.h"

namespace physx
{
	using namespace pubfnd;
	using namespace shdfnd;
};

#include "MemTracker.h"

#include "PxProfileZone.h"
#include "PxProfileZoneManager.h"
#include "PxProfileEventSystem.h"
#include "PxProfileEventHandler.h"
#include "PxProfileEventNames.h"
#include "PxProfileZone.h"
#include "PxProfileScopedEvent.h"
#include "PxProfileCompileTimeEventFilter.h"
#include "PxProfileScopedEvent.h"
#include "PVDBinding.h"
#include "PvdConnectionType.h"
#include "PVDBindingUserAllocator.h"
#include "PVDBindingErrorStream.h"

//#pragma comment(lib,"PhysXProfileSDK.lib")

namespace PVD
{

struct SLocalFoundation
{
	static PVDBindingUserAllocator sAllocator;
	physx::profile::PVDBindingErrorStream mErrorStream;
	SLocalFoundation()
	{
		using namespace physx::shdfnd;
		Foundation::createInstance(PX_PUBLIC_FOUNDATION_VERSION, mErrorStream, sAllocator);
	}
	~SLocalFoundation()
	{
		using namespace physx::shdfnd;
		Foundation::destroyInstance();
	}
};

PVDBindingUserAllocator SLocalFoundation::sAllocator("");

}


namespace physx
{
    using namespace physx::pubfnd3;
};

#ifdef WIN32
#include <windows.h>
#endif

#pragma warning(disable:4100 4996)

using namespace physx;

namespace PX_USER_PROFILER_CALLBACK
{

typedef SimpleHash< PxU16 > EventPVDMap;

class PxUserProfilerCallbackPVD;

class PxUserProfilerCallbackPVD : public PxUserProfilerCallback, public UserAllocated
{
public:
	PxUserProfilerCallbackPVD(PxUserProfilerParent *parent,const PxUserProfilerCallback::Desc &desc)
	{
		mActive = desc.profilerActive;
		mParent = parent;
		mOwnsPvdBinding = false;
		if ( desc.useMemTracker )
		{
			mMemTracker = createMemTracker();
			mMemTracker->setLogLevel(desc.logEveryAllocation,desc.logEveryFrame,desc.verifySingleThreaded);
		}
		else
		{
			mMemTracker = NULL;
		}
		mPvdBinding = desc.pvdBinding;
		if ( desc.createProfilerContext )
		{
			// TODO: Create PVD binding here!
			mPvdBinding = &PVD::PvdBinding::create( false );
			//Attempt to connect automatically to pvd.
			mPvdBinding->connect( "localhost", DEFAULT_PVD_BINDING_PORT, DEFAULT_PVD_BINDING_TIMEOUT_MS, PVD::PvdConnectionType::Profile | PVD::PvdConnectionType::Memory );
			mOwnsPvdBinding = true;
		}
		mProfileZone = NULL;
		if ( mPvdBinding )
		{
			mProfileZone = &mPvdBinding->getProfileManager().createProfileZone("PxUserProfilerCallback",PxProfileNames(),0x4000);
		}
	}

  virtual ~PxUserProfilerCallbackPVD(void)
  {
    if ( mMemTracker )
    {
      releaseMemTracker(mMemTracker);
    }
	releaseProfiler();
  }
  void releaseProfiler(void)
  {
	if ( mProfileZone )
	{
		mProfileZone->release();
		mProfileZone = NULL;
	}
	if ( mPvdBinding && mOwnsPvdBinding )
	{
		mPvdBinding->release();
		mPvdBinding = NULL;
		mOwnsPvdBinding = false;
	}
  }


	virtual bool trackInfo(const void *mem,TrackInfo &info)
	{
		bool ret = false;
		if ( mMemTracker )
		{
			mMemMutex.lock();
			ret = mMemTracker->trackInfo(mem,info);
			mMemMutex.unlock();
		}
		return ret;
	}

  virtual void trackAlloc(void *mem,size_t size,MemoryType type,const char *context,const char *className,const char *fileName,physx::PxU32 lineno)
  {
	  if ( mMemTracker )
	  {
		mMemMutex.lock();
		mMemTracker->trackAlloc( getCurrentThreadId(),
								 mem,
								 size,
								 type,
								 context,
								 className,
								 fileName,
								 lineno);
		mMemMutex.unlock();
	  }
  }

  virtual void trackRealloc(void *oldMem,
	  void *newMem,
	  size_t newSize,
	  const char *context,
	  const char *className,
	  const char *fileName,
	  physx::PxU32 lineno)
  {
	  if ( mMemTracker )
	  {
		   mMemMutex.lock();
			mMemTracker->trackRealloc( getCurrentThreadId(),
									   oldMem,
									   newMem,
									   newSize,
									   context,
									   className,
									   fileName,
									   lineno);
			mMemMutex.unlock();
	  }
  }



  virtual void trackFree(void *mem,MemoryType type,const char *context,const char *fileName,physx::PxU32 lineno)
  {
	  if ( mMemTracker )
	  {
			  mMemMutex.lock();
			mMemTracker->trackFree(getCurrentThreadId(),
								   mem,
								   type,
								   context,
								   fileName,
								   lineno);
			mMemMutex.unlock();
	  }
  }

  virtual const char * trackValidateFree(void *mem,MemoryType type,const char *context,const char *fileName,physx::PxU32 lineno)
  {
	  const char *ret = NULL;
	  if ( mMemTracker )
	  {
		  mMemMutex.lock();
		  ret = mMemTracker->trackValidateFree(getCurrentThreadId(),
			  mem,
			  type,
			  context,
			  fileName,
			  lineno);
		  mMemMutex.unlock();
	  }
	  return ret;
  }


  size_t getCurrentThreadId(void)
  {
	  size_t ret = 0;

#ifdef WIN32
	  ret = GetCurrentThreadId();
#endif
	  return ret;
  }

  virtual bool memoryReport(MemoryReportFormat format,const char *fname,bool reportAllLeaks) // detect memory leaks and, if any, write out a report to the filename specified.
  {
    bool ret = false;

    if ( mMemTracker )
    {
		mMemMutex.lock();
      size_t leakCount;
      size_t leaked = mMemTracker->detectLeaks(leakCount);
      if ( leaked )
      {
		  physx::PxU32 dataLen;
        void *mem = mMemTracker->generateReport(format,fname,dataLen,reportAllLeaks);
		if ( mem )
		{
			FILE *fph = fopen(fname,"wb");
			fwrite(mem,dataLen,1,fph);
			fclose(fph);
			mMemTracker->releaseReportMemory(mem);
		}
        ret = true; // it leaked memory!
      }
	  mMemMutex.unlock();
    }
    return ret;
  }

  	/**
	\brief This method is used to generate a memory usage report in memory.  If it returns NULL then no active memory blocks are registered. (i.e. no leaks on application exit.
	*/
	virtual const void * memoryReport(MemoryReportFormat format,	// The format to output the report in.
									const char *reportName,			// The name of the report.
									bool reportAllLeaks,			// Whether or not to output every single memory leak.  *WARNING* Only enable this if you know you wish to identify a specific limited number of memory leaks. 
									physx::PxU32 &reportSize)	// The size of the output report.
	{
		const void *ret = NULL;

		if ( mMemTracker )
		{
			mMemMutex.lock();
			size_t leakCount;
			size_t leaked = mMemTracker->detectLeaks(leakCount);
			if ( leaked )
			{
				physx::PxU32 dataLen;
				void *mem = mMemTracker->generateReport(format,reportName,dataLen,reportAllLeaks);
				if ( mem )
				{
					ret = mem;
					reportSize = dataLen;
				}
			}
			mMemMutex.unlock();
		}
		return ret;
	}

	/**
	\brief Releases the memory allocated for a previous memory report.
	*/
	virtual void		releaseMemoryReport(const void *reportData)
	{
		if ( mMemTracker )
		{
			mMemMutex.lock();
			mMemTracker->releaseReportMemory((void *)reportData);
			mMemMutex.unlock();
		}
	}


  virtual void profileStat(const char *statName,		// The descriptive name of this statistic
	  PxU64 context,		// The context of this statistic. Data is presented 'in context' by PVD.  This parameter is ignored by other profiler tools.
	  physx::PxI64 statValue,		// Up to a 16 bit signed integer value for the statistic.
	  const char *fileName,		// The source code file name where this statistic event was generated.
	  physx::PxU32 lineno)	// The source code line number where this statistic event was generated.
	{
		if ( mProfileZone )
		{
			PxU16 eventId = mProfileZone->getEventIdForName(statName);
			mProfileZone->eventValue(eventId,context,statValue);
		}
	}

  // Mark the beginning of a profile zone.
	virtual void profileBegin(const char *str,physx::pubfnd3::PxU64 context,const char *fileName,physx::PxU32 lineno) 
   {
	if ( mProfileZone )
	{
		PxU16 eventId = mProfileZone->getEventIdForName(str);
		mProfileZone->startEvent(eventId,context);
	}
  }

  virtual void profileBegin(const char *str,physx::pubfnd3::PxU64 context,physx::pubfnd3::PxU32 threadId,const char *fileName,physx::PxU32 lineno) 
  {
	  if ( mProfileZone )
	  {
		  PxU16 eventId = mProfileZone->getEventIdForName(str);
		  mProfileZone->startEvent(eventId,context,threadId);
	  }
  }


  // Mark the end of a profile zone
  virtual void profileEnd(const char *event,physx::pubfnd3::PxU64 context)
  {
	  if ( mProfileZone )
	  {
			PxU16 eventId = mProfileZone->getEventIdForName(event);
			mProfileZone->stopEvent(eventId,context);
	  }
  }

  virtual void profileEnd(const char *event,physx::pubfnd3::PxU64 context,physx::pubfnd3::PxU32 threadId)
  {
	  if ( mProfileZone )
	  {
			PxU16 eventId = mProfileZone->getEventIdForName(event);
			mProfileZone->stopEvent(eventId,context,threadId);
	  }
  }


  void profileMessageChannel(MessageChannelType type,const char *channel,const char *message,...)
  {
  }

  virtual void profileSectionName(const char *fmt,...) 
  {
  }

  virtual void profileFrame(void) 
  {
	  mMemMutex.lock();
	  if ( mMemTracker )
	  {
		  mMemTracker->trackFrame();
		  mMemTracker->plotStats();
	  }
	  mMemMutex.unlock();
	  //If we don't own the pvd binding then apex or physx will be providing frame markers
	  if ( mOwnsPvdBinding ) 
	  {
		  PxU64 instPtr = static_cast<PxU64>( reinterpret_cast<size_t>( this ) );
		  mPvdBinding->endFrame( instPtr );
		  mPvdBinding->beginFrame( instPtr );
	  }
  }

  virtual void release(void)
  {
	  PxUserProfilerParent *parent = mParent;
	  delete this;
	  parent->release();
  }

  virtual void	objectInit(void *mem,const char *object,const char *fileName,physx::PxU32 lineno)
  {
  }

  virtual void	objectKill(void *mem,const char *fileName,physx::PxU32 lineno)
  {
  }

  virtual void	objectInitArray(void *mem,size_t objectSize,size_t arraySize,const char *object,const char *fileName,physx::PxU32 lineno) 
  {
  }

  virtual void	objectKillArray(void *mem,const char *fileName,physx::PxU32 lineno) 
  {
  }

	virtual void	setProfilerHandle(Type type,void *context)
	{
		if ( type == PT_PVD )
		{
			releaseProfiler();
			{
				mPvdBinding = (PVD::PvdBinding *)context;
				if ( mPvdBinding )
					mProfileZone = &mPvdBinding->getProfileManager().createProfileZone("PxUserProfilerCallback",PxProfileNames(),0x4000);
			}
		}
	}

	virtual void	*getProfilerHandle(void)
	{
		return mPvdBinding;
	}

	/**
	\brief Reports the 'Type' of 3rd party profiler SDK being used.
	*/
	virtual Type	getProfilerType(void)
	{
		return PT_PVD;
	}

	virtual void setProfilerActive(bool state)
	{
		mActive = state;
	}

	virtual bool getProfilerActive(void) const
	{
		return mActive;
	}

		/**
	\brief Processes a single data point to be plotted by the profiler. 
	*/
	virtual void	profilePlot(PlotType type,				// How to display/format the data.
								physx::PxF32 value,			// The data value to plot.
								const char *name)		// The name of the data item.
	{

	}

	/**
	\brief Processes a single data point to be plotted by the profiler. 
	*/
	virtual void	profilePlot(PlotType type,				// How to display/format the data.
								physx::PxF64 value,			// The data value to plot.
								const char *name)		// The name of the data item.
	{

	}

	/**
	\brief Processes a single data point to be plotted by the profiler. 
	*/
	virtual void	profilePlot(PlotType type,				// How to display/format the data.
								PxU64 value,			// The data value to plot.
								const char *name)		// The name of the data item.
	{

	}

	/**
	\brief Processes a single data point to be plotted by the profiler. 
	*/
	virtual void	profilePlot(PlotType type,				// How to display/format the data.
								physx::PxI64 value,			// The data value to plot.
								const char *name)		// The name of the data item.
	{

	}

		/**
	\brief Processes a single data point to be plotted by the profiler. 
	*/
	virtual void	profilePlot(PlotType type,				// How to display/format the data.
								physx::PxU32 value,			// The data value to plot.
								const char *name)		// The name of the data item.
	{

	}

	/**
	\brief Processes a single data point to be plotted by the profiler. 
	*/
	virtual void	profilePlot(PlotType type,				// How to display/format the data.
								physx::PxI32 value,			// The data value to plot.
								const char *name)		// The name of the data item.
	{

	}




private:
	bool					mActive;
	MemTracker				*mMemTracker;
	PVD::PvdBinding			*mPvdBinding;
	physx::PxProfileZone	*mProfileZone;
	physx::Mutex			mMemMutex;
	PxUserProfilerParent	*mParent;
	bool					mOwnsPvdBinding;
};


PxUserProfilerCallback * createPxUserProfilerCallbackPVD(PxUserProfilerParent *parent,const PxUserProfilerCallback::Desc &desc)
{
	PxUserProfilerCallbackPVD *ret = PX_NEW(PxUserProfilerCallbackPVD)(parent,desc);
	return static_cast<PxUserProfilerCallback *>(ret);
}



};