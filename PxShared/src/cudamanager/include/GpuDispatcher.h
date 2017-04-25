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

#ifndef PXTASK_GPUDISPATCHER_H
#define PXTASK_GPUDISPATCHER_H

#include "task/PxTask.h"
#include "task/PxTaskDefine.h"
#include "task/PxGpuTask.h"
#include "task/PxTaskManager.h"
#include "task/PxGpuDispatcher.h"
#include "foundation/PxProfiler.h"

#include "PsUserAllocated.h"
#include "PsThread.h"
#include "PsAtomic.h"
#include "PsMutex.h"
#include "PsSync.h"
#include "PsArray.h"

#include <cuda.h>

namespace physx { 

typedef uint16_t EventID;

void releaseGpuDispatcher(PxGpuDispatcher&);

class KernelWrangler;
class BlockingWaitThread;
class FanoutTask;
class LaunchTask;
class BlockTask;
class PxGpuWorkerThread;

class GpuDispatcherImpl : public PxGpuDispatcher, public shdfnd::UserAllocated
{
public:
	GpuDispatcherImpl(PxErrorCallback& errorCallback, PxCudaContextManager& ctx);
	virtual ~GpuDispatcherImpl();

	void	start();
	void    startSimulation();
	void	startGroup();
	void    submitTask(PxTask& task);
	void    finishGroup();
	void    addCompletionPrereq(PxBaseTask& task);
	bool	failureDetected() const;
	void    forceFailureMode();
	void    stopSimulation();
	void    launchCopyKernel(PxGpuCopyDesc* desc, uint32_t count, CUstream stream);
	
	PxBaseTask&	getPreLaunchTask();
	void		addPreLaunchDependent(PxBaseTask& dependent);

	PxBaseTask&	getPostLaunchTask();
	void		addPostLaunchDependent(PxBaseTask& dependent);

	PxCudaContextManager* getCudaContextManager();

	PxGpuWorkerThread* mDispatcher;
	BlockingWaitThread* mBlockingThread;
	LaunchTask* mLaunchTask; // predecessor of tasks launching kernels
	BlockTask* mBlockTask; // continuation of tasks launching kernels
	FanoutTask* mSyncTask; // predecessor of tasks waiting for cuda context synchronize
};

class JobQueue
{
	PX_NOCOPY(JobQueue)
public:
	JobQueue() : taskarray(PX_DEBUG_EXP("PxTask*")) {}
	void push(PxTask* t)
	{
		access.lock();
		taskarray.pushBack(t);
		access.unlock();
	}
	PxTask* popBack()
	{
		access.lock();
		PxTask* t = NULL;
		if (taskarray.size())
		{
			t = taskarray.popBack();
		}
		access.unlock();
		return t;
	}
	uint32_t size()
	{
		return taskarray.size();
	}
	bool empty()
	{
		return taskarray.size() == 0;
	}

private:
	shdfnd::Array<PxTask*> taskarray;
	shdfnd::Mutex		  access;
};

class EventPool
{
	PX_NOCOPY(EventPool)
public:
	EventPool(uint32_t inflags) : flags(inflags), evarray(PX_DEBUG_EXP("CUevent")) {}
	void add(CUevent ev)
	{
		access.lock();
		evarray.pushBack(ev);
		access.unlock();
	}
	CUevent get()
	{
		access.lock();
		CUevent ev;
		if (evarray.size())
		{
			ev = evarray.popBack();
		}
		else
		{
			cuEventCreate(&ev, flags);
		}
		access.unlock();
		return ev;
	}
	bool empty() const
	{
		return evarray.size() == 0;
	}
	void clear()
	{
		access.lock();
		for (uint32_t i = 0; i < evarray.size(); i++)
		{
			cuEventDestroy(evarray[i]);
		}
		access.unlock();
	}

private:
	uint32_t flags;
	shdfnd::Array<CUevent> evarray;
	shdfnd::Mutex access;
};

class StreamCache
{
public:
	StreamCache() : sarray(PX_DEBUG_EXP("CUstream")), freeIndices(PX_DEBUG_EXP("freeIndices"))
	{
	}
	CUstream get(uint32_t s)
	{
		PX_ASSERT(s);
		return sarray[ s - 1 ];
	}
	void push(uint32_t s)
	{
		freeIndices.pushBack(s);
	}
	uint32_t popBack()
	{
		if (freeIndices.size())
		{
			return freeIndices.popBack();
		}
		else
		{
			CUstream s;
			cuStreamCreate(&s, 0);
			sarray.pushBack(s);
			return sarray.size();
		}
	}
	void reset()
	{
		freeIndices.resize(sarray.size());
		for (uint32_t i = 0 ; i < sarray.size() ; i++)
		{
			freeIndices[i] = i + 1;
		}
	}
	bool empty()
	{
		return freeIndices.size() == 0;
	}

private:
	shdfnd::Array<CUstream>	 sarray;
	shdfnd::Array<uint32_t>	 freeIndices;
};

class KernelBar
{
public:
	KernelBar()
	{
		reset();
	}
	void reset()
	{
		start = 0xffffffff;
		stop = 0;
	}

