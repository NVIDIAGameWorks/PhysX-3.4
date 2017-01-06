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

#pragma once

#include "PxVec4.h"
#include "PxQuat.h"
#include "PxVec3.h"
#include "ApexUsingNamespace.h"

namespace nvidia
{

namespace cloth
{

inline float (&array(PxVec3& v))[3]
{
	return reinterpret_cast<float(&)[3]>(v);
}
inline const float (&array(const PxVec3& v))[3]
{
	return reinterpret_cast<const float(&)[3]>(v);
}
inline float (&array(PxVec4& v))[4]
{
	return reinterpret_cast<float(&)[4]>(v);
}
inline const float (&array(const PxVec4& v))[4]
{
	return reinterpret_cast<const float(&)[4]>(v);
}
inline float (&array(PxQuat& q))[4]
{
	return reinterpret_cast<float(&)[4]>(q);
}
inline const float (&array(const PxQuat& q))[4]
{
	return reinterpret_cast<const float(&)[4]>(q);
}

} // namespace cloth

} // namespace nvidia
