/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RELEASER_H
#define RELEASER_H

#include <stdio.h>

#include "ApexInterface.h"
#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParameterizedTraits.h"

#define RELEASE_AT_EXIT(obj, typeName) Releaser<typeName> obj ## Releaser(obj);

template<typename T>
class Releaser
{
	Releaser(Releaser<T>& rhs);
	Releaser<T>& operator =(Releaser<T>& rhs);

public:
	Releaser(T* obj, void* memory = NULL) : mObj(obj), mMemory(memory) 
	{
	}

	~Releaser() 
	{
		doRelease();
	}

	void doRelease();

	void reset(T* newObj = NULL, void* newMemory = NULL)
	{
		if (newObj != mObj)
		{
			doRelease(mObj);
		}
		mObj = newObj;
		mMemory = newMemory;
	}

private:
	T* mObj;

	void* mMemory;
};

template<> PX_INLINE void Releaser<NvParameterized::Interface>::doRelease()
{
	if (mObj != NULL)
	{
		mObj->destroy();
	}
}

template<> PX_INLINE void Releaser<NvParameterized::Traits>::doRelease()
{
	if (mMemory != NULL)
	{
		mObj->free(mMemory);
	}
}

template<> PX_INLINE void Releaser<nvidia::apex::ApexInterface>::doRelease()
{
	if (mObj != NULL)
	{
		mObj->release();
	}
}

template<> PX_INLINE void Releaser<physx::PxFileBuf>::doRelease()
{
	if (mObj != NULL)
	{
		mObj->release();
	}
}

template<> PX_INLINE void Releaser<FILE>::doRelease()
{
	if (mObj != NULL)
	{
		fclose(mObj);
	}
}

#endif	// RESOURCE_MANAGER_H