	uint32_t start;
	uint32_t stop;
};

const int SIZE_COMPLETION_RING = 1024;

struct CudaBatch
{
	CUevent		blockingEvent;
	CUstream    blockingStream; // sync on stream instead of event if lsb is zero (faster)
	PxBaseTask*   continuationTask;
};

struct ReadyTask
{
	PxGpuTask* 	task;
	uint32_t       iteration;
};

class PxGpuWorkerThread : public shdfnd::Thread
{
	PX_NOCOPY(PxGpuWorkerThread)
public:
	PxGpuWorkerThread();
	~PxGpuWorkerThread();

	void					setCudaContext(PxCudaContextManager& ctx);

	/* API to TaskManager */
	void					startSimulation();
	void					stopSimulation();

	/* API to GPU tasks */
	void					addCompletionPrereq(PxBaseTask& task);

	/* PxGpuTask execution thread */
	void					execute();
	void					pollSubmitted(shdfnd::Array<ReadyTask> *ready);
	void					processActiveTasks();
	void					flushBatch(CUevent endEvent, CUstream, PxBaseTask* task);
	void					launchCopyKernel(PxGpuCopyDesc* desc, uint32_t count, CUstream stream);

	/* Blocking wait thread */
	void					blockingWaitFunc();

	StreamCache				mCachedStreams;
	shdfnd::Array<PxBaseTask*> mCompletionTasks;
	JobQueue      			mSubmittedTaskList;
	volatile int			mActiveGroups;
	shdfnd::Sync			mInputReady;
	shdfnd::Sync			mRecordEventQueued;
	PxCudaContextManager* 	mCtxMgr;
	bool                    mNewTasksSubmitted;
	bool                    mFailureDetected;

	bool                    mUsingConcurrentStreams;

	CudaBatch				mCompletionRing[ SIZE_COMPLETION_RING ];
	volatile int            mCompletionRingPush;
	volatile int            mCompletionRingPop;

	EventPool               mCachedBlockingEvents;
	EventPool               mCachedNonBlockingEvents;

	volatile int			mCountActiveScenes;

	uint32_t*				mSmStartTimes;
	uint32_t                 mSmClockFreq;

	shdfnd::Array<ReadyTask> mReady[ PxGpuTaskHint::NUM_GPU_TASK_HINTS ];

	KernelWrangler*         mUtilKernelWrapper;

	CUevent                 mStartEvent;

	shdfnd::Mutex			mMutex;
};

class BlockingWaitThread : public shdfnd::Thread
{
public:
	BlockingWaitThread(PxGpuWorkerThread& worker) : mWorker(worker) {}
	~BlockingWaitThread() {}

	void		    execute();

protected:
	PxGpuWorkerThread& mWorker;

private:
	BlockingWaitThread& operator=(const BlockingWaitThread&);
};

#define GD_CHECK_CALL(call)  { CUresult ret = call;                          \
		if( CUDA_SUCCESS != ret ) { mFailureDetected=true; PX_ASSERT(!ret); } }

}

#endif // PXTASK_GPUDISPATCHER_H
