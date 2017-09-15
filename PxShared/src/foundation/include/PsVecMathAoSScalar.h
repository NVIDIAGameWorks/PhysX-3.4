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

#ifndef PSFOUNDATION_PSVECMATHAOSSCALAR_H
#define PSFOUNDATION_PSVECMATHAOSSCALAR_H

#if COMPILE_VECTOR_INTRINSICS
#error Scalar version should not be included when using vector intrinsics.
#endif

struct VecI16V;
struct VecU16V;
struct VecI32V;
struct VecU32V;
struct Vec4V;
typedef Vec4V QuatV;

PX_ALIGN_PREFIX(16)
struct FloatV
{
	PxF32 x;
	PxF32 pad[3];
	FloatV()
	{
	}
	FloatV(const PxF32 _x) : x(_x)
	{
	}
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Vec4V
{
	PxF32 x, y, z, w;
	Vec4V()
	{
	}
	Vec4V(const PxF32 _x, const PxF32 _y, const PxF32 _z, const PxF32 _w) : x(_x), y(_y), z(_z), w(_w)
	{
	}
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct Vec3V
{
	PxF32 x, y, z;
	PxF32 pad;
	Vec3V()
	{
	}
	Vec3V(const PxF32 _x, const PxF32 _y, const PxF32 _z) : x(_x), y(_y), z(_z), pad(0.0f)
	{
	}
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct BoolV
{
	PxU32 ux, uy, uz, uw;
	BoolV()
	{
	}
	BoolV(const PxU32 _x, const PxU32 _y, const PxU32 _z, const PxU32 _w) : ux(_x), uy(_y), uz(_z), uw(_w)
	{
	}
} PX_ALIGN_SUFFIX(16);

struct Mat33V
{
	Mat33V()
	{
	}
	Mat33V(const Vec3V& c0, const Vec3V& c1, const Vec3V& c2) : col0(c0), col1(c1), col2(c2)
	{
	}
	Vec3V col0;
	Vec3V col1;
	Vec3V col2;
};

struct Mat34V
{
	Mat34V()
	{
	}
	Mat34V(const Vec3V& c0, const Vec3V& c1, const Vec3V& c2, const Vec3V& c3) : col0(c0), col1(c1), col2(c2), col3(c3)
	{
	}
	Vec3V col0;
	Vec3V col1;
	Vec3V col2;
	Vec3V col3;
};

struct Mat43V
{
	Mat43V()
	{
	}
	Mat43V(const Vec4V& c0, const Vec4V& c1, const Vec4V& c2) : col0(c0), col1(c1), col2(c2)
	{
	}
	Vec4V col0;
	Vec4V col1;
	Vec4V col2;
};

struct Mat44V
{
	Mat44V()
	{
	}
	Mat44V(const Vec4V& c0, const Vec4V& c1, const Vec4V& c2, const Vec4V& c3) : col0(c0), col1(c1), col2(c2), col3(c3)
	{
	}
	Vec4V col0;
	Vec4V col1;
	Vec4V col2;
	Vec4V col3;
};

PX_ALIGN_PREFIX(16)
struct VecU32V
{
	PxU32 u32[4];
	PX_FORCE_INLINE VecU32V()
	{
	}
	PX_FORCE_INLINE VecU32V(PxU32 a, PxU32 b, PxU32 c, PxU32 d)
	{
		u32[0] = a;
		u32[1] = b;
		u32[2] = c;
		u32[3] = d;
	}
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct VecI32V
{
	PxI32 i32[4];
	PX_FORCE_INLINE VecI32V()
	{
	}
	PX_FORCE_INLINE VecI32V(PxI32 a, PxI32 b, PxI32 c, PxI32 d)
	{
		i32[0] = a;
		i32[1] = b;
		i32[2] = c;
		i32[3] = d;
	}
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct VecI16V
{
	PxI16 i16[8];
	PX_FORCE_INLINE VecI16V()
	{
	}
	PX_FORCE_INLINE VecI16V(PxI16 a, PxI16 b, PxI16 c, PxI16 d, PxI16 e, PxI16 f, PxI16 g, PxI16 h)
	{
		i16[0] = a;
		i16[1] = b;
		i16[2] = c;
		i16[3] = d;
		i16[4] = e;
		i16[5] = f;
		i16[6] = g;
		i16[7] = h;
	}
} PX_ALIGN_SUFFIX(16);

PX_ALIGN_PREFIX(16)
struct VecU16V
{
	union
	{
		PxU16 u16[8];
		PxI16 i16[8];
	};
	PX_FORCE_INLINE VecU16V()
	{
	}
	PX_FORCE_INLINE VecU16V(PxU16 a, PxU16 b, PxU16 c, PxU16 d, PxU16 e, PxU16 f, PxU16 g, PxU16 h)
	{
		u16[0] = a;
		u16[1] = b;
		u16[2] = c;
		u16[3] = d;
		u16[4] = e;
		u16[5] = f;
		u16[6] = g;
		u16[7] = h;
	}
} PX_ALIGN_SUFFIX(16);

#define FloatVArg FloatV &
#define Vec3VArg Vec3V &
#define Vec4VArg Vec4V &
#define BoolVArg BoolV &
#define VecU32VArg VecU32V &
#define VecI32VArg VecI32V &
#define VecU16VArg VecU16V &
#define VecI16VArg VecI16V &
#define QuatVArg QuatV &

#define VecCrossV Vec3V

typedef VecI32V VecShiftV;
#define VecShiftVArg VecShiftV &

#endif // PX_PHYSICS_COMMON_VECMATH_INLINE_SCALAR
