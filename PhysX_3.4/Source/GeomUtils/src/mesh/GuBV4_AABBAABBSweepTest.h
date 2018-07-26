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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_BV4_AABB_AABB_SWEEP_TEST_H
#define GU_BV4_AABB_AABB_SWEEP_TEST_H

#ifndef GU_BV4_USE_SLABS
#if PX_INTEL_FAMILY
	PX_FORCE_INLINE Ps::IntBool BV4_SegmentAABBOverlap(const PxVec3& center, const PxVec3& extents, const PxVec3& extents2, const RayParams* PX_RESTRICT params)
	{
		const PxU32 maskI = 0x7fffffff;
		const Vec4V fdirV = V4LoadA_Safe(&params->mFDir_PaddedAligned.x);
		const Vec4V extentsV = V4Add(V4LoadU(&extents.x), V4LoadU(&extents2.x));
		const Vec4V DV = V4Sub(V4LoadA_Safe(&params->mData2_PaddedAligned.x), V4LoadU(&center.x));
		__m128 absDV = _mm_and_ps(DV, _mm_load1_ps((float*)&maskI));
		absDV = _mm_cmpgt_ps(absDV, V4Add(extentsV, fdirV));
		const PxU32 test = (PxU32)_mm_movemask_ps(absDV);
		if(test&7)
			return 0;

		if(1)
		{
			const Vec4V dataZYX_V = V4LoadA_Safe(&params->mData_PaddedAligned.x);
			const __m128 dataXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(dataZYX_V), _MM_SHUFFLE(3,0,2,1)));
			const __m128 DXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(DV), _MM_SHUFFLE(3,0,2,1)));
			const Vec4V fV = V4Sub(V4Mul(dataZYX_V, DXZY_V), V4Mul(dataXZY_V, DV));

			const Vec4V fdirZYX_V = V4LoadA_Safe(&params->mFDir_PaddedAligned.x);
			const __m128 fdirXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(fdirZYX_V), _MM_SHUFFLE(3,0,2,1)));
			const __m128 extentsXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(extentsV), _MM_SHUFFLE(3,0,2,1)));
			// PT: TODO: use V4MulAdd here (TA34704)
			const Vec4V fg = V4Add(V4Mul(extentsV, fdirXZY_V), V4Mul(extentsXZY_V, fdirZYX_V));

			__m128 absfV = _mm_and_ps(fV, _mm_load1_ps((float*)&maskI));
			absfV = _mm_cmpgt_ps(absfV, fg);
			const PxU32 test2 = (PxU32)_mm_movemask_ps(absfV);
			if(test2&7)
				return 0;
			return 1;
		}
	}

#ifdef GU_BV4_QUANTIZED_TREE
	template<class T>
	PX_FORCE_INLINE Ps::IntBool BV4_SegmentAABBOverlap(const T* PX_RESTRICT node, const PxVec3& extents2, const RayParams* PX_RESTRICT params)
	{
		const __m128i testV = _mm_load_si128((__m128i*)node->mAABB.mData);
		const __m128i qextentsV = _mm_and_si128(testV, _mm_set1_epi32(0x0000ffff));
		const __m128i qcenterV = _mm_srai_epi32(testV, 16);
		const Vec4V centerV0 = V4Mul(_mm_cvtepi32_ps(qcenterV), V4LoadA_Safe(&params->mCenterOrMinCoeff_PaddedAligned.x));
		const Vec4V extentsV0 = V4Mul(_mm_cvtepi32_ps(qextentsV), V4LoadA_Safe(&params->mExtentsOrMaxCoeff_PaddedAligned.x));

		const PxU32 maskI = 0x7fffffff;
		const Vec4V fdirV = V4LoadA_Safe(&params->mFDir_PaddedAligned.x);
		const Vec4V extentsV = V4Add(extentsV0, V4LoadU(&extents2.x));
		const Vec4V DV = V4Sub(V4LoadA_Safe(&params->mData2_PaddedAligned.x), centerV0);
		__m128 absDV = _mm_and_ps(DV, _mm_load1_ps((float*)&maskI));
		absDV = _mm_cmpgt_ps(absDV, V4Add(extentsV, fdirV));
		const PxU32 test = (PxU32)_mm_movemask_ps(absDV);
		if(test&7)
			return 0;

		if(1)
		{
			const Vec4V dataZYX_V = V4LoadA_Safe(&params->mData_PaddedAligned.x);
			const __m128 dataXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(dataZYX_V), _MM_SHUFFLE(3,0,2,1)));
			const __m128 DXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(DV), _MM_SHUFFLE(3,0,2,1)));
			const Vec4V fV = V4Sub(V4Mul(dataZYX_V, DXZY_V), V4Mul(dataXZY_V, DV));

			const Vec4V fdirZYX_V = V4LoadA_Safe(&params->mFDir_PaddedAligned.x);
			const __m128 fdirXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(fdirZYX_V), _MM_SHUFFLE(3,0,2,1)));
			const __m128 extentsXZY_V = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(extentsV), _MM_SHUFFLE(3,0,2,1)));
			// PT: TODO: use V4MulAdd here (TA34704)
			const Vec4V fg = V4Add(V4Mul(extentsV, fdirXZY_V), V4Mul(extentsXZY_V, fdirZYX_V));

			__m128 absfV = _mm_and_ps(fV, _mm_load1_ps((float*)&maskI));
			absfV = _mm_cmpgt_ps(absfV, fg);
			const PxU32 test2 = (PxU32)_mm_movemask_ps(absfV);
			if(test2&7)
				return 0;
			return 1;
		}
	}
#endif
#endif
#endif

#endif // GU_BV4_AABB_AABB_SWEEP_TEST_H
