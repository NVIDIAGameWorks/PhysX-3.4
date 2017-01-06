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

#ifndef GU_BV4_SLABS_SWIZZLED_NO_ORDER_H
#define GU_BV4_SLABS_SWIZZLED_NO_ORDER_H

	// Generic, no sort
	template<class LeafTestT, class ParamsT>
	static Ps::IntBool BV4_ProcessStreamSwizzledNoOrder(const BVDataPacked* PX_RESTRICT node, PxU32 initData, ParamsT* PX_RESTRICT params)
	{
		const BVDataPacked* root = node;

		PxU32 nb=1;
		PxU32 stack[GU_BV4_STACK_SIZE];
		stack[0] = initData;

		do
		{
			const PxU32 childData = stack[--nb];
			node = root + getChildOffset(childData);

			const BVDataSwizzled* tn = reinterpret_cast<const BVDataSwizzled*>(node);

			const PxU32 nodeType = getChildType(childData);

			if(nodeType>1 && BV4_ProcessNodeNoOrder_Swizzled<LeafTestT, 3>(stack, nb, tn, params))
				return 1;
			if(nodeType>0 && BV4_ProcessNodeNoOrder_Swizzled<LeafTestT, 2>(stack, nb, tn, params))
				return 1;
			if(BV4_ProcessNodeNoOrder_Swizzled<LeafTestT, 1>(stack, nb, tn, params))
				return 1;
			if(BV4_ProcessNodeNoOrder_Swizzled<LeafTestT, 0>(stack, nb, tn, params))
				return 1;

		}while(nb);

		return 0;
	}

#endif // GU_BV4_SLABS_SWIZZLED_NO_ORDER_H
