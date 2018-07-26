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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.

#ifndef PXTASK_PXTASK_H
#define PXTASK_PXTASK_H

#include "task/PxTaskDefine.h"
#include "task/PxTaskManager.h"
#include "task/PxCpuDispatcher.h"
#include "task/PxGpuDispatcher.h"
#include "foundation/PxAssert.h"

namespace physx
{

/**
 * \brief Base class of all task types
 *
 * PxBaseTask defines a runnable reference counted task with built-in profiling.
 */
class PxBaseTask
{
public:
	PxBaseTask() : mContextID(0), mTm(NULL) {}
	virtual ~PxBaseTask() {}

    /**
     * \brief The user-implemented run method where the task's work should be performed
     *
     * run() methods must be thread safe, stack friendly (no alloca, etc), and
     * must never block.
     */
    virtual void        run() = 0;

    /**
     * \brief Return a user-provided task name for profiling purposes.
     *
     * It does not have to be unique, but unique names are helpful.
	 *
	 * \return The name of this task
     */
    virtual const char*	getName() const = 0;

    //! \brief Implemented by derived implementation classes
    virtual void		addReference() = 0;
    //! \brief Implemented by derived implementation classes
    virtual void		removeReference() = 0;
	//! \brief Implemented by derived implementation classes
	virtual int32_t		getReference() const = 0;

    /** \brief Implemented by derived implementation classes
	 *
	 * A task may assume in its release() method that the task system no longer holds 
	 * references to it - so it may safely run its destructor, recycle itself, etc.
	 * provided no additional user references to the task exist
	 */
    virtual void		release() = 0;

    /**
     * \brief Return PxTaskManager to which this task was submitted
     *
     * Note, can return NULL if task was not submitted, or has been
     * completed.
     */
	PX_FORCE_INLINE PxTaskManager* getTaskManager() const
	{
		return mTm;
	}

	PX_FORCE_INLINE	void	setContextId(PxU64 id)			{ mContextID = id;		}
	PX_FORCE_INLINE	PxU64	getContextId()			const	{ return mContextID;	}

protected:
	PxU64				mContextID;		//!< Context ID for profiler interface
	PxTaskManager*		mTm;			//!< Owning PxTaskManager instance

	friend class PxTaskMgr;
};


/**
 * \brief A PxBaseTask implementation with deferred execution and full dependencies
 *
 * A PxTask must be submitted to a PxTaskManager to to be executed, Tasks may
 * optionally be named when they are submitted.
 */
class PxTask : public PxBaseTask
{
public:
	PxTask() : mTaskID(0) {}
	virtual ~PxTask() {}

    //! \brief Release method implementation
    virtual void release()
	{
		PX_ASSERT(mTm);

        // clear mTm before calling taskCompleted() for safety
		PxTaskManager* save = mTm;
		mTm = NULL;
		save->taskCompleted( *this );
	}

    //! \brief Inform the PxTaskManager this task must finish before the given
    //         task is allowed to start.
    PX_INLINE void finishBefore( PxTaskID taskID )
	{
		PX_ASSERT(mTm);
		mTm->finishBefore( *this, taskID);
	}

    //! \brief Inform the PxTaskManager this task cannot start until the given
    //         task has completed.
    PX_INLINE void startAfter( PxTaskID taskID )
	{
		PX_ASSERT(mTm);
		mTm->startAfter( *this, taskID );
	}

    /**
     * \brief Manually increment this task's reference count.  The task will
     * not be allowed to run until removeReference() is called.
     */
    PX_INLINE void addReference()
	{
		PX_ASSERT(mTm);
		mTm->addReference( mTaskID );
	}

    /**
     * \brief Manually decrement this task's reference count.  If the reference
     * count reaches zero, the task will be dispatched.
     */
    PX_INLINE void removeReference()
	{
		PX_ASSERT(mTm);
		mTm->decrReference( mTaskID );
	}

	/** 
	 * \brief Return the ref-count for this task 
	 */
	PX_INLINE int32_t getReference() const
	{
		return mTm->getReference( mTaskID );
	}
	
