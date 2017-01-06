/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTH_STRUCTS_H
#define CLOTH_STRUCTS_H


namespace nvidia
{

namespace shdfnd
{
template <typename Alloc> class MutexT;
}

namespace cloth
{
	class Factory;
}

namespace clothing
{

struct ClothFactory
{
	PX_INLINE ClothFactory(cloth::Factory* f, nvidia::MutexT< ReflectionAllocator<MutexImpl> >* m) : factory(f), mutex(m) {}

	PX_INLINE void clear()
	{
		factory = NULL;
		mutex = NULL;
	}

	cloth::Factory* factory;
	nvidia::MutexT< ReflectionAllocator<MutexImpl> >* mutex;
};


}
}


#endif // CLOTH_STRUCTS_H
