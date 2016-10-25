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

#ifndef JOB_SWARM_H

#define JOB_SWARM_H

/*!
**
** Copyright (c) 20011 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "PlatformConfigHACD.h"

namespace JOB_SWARM
{

// Any 'Job' you want to execute should inherit this class and implement the three virtual methods.
// 'job_process' defines the 'work' you want to do and runs in a seperate thread.  It must be thread safe code.
// job_onFinish is called from the main application thread and when the job has completed so you can do something with the results.
// job_onCancel is called from the main application thread when this job was cancelled so any cleanup necessary can be performed.
class JobSwarmInterface
{
public:
  virtual void job_process(void *userData,hacd::HaI32 userId) = 0;   // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
  virtual void job_onFinish(void *userData,hacd::HaI32 userId) = 0;  // runs in primary thread of the context
  virtual void job_onCancel(void *userData,hacd::HaI32 userId) = 0;  // runs in primary thread of the context
};


class SwarmJob;

// This is a single instance of a JobSwarm system.  You can have multiple JobSwarmContexts in a single application.
class JobSwarmContext
{
public:

  virtual SwarmJob *   createSwarmJob(JobSwarmInterface *iface,void *userData,hacd::HaI32 userId) = 0; // creates a job to be processed and returns a handle.
  virtual void         cancel(SwarmJob *job) = 0; // cancels the job, use cannot delete the memory until he receives the onCancel event!

  virtual bool processSwarmJobs(void) = 0; // This is a pump loop run in the main thread to handle the disposition of finished and/or cancelled jobs.  Returns true if there are still outstanding jobs not yet full procesed.
  virtual void         setUseThreads(bool state) = 0; // Whether or not to run in hardware threads.  This is for debugging only, threading is always true by default.
};

JobSwarmContext * createJobSwarmContext(hacd::HaU32 maxThreadCount=4); // create a JobSwarmContext with the give number of physical threads
bool              releaseJobSwarmContext(JobSwarmContext *c); // release a JobSwarmContet


}; // end of namespace


#endif