	/**
	 * \brief Return the unique ID for this task
	 */
	PX_INLINE PxTaskID	    getTaskID() const
	{
		return mTaskID;
	}

	/**
	 * \brief Called by PxTaskManager at submission time for initialization
	 *
	 * Perform simulation step initialization here.
	 */
	virtual void submitted()
	{
		mStreamIndex = 0;
		mPreSyncRequired = false;
	}

	/**
	 * \brief Specify that the GpuTask sync flag be set
	 */
	PX_INLINE void		requestSyncPoint()
	{
		mPreSyncRequired = true;
	}


protected:
	PxTaskID			mTaskID;			//!< ID assigned at submission
	uint32_t			mStreamIndex;		//!< GpuTask CUDA stream index
	bool				mPreSyncRequired;	//!< GpuTask sync flag

	friend class PxTaskMgr;
	friend class PxGpuWorkerThread;
};


/**
 * \brief A PxBaseTask implementation with immediate execution and simple dependencies
 *
 * A PxLightCpuTask bypasses the PxTaskManager launch dependencies and will be
 * submitted directly to your scene's CpuDispatcher.  When the run() function
 * completes, it will decrement the reference count of the specified
 * continuation task.
 *
 * You must use a full-blown PxTask if you want your task to be resolved
 * by another PxTask, or you need more than a single dependency to be
 * resolved when your task completes, or your task will not run on the
 * CpuDispatcher.
 */
class PxLightCpuTask : public PxBaseTask
{
public:
	PxLightCpuTask()
		: mCont( NULL )
		, mRefCount( 0 )
	{
	}
	virtual ~PxLightCpuTask()
	{
		mTm = NULL;
	}

    /**
     * \brief Initialize this task and specify the task that will have its ref count decremented on completion.
     *
     * Submission is deferred until the task's mRefCount is decremented to zero.  
	 * Note that we only use the PxTaskManager to query the appropriate dispatcher.
	 *
	 * \param[in] tm The PxTaskManager this task is managed by
	 * \param[in] c The task to be executed when this task has finished running
	 */
	PX_INLINE void setContinuation(PxTaskManager& tm, PxBaseTask* c)
	{
		PX_ASSERT( mRefCount == 0 );
		mRefCount = 1;
		mCont = c;
		mTm = &tm;
		if( mCont )
		{
			mCont->addReference();
	    }
	}

    /**
     * \brief Initialize this task and specify the task that will have its ref count decremented on completion.
     *
     * This overload of setContinuation() queries the PxTaskManager from the continuation
     * task, which cannot be NULL.
	 * \param[in] c The task to be executed after this task has finished running
	 */
	PX_INLINE void setContinuation( PxBaseTask* c )
	{
		PX_ASSERT( c );
		PX_ASSERT( mRefCount == 0 );
		mRefCount = 1;
		mCont = c;
		if( mCont )
		{
			mCont->addReference();
			mTm = mCont->getTaskManager();
			PX_ASSERT( mTm );
		}
	}

    /**
     * \brief Retrieves continuation task
	 */
	PX_INLINE PxBaseTask*	getContinuation()	const
	{
		return mCont;
	}

    /**
     * \brief Manually decrement this task's reference count.  If the reference
     * count reaches zero, the task will be dispatched.
     */
	PX_INLINE void removeReference()
	{
		mTm->decrReference(*this);
	}

	/** \brief Return the ref-count for this task */
	PX_INLINE int32_t getReference() const
	{
		return mRefCount;
	}

    /**
     * \brief Manually increment this task's reference count.  The task will
     * not be allowed to run until removeReference() is called.
     */
	PX_INLINE void addReference()
	{
		mTm->addReference(*this);
	}

    /**
     * \brief called by CpuDispatcher after run method has completed
     *
     * Decrements the continuation task's reference count, if specified.
     */
	PX_INLINE void release()
	{
		if( mCont )
		{
			mCont->removeReference();
	    }
	}

protected:

	PxBaseTask*			mCont;          //!< Continuation task, can be NULL
	volatile int32_t	mRefCount;      //!< PxTask is dispatched when reaches 0

	friend class PxTaskMgr;
};


}// end physx namespace


#endif // PXTASK_PXTASK_H
