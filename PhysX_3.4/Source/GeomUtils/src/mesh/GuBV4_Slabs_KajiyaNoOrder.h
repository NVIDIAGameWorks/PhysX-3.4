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

#ifndef GU_BV4_SLABS_KAJIYA_NO_ORDER_H
#define GU_BV4_SLABS_KAJIYA_NO_ORDER_H

#include "GuBVConstants.h"

	// Kajiya, no sort
	template<int inflateT, class LeafTestT, class ParamsT>
	static Ps::IntBool BV4_ProcessStreamKajiyaNoOrder(const BVDataPacked* PX_RESTRICT node, PxU32 initData, ParamsT* PX_RESTRICT params)
	{
		const BVDataPacked* root = node;

		PxU32 nb=1;
		PxU32 stack[GU_BV4_STACK_SIZE];
		stack[0] = initData;

		///

		Vec4V fattenAABBsX, fattenAABBsY, fattenAABBsZ;
		if(inflateT)
		{
			Vec4V fattenAABBs4 = V4LoadU_Safe(&params->mOriginalExtents_Padded.x);
			fattenAABBs4 = V4Add(fattenAABBs4, epsInflateFloat4);	// US2385 - shapes are "closed" meaning exactly touching shapes should report overlap
			fattenAABBsX = V4SplatElement<0>(fattenAABBs4);
			fattenAABBsY = V4SplatElement<1>(fattenAABBs4);
			fattenAABBsZ = V4SplatElement<2>(fattenAABBs4);
		}

		///

		SLABS_INIT

#ifdef GU_BV4_QUANTIZED_TREE
		const Vec4V minCoeffV = V4LoadA_Safe(&params->mCenterOrMinCoeff_PaddedAligned.x);
		const Vec4V maxCoeffV = V4LoadA_Safe(&params->mExtentsOrMaxCoeff_PaddedAligned.x);
		const Vec4V minCoeffxV = V4SplatElement<0>(minCoeffV);
		const Vec4V minCoeffyV = V4SplatElement<1>(minCoeffV);
		const Vec4V minCoeffzV = V4SplatElement<2>(minCoeffV);
		const Vec4V maxCoeffxV = V4SplatElement<0>(maxCoeffV);
		const Vec4V maxCoeffyV = V4SplatElement<1>(maxCoeffV);
		const Vec4V maxCoeffzV = V4SplatElement<2>(maxCoeffV);
#endif

		do
		{
			const PxU32 childData = stack[--nb];
			node = root + getChildOffset(childData);

			const BVDataSwizzled* tn = reinterpret_cast<const BVDataSwizzled*>(node);

#ifdef GU_BV4_QUANTIZED_TREE
			Vec4V minx4a;
			Vec4V maxx4a;
			OPC_DEQ4(maxx4a, minx4a, mX, minCoeffxV, maxCoeffxV)

			Vec4V miny4a;
			Vec4V maxy4a;
			OPC_DEQ4(maxy4a, miny4a, mY, minCoeffyV, maxCoeffyV)

			Vec4V minz4a;
			Vec4V maxz4a;
			OPC_DEQ4(maxz4a, minz4a, mZ, minCoeffzV, maxCoeffzV)
#else
			Vec4V minx4a = V4LoadA(tn->mMinX);
			Vec4V miny4a = V4LoadA(tn->mMinY);
			Vec4V minz4a = V4LoadA(tn->mMinZ);

			Vec4V maxx4a = V4LoadA(tn->mMaxX);
			Vec4V maxy4a = V4LoadA(tn->mMaxY);
			Vec4V maxz4a = V4LoadA(tn->mMaxZ);
#endif
			if(inflateT)
			{
				maxx4a = V4Add(maxx4a, fattenAABBsX); maxy4a = V4Add(maxy4a, fattenAABBsY); maxz4a = V4Add(maxz4a, fattenAABBsZ);
				minx4a = V4Sub(minx4a, fattenAABBsX); miny4a = V4Sub(miny4a, fattenAABBsY); minz4a = V4Sub(minz4a, fattenAABBsZ);
			}

			SLABS_TEST

			SLABS_TEST2

#define DO_LEAF_TEST(x)														\
				{if(tn->isLeaf(x))											\
				{															\
					if(LeafTestT::doLeafTest(params, tn->getPrimitive(x)))	\
						return 1;											\
				}															\
				else														\
					stack[nb++] = tn->getChildData(x);}

			const PxU32 nodeType = getChildType(childData);
			if(!(code&8) && nodeType>1)
				DO_LEAF_TEST(3)

			if(!(code&4) && nodeType>0)
				DO_LEAF_TEST(2)

			if(!(code&2))
				DO_LEAF_TEST(1)

			if(!(code&1))
				DO_LEAF_TEST(0)

		}while(nb);

		return 0;
	}
#undef DO_LEAF_TEST

#endif // GU_BV4_SLABS_KAJIYA_NO_ORDER_H
