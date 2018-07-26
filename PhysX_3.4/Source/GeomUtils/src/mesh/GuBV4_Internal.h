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

#ifndef GU_BV4_INTERNAL_H
#define GU_BV4_INTERNAL_H

#include "CmPhysXCommon.h"
#include "PsFPU.h"

	static	PX_FORCE_INLINE	PxU32	getChildOffset(PxU32 data)	{ return data>>GU_BV4_CHILD_OFFSET_SHIFT_COUNT;	}
	static	PX_FORCE_INLINE	PxU32	getChildType(PxU32 data)	{ return (data>>1)&3;							}

	// PT: the general structure is that there is a root "process stream" function which is the entry point for the query.
	// It then calls "process node" functions for each traversed node, except for the Slabs-based raycast versions that deal
	// with 4 nodes at a time within the "process stream" function itself. When a leaf is found, "doLeafTest" functors
	// passed to the "process stream" entry point are called.
#ifdef GU_BV4_USE_SLABS
	#define	processStreamNoOrder			BV4_ProcessStreamSwizzledNoOrder
	#define	processStreamOrdered			BV4_ProcessStreamSwizzledOrdered
	#define	processStreamRayNoOrder(a, b)	BV4_ProcessStreamKajiyaNoOrder<a, b>
	#define	processStreamRayOrdered(a, b)	BV4_ProcessStreamKajiyaOrdered<a, b>
#else
	#define	processStreamNoOrder			BV4_ProcessStreamNoOrder
	#define	processStreamOrdered			BV4_ProcessStreamOrdered2
	#define	processStreamRayNoOrder(a, b)	BV4_ProcessStreamNoOrder<b>
	#define	processStreamRayOrdered(a, b)	BV4_ProcessStreamOrdered2<b>
#endif

#ifndef GU_BV4_USE_SLABS
#ifdef	GU_BV4_PRECOMPUTED_NODE_SORT
	// PT: see http://www.codercorner.com/blog/?p=734

	// PT: TODO: refactor with dup in bucket pruner (TA34704)
	PX_FORCE_INLINE PxU32 computeDirMask(const PxVec3& dir)
	{
		// XYZ
		// ---
		// --+
		// -+-
		// -++
		// +--
		// +-+
		// ++-
		// +++

		const PxU32 X = PX_IR(dir.x)>>31;
		const PxU32 Y = PX_IR(dir.y)>>31;
		const PxU32 Z = PX_IR(dir.z)>>31;
		const PxU32 bitIndex = Z|(Y<<1)|(X<<2);
		return 1u<<bitIndex;
	}

	// 0    0    0      PP PN NP NN		0 1 2 3
	// 0    0    1      PP PN NN NP		0 1 3 2
	// 0    1    0      PN PP NP NN		1 0 2 3
	// 0    1    1      PN PP NN NP		1 0 3 2
	// 1    0    0      NP NN PP PN		2 3 0 1
	// 1    0    1      NN NP PP PN		3 2	0 1
	// 1    1    0      NP NN PN PP		2 3	1 0
	// 1    1    1      NN NP PN PP		3 2	1 0
	static const PxU8 order[] = {
		0,1,2,3,
		0,1,3,2,
		1,0,2,3,
		1,0,3,2,
		2,3,0,1,
		3,2,0,1,
		2,3,1,0,
		3,2,1,0,
	};

	PX_FORCE_INLINE PxU32 decodePNS(const BVDataPacked* PX_RESTRICT node, const PxU32 dirMask)
	{
		const PxU32 bit0 = (node[0].decodePNSNoShift() & dirMask) ? 1u : 0;
		const PxU32 bit1 = (node[1].decodePNSNoShift() & dirMask) ? 1u : 0;
		const PxU32 bit2 = (node[2].decodePNSNoShift() & dirMask) ? 1u : 0;		//### potentially reads past the end of the stream here!
		return bit2|(bit1<<1)|(bit0<<2);
	}
#endif	// GU_BV4_PRECOMPUTED_NODE_SORT

	#define PNS_BLOCK(i, a, b, c, d)										\
		case i:																\
		{																	\
			if(code & (1<<a))	{ stack[nb++] = node[a].getChildData();	}	\
			if(code & (1<<b))	{ stack[nb++] = node[b].getChildData();	}	\
			if(code & (1<<c))	{ stack[nb++] = node[c].getChildData();	}	\
			if(code & (1<<d))	{ stack[nb++] = node[d].getChildData();	}	\
		}break;

	#define PNS_BLOCK1(i, a, b, c, d)										\
		case i:																\
		{																	\
		stack[nb] = node[a].getChildData();	nb += (code & (1<<a))?1:0;		\
		stack[nb] = node[b].getChildData();	nb += (code & (1<<b))?1:0;		\
		stack[nb] = node[c].getChildData();	nb += (code & (1<<c))?1:0;		\
		stack[nb] = node[d].getChildData();	nb += (code & (1<<d))?1:0;		\
		}break;

	#define PNS_BLOCK2(a, b, c, d)	{										\
		if(code & (1<<a))	{ stack[nb++] = node[a].getChildData();	}		\
		if(code & (1<<b))	{ stack[nb++] = node[b].getChildData();	}		\
		if(code & (1<<c))	{ stack[nb++] = node[c].getChildData();	}		\
		if(code & (1<<d))	{ stack[nb++] = node[d].getChildData();	}	}	\

