/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CUDA_TEST_MANAGER_H
#define CUDA_TEST_MANAGER_H

/*!
\file
\brief classes CudaTestManager
*/

#include <ApexUsingNamespace.h>

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/** 
 */

 /**
\brief A class that set information on kernels should be tested and give directive for creation ApexCudaTestContext
*/
class CudaTestManager
{
public:
	 /**
	\brief Set the write path
	*/
	virtual void setWritePath(const char* path) = 0;

	 /**
	\brief Set write for function
	*/
	virtual void setWriteForFunction(const char* functionName, const char* moduleName) = 0;

	 /**
	\brief Set the maximum number of frames
	*/
	virtual void setMaxSamples(uint32_t maxFrames) = 0;

	 /**
	\brief Set the frames
	*/
	virtual void setFrames(uint32_t numFrames, const uint32_t* frames) = 0;
	
	 /**
	\brief Set the frame period
	*/
	virtual void setFramePeriod(uint32_t period) = 0;

	/**
	\brief Set maximal count of kernel calls per frame 
	*/
	virtual void setCallPerFrameMaxCount(uint32_t cpfMaxCount) = 0;

	/**
	\brief Set flag for writing kernel context for kernel that return not successful error code
	*/
	virtual void setWriteForNotSuccessfulKernel(bool flag) = 0;

	 /**
	\brief Run a kernel
	*/
	virtual bool runKernel(const char* path) = 0;
};

PX_POP_PACK

}
}

#endif // CUDA_TEST_MANAGER_H

