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

#ifndef GU_BV4_SLABS_SWIZZLED_ORDERED_H
#define GU_BV4_SLABS_SWIZZLED_ORDERED_H

	// Generic + PNS
	template<class LeafTestT, class ParamsT>
	static void BV4_ProcessStreamSwizzledOrdered(const BVDataPacked* PX_RESTRICT node, PxU32 initData, ParamsT* PX_RESTRICT params)
	{
		const BVDataPacked* root = node;

		PxU32 nb=1;
		PxU32 stack[GU_BV4_STACK_SIZE];
		stack[0] = initData;

		const PxU32* tmp = reinterpret_cast<const PxU32*>(&params->mLocalDir_Padded);
		const PxU32 X = tmp[0]>>31;
		const PxU32 Y = tmp[1]>>31;
		const PxU32 Z = tmp[2]>>31;
//		const PxU32 X = PX_IR(params->mLocalDir_Padded.x)>>31;
//		const PxU32 Y = PX_IR(params->mLocalDir_Padded.y)>>31;
//		const PxU32 Z = PX_IR(params->mLocalDir_Padded.z)>>31;
		const PxU32 bitIndex = 3+(Z|(Y<<1)|(X<<2));
		const PxU32 dirMask = 1u<<bitIndex;

		do
		{
			const PxU32 childData = stack[--nb];
			node = root + getChildOffset(childData);
			const PxU32 nodeType = getChildType(childData);

			const BVDataSwizzled* tn = reinterpret_cast<const BVDataSwizzled*>(node);

			PxU32 code2 = 0;
			BV4_ProcessNodeOrdered2_Swizzled<LeafTestT, 0>(code2, tn, params);
			BV4_ProcessNodeOrdered2_Swizzled<LeafTestT, 1>(code2, tn, params);
			if(nodeType>0)
				BV4_ProcessNodeOrdered2_Swizzled<LeafTestT, 2>(code2, tn, params);
			if(nodeType>1)
				BV4_ProcessNodeOrdered2_Swizzled<LeafTestT, 3>(code2, tn, params);

			SLABS_PNS

		}while(nb);
	}

#endif // GU_BV4_SLABS_SWIZZLED_ORDERED_H