#if PX_INTEL_FAMILY
	template<class LeafTestT, class ParamsT>
	static Ps::IntBool BV4_ProcessStreamNoOrder(const BVDataPacked* PX_RESTRICT node, PxU32 initData, ParamsT* PX_RESTRICT params)
	{
		const BVDataPacked* root = node;

		PxU32 nb=1;
		PxU32 stack[GU_BV4_STACK_SIZE];
		stack[0] = initData;

		do
		{
			const PxU32 childData = stack[--nb];
			node = root + getChildOffset(childData);
			const PxU32 nodeType = getChildType(childData);

			if(nodeType>1 && BV4_ProcessNodeNoOrder<LeafTestT, 3>(stack, nb, node, params))
				return 1;
			if(nodeType>0 && BV4_ProcessNodeNoOrder<LeafTestT, 2>(stack, nb, node, params))
				return 1;
			if(BV4_ProcessNodeNoOrder<LeafTestT, 1>(stack, nb, node, params))
				return 1;
			if(BV4_ProcessNodeNoOrder<LeafTestT, 0>(stack, nb, node, params))
				return 1;

		}while(nb);

		return 0;
	}

	template<class LeafTestT, class ParamsT>
	static void BV4_ProcessStreamOrdered(const BVDataPacked* PX_RESTRICT node, PxU32 initData, ParamsT* PX_RESTRICT params)
	{
		const BVDataPacked* root = node;

		PxU32 nb=1;
		PxU32 stack[GU_BV4_STACK_SIZE];
		stack[0] = initData;

		const PxU32 dirMask = computeDirMask(params->mLocalDir)<<3;

		do
		{
			const PxU32 childData = stack[--nb];
			node = root + getChildOffset(childData);

			const PxU8* PX_RESTRICT ord = order + decodePNS(node, dirMask)*4;
			const PxU32 limit = 2 + getChildType(childData);

			BV4_ProcessNodeOrdered<LeafTestT>(stack, nb, node, params, ord[0], limit);
			BV4_ProcessNodeOrdered<LeafTestT>(stack, nb, node, params, ord[1], limit);
			BV4_ProcessNodeOrdered<LeafTestT>(stack, nb, node, params, ord[2], limit);
			BV4_ProcessNodeOrdered<LeafTestT>(stack, nb, node, params, ord[3], limit);
		}while(Nb);
	}

	// Alternative, experimental version using PNS
	template<class LeafTestT, class ParamsT>
	static void BV4_ProcessStreamOrdered2(const BVDataPacked* PX_RESTRICT node, PxU32 initData, ParamsT* PX_RESTRICT params)
	{
		const BVDataPacked* root = node;

		PxU32 nb=1;
		PxU32 stack[GU_BV4_STACK_SIZE];
		stack[0] = initData;

		const PxU32 X = PX_IR(params->mLocalDir_Padded.x)>>31;
		const PxU32 Y = PX_IR(params->mLocalDir_Padded.y)>>31;
		const PxU32 Z = PX_IR(params->mLocalDir_Padded.z)>>31;
		const PxU32 bitIndex = 3+(Z|(Y<<1)|(X<<2));
		const PxU32 dirMask = 1u<<bitIndex;

		do
		{
			const PxU32 childData = stack[--nb];
			node = root + getChildOffset(childData);
			const PxU32 nodeType = getChildType(childData);

			PxU32 code = 0;
			BV4_ProcessNodeOrdered2<LeafTestT, 0>(code, node, params);
			BV4_ProcessNodeOrdered2<LeafTestT, 1>(code, node, params);
			if(nodeType>0)
				BV4_ProcessNodeOrdered2<LeafTestT, 2>(code, node, params);
			if(nodeType>1)
				BV4_ProcessNodeOrdered2<LeafTestT, 3>(code, node, params);

			if(code)
			{
				// PT: TODO: check which implementation is best on each platform (TA34704)
#define FOURTH_TEST	// Version avoids computing the PNS index, and also avoids all non-constant shifts. Full of branches though. Fastest on Win32.
#ifdef FOURTH_TEST
				{
					if(node[0].decodePNSNoShift() & dirMask)	//  Bit2
					{
						if(node[1].decodePNSNoShift() & dirMask)	// Bit1
						{
							if(node[2].decodePNSNoShift() & dirMask)	// Bit0
								PNS_BLOCK2(3,2,1,0)	// 7
							else
								PNS_BLOCK2(2,3,1,0)	// 6
						}
						else
						{
							if(node[2].decodePNSNoShift() & dirMask)	// Bit0
								PNS_BLOCK2(3,2,0,1)	// 5
							else
								PNS_BLOCK2(2,3,0,1)	// 4
						}
					}
					else
					{
						if(node[1].decodePNSNoShift() & dirMask)	// Bit1
						{
							if(node[2].decodePNSNoShift() & dirMask)	// Bit0
								PNS_BLOCK2(1,0,3,2)	// 3
							else
								PNS_BLOCK2(1,0,2,3)	// 2
						}
						else
						{
							if(node[2].decodePNSNoShift() & dirMask)	// Bit0
								PNS_BLOCK2(0,1,3,2)	// 1
							else
								PNS_BLOCK2(0,1,2,3)	// 0
						}
					}
				}
#endif
			}
		}while(nb);
	}
#endif	// PX_INTEL_FAMILY
#endif	// GU_BV4_USE_SLABS

#endif // GU_BV4_INTERNAL_H
