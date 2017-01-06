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

#ifndef PSFOUNDATION_PSVECMATHSSE_H
#define PSFOUNDATION_PSVECMATHSSE_H

namespace
{
	const PX_ALIGN(16, PxF32) minus1w[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
}

PX_FORCE_INLINE void QuatGetMat33V(const QuatVArg q, Vec3V& column0, Vec3V& column1, Vec3V& column2)
{
    const __m128 q2 = V4Add(q, q);
    const __m128 qw2 = V4MulAdd(q2, V4GetW(q), _mm_load_ps(minus1w));			// (2wx, 2wy, 2wz, 2ww-1)
    const __m128 nw2 = Vec3V_From_Vec4V(V4Neg(qw2));							// (-2wx, -2wy, -2wz, 0)
    const __m128 v = Vec3V_From_Vec4V(q);

    const __m128 a0 = _mm_shuffle_ps(qw2, nw2, _MM_SHUFFLE(3, 1, 2, 3));		// (2ww-1, 2wz, -2wy, 0)
    column0 = V4MulAdd(v, V4GetX(q2), a0);

    const __m128 a1 = _mm_shuffle_ps(qw2, nw2, _MM_SHUFFLE(3, 2, 0, 3));		// (2ww-1, 2wx, -2wz, 0)
    column1 = V4MulAdd(v, V4GetY(q2), _mm_shuffle_ps(a1, a1, _MM_SHUFFLE(3, 1, 0, 2)));

    const __m128 a2 = _mm_shuffle_ps(qw2, nw2, _MM_SHUFFLE(3, 0, 1, 3));		// (2ww-1, 2wy, -2wx, 0)
    column2 = V4MulAdd(v, V4GetZ(q2), _mm_shuffle_ps(a2, a2, _MM_SHUFFLE(3, 0, 2, 1)));
}

#endif // PSFOUNDATION_PSVECMATHSSE_H

