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

#ifndef SAMPLE_ALLOCATOR_H
#define SAMPLE_ALLOCATOR_H

#include "foundation/PxAllocatorCallback.h"
#include "common/PxPhysXCommonConfig.h"
#include "PsMutex.h"
#include "PxTkNamespaceMangle.h"

using namespace physx;

	class PxSampleAllocator : public PxAllocatorCallback
	{
		public:
						PxSampleAllocator();
						~PxSampleAllocator();

		virtual void*	allocate(size_t size, const char* typeName, const char* filename, int line);
				void*	allocate(size_t size, const char* filename, int line)	{ return allocate(size, NULL, filename, line); }
		virtual void	deallocate(void* ptr);

		protected:
				Ps::MutexT<Ps::RawAllocator> mMutex;

				void**		mMemBlockList;
				PxU32		mMemBlockListSize;
				PxU32		mFirstFree;
				PxU32		mMemBlockUsed;

		public:
				PxI32		mNbAllocatedBytes;
				PxI32		mHighWaterMark;
				PxI32		mTotalNbAllocs;
				PxI32		mNbAllocs;
	};

	void				initSampleAllocator();
	void				releaseSampleAllocator();
	PxSampleAllocator*	getSampleAllocator();

	class SampleAllocateable
	{
		public:
		PX_FORCE_INLINE	void*	operator new		(size_t, void* ptr)													{ return ptr;	}
		PX_FORCE_INLINE	void*	operator new		(size_t size, const char* handle, const char * filename, int line)	{ return getSampleAllocator()->allocate(size, handle, filename, line);	}
		PX_FORCE_INLINE	void*	operator new[]		(size_t size, const char* handle, const char * filename, int line)	{ return getSampleAllocator()->allocate(size, handle, filename, line);	}
		PX_FORCE_INLINE	void	operator delete		(void* p)															{ getSampleAllocator()->deallocate(p);	}
		PX_FORCE_INLINE	void	operator delete		(void* p, PxU32, const char*, int)									{ getSampleAllocator()->deallocate(p);	}
		PX_FORCE_INLINE	void	operator delete		(void* p, const char*, const char *, int)							{ getSampleAllocator()->deallocate(p);	}
		PX_FORCE_INLINE	void	operator delete[]	(void* p)															{ getSampleAllocator()->deallocate(p);	}
		PX_FORCE_INLINE	void	operator delete[]	(void* p, PxU32, const char*, int)									{ getSampleAllocator()->deallocate(p);	}
		PX_FORCE_INLINE	void	operator delete[]	(void* p, const char*, const char *, int)							{ getSampleAllocator()->deallocate(p);	}
	};

	#define SAMPLE_ALLOC(x)	getSampleAllocator()->allocate(x, 0, __FILE__, __LINE__)
	#define	SAMPLE_FREE(x)	if(x)	{ getSampleAllocator()->deallocate(x); x = NULL;	}
	#define SAMPLE_NEW(x)	new(#x, __FILE__, __LINE__) x

#endif
