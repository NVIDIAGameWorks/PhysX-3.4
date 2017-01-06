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


#ifndef PXS_DEFAULT_MEMORY_MANAGER_H
#define PXS_DEFAULT_MEMORY_MANAGER_H

#include "PxsMemoryManager.h"
#include "PsAllocator.h"
#include "PsArray.h"

namespace physx
{
	
	class PxsDefaultMemoryAllocator : public Ps::VirtualAllocatorCallback
	{
	public:

		PxsDefaultMemoryAllocator(const char* name = NULL)
		{
			PX_UNUSED(name);
#if 0 //PX_USE_NAMED_ALLOCATOR
			if (name)
				strcpy(mName, name);
			else
				strcpy(mName, "");
#endif
		}

		virtual ~PxsDefaultMemoryAllocator()
		{
		}

		virtual void* allocate(const size_t newByteSize, const char* filename, const int line)
		{
			PX_UNUSED(line);
			PX_UNUSED(filename);
#if 0 //PX_USE_NAMED_ALLOCATOR
			return PX_ALLOC(newByteSize, mName);
#else
			return PX_ALLOC(newByteSize, filename);
#endif
		}

		virtual void deallocate(void* ptr)
		{
			if (ptr)
				PX_FREE(ptr);
		}

#if 0 //PX_USE_NAMED_ALLOCATOR
		char mName[32];
#endif
	};


	class PxsDefaultMemoryManager : public PxsMemoryManager
	{
	public:
		virtual ~PxsDefaultMemoryManager();
		virtual Ps::VirtualAllocatorCallback* createHostMemoryAllocator(const PxU32 gpuComputeVersion = 0);
		virtual Ps::VirtualAllocatorCallback* createDeviceMemoryAllocator(const PxU32 gpuComputeVersion = 0);

		virtual void destroyMemoryAllocator();

		Ps::Array<Ps::VirtualAllocatorCallback*> mAllocators;

	};

}

#endif
