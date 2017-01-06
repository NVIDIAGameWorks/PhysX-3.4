/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "Allocator.h"
#include "PsAllocator.h"

namespace nvidia
{

void* cloth::allocate(size_t n)
{
	return n ? nvidia::getAllocator().allocate(n, "", __FILE__, __LINE__) : 0;
}

void cloth::deallocate(void* ptr)
{
	if(ptr)
		nvidia::getAllocator().deallocate(ptr);
}
}
