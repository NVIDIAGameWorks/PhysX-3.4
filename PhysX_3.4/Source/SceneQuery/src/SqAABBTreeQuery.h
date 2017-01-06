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

#ifndef SQ_AABBTREEQUERY_H
#define SQ_AABBTREEQUERY_H

#include "SqAABBTree.h"
#include "SqPrunerTestsSIMD.h"

namespace physx
{
	namespace Sq
	{
		#define RAW_TRAVERSAL_STACK_SIZE 256

		//////////////////////////////////////////////////////////////////////////

		static PX_FORCE_INLINE void getBoundsTimesTwo(Vec4V& center, Vec4V& extents, const PxBounds3* boxes, PoolIndex poolIndex)
		{
			const PxBounds3* objectBounds = boxes + poolIndex;

			const Vec4V minV = V4LoadU(&objectBounds->minimum.x);
			const Vec4V maxV = V4LoadU(&objectBounds->maximum.x);

			center = V4Add(maxV, minV);
			extents = V4Sub(maxV, minV);
		}

		//////////////////////////////////////////////////////////////////////////

		template<typename Test>
		class AABBTreeOverlap
		{
		public:
			bool operator()(const PrunerPayload* objects, const PxBounds3* boxes, const AABBTree& tree, const Test& test, PrunerCallback& visitor)
			{
				using namespace Cm;

				const AABBTreeRuntimeNode* stack[RAW_TRAVERSAL_STACK_SIZE];
				const AABBTreeRuntimeNode* const nodeBase = tree.getNodes();
				stack[0] = nodeBase;
				PxU32 stackIndex = 1;

				while (stackIndex > 0)
				{
					const AABBTreeRuntimeNode* node = stack[--stackIndex];
					Vec3V center, extents;
					node->getAABBCenterExtentsV(&center, &extents);
					while (test(center, extents))
					{
						if (node->isLeaf())
						{
							PxU32 nbPrims = node->getNbPrimitives();
							const bool doBoxTest = nbPrims > 1;
							const PxU32* prims = node->getPrimitives(tree.getIndices());
							while (nbPrims--)
							{
								const PxU32* prunableIndex = prims;
								prims++;

								const PoolIndex poolIndex = *prunableIndex;
								if (doBoxTest)
								{
									Vec4V center2, extents2;
									getBoundsTimesTwo(center2, extents2, boxes, poolIndex);

									const float half = 0.5f;
									const FloatV halfV = FLoad(half);

									const Vec4V extents_ = V4Scale(extents2, halfV);
									const Vec4V center_ = V4Scale(center2, halfV);

									if (!test(Vec3V_From_Vec4V(center_), Vec3V_From_Vec4V(extents_)))
										continue;
								}

								PxReal unusedDistance;
								if (!visitor.invoke(unusedDistance, objects[poolIndex]))
									return false;
							}
							break;
						}

						const AABBTreeRuntimeNode* children = node->getPos(nodeBase);

						node = children;
						stack[stackIndex++] = children + 1;
						PX_ASSERT(stackIndex < RAW_TRAVERSAL_STACK_SIZE);
						node->getAABBCenterExtentsV(&center, &extents);
					}
				}
				return true;
			}
		};

		//////////////////////////////////////////////////////////////////////////

		template <bool tInflate> // use inflate=true for sweeps, inflate=false for raycasts
		static PX_FORCE_INLINE bool doLeafTest(const AABBTreeRuntimeNode* node, Gu::RayAABBTest& test, PxReal& md, PxReal oldMaxDist,
			const PrunerPayload* objects, const PxBounds3* boxes, const AABBTree& tree,
			PxReal& maxDist, PrunerCallback& pcb)
		{
			PxU32 nbPrims = node->getNbPrimitives();
			const bool doBoxTest = nbPrims > 1;
			const PxU32* prims = node->getPrimitives(tree.getIndices());
			while (nbPrims--)
			{
				const PxU32* prunableIndex = prims;
				prims++;

				const PoolIndex poolIndex = *prunableIndex;
				if (doBoxTest)
				{
					Vec4V center_, extents_;
					getBoundsTimesTwo(center_, extents_, boxes, poolIndex);

					if (!test.check<tInflate>(Vec3V_From_Vec4V(center_), Vec3V_From_Vec4V(extents_)))
						continue;
				}

				if (!pcb.invoke(md, objects[poolIndex]))
					return false;

				if (md < oldMaxDist)
				{
					maxDist = md;
					test.setDistance(md);
				}
			}
			return true;
		}

		//////////////////////////////////////////////////////////////////////////

		template <bool tInflate> // use inflate=true for sweeps, inflate=false for raycasts
		class AABBTreeRaycast
		{
		public:
			bool operator()(
				const PrunerPayload* objects, const PxBounds3* boxes, const AABBTree& tree,
				const PxVec3& origin, const PxVec3& unitDir, PxReal& maxDist, const PxVec3& inflation,
				PrunerCallback& pcb)
			{
				using namespace Cm;

				// PT: we will pass center*2 and extents*2 to the ray-box code, to save some work per-box
				// So we initialize the test with values multiplied by 2 as well, to get correct results
				Gu::RayAABBTest test(origin*2.0f, unitDir*2.0f, maxDist, inflation*2.0f);

				const AABBTreeRuntimeNode* stack[RAW_TRAVERSAL_STACK_SIZE]; // stack always contains PPU addresses
				const AABBTreeRuntimeNode* const nodeBase = tree.getNodes();
				stack[0] = nodeBase;
				PxU32 stackIndex = 1;

				PxReal oldMaxDist;
				while (stackIndex--)
				{
					const AABBTreeRuntimeNode* node = stack[stackIndex];
					Vec3V center, extents;
					node->getAABBCenterExtentsV2(&center, &extents);
					if (test.check<tInflate>(center, extents))	// TODO: try timestamp ray shortening to skip this
					{
						PxReal md = maxDist; // has to be before the goto below to avoid compile error
						while (!node->isLeaf())
						{
							const AABBTreeRuntimeNode* children = node->getPos(nodeBase);

							Vec3V c0, e0;
							children[0].getAABBCenterExtentsV2(&c0, &e0);
							const PxU32 b0 = test.check<tInflate>(c0, e0);

							Vec3V c1, e1;
							children[1].getAABBCenterExtentsV2(&c1, &e1);
							const PxU32 b1 = test.check<tInflate>(c1, e1);

							if (b0 && b1)	// if both intersect, push the one with the further center on the stack for later
							{
								// & 1 because FAllGrtr behavior differs across platforms
								const PxU32 bit = FAllGrtr(V3Dot(V3Sub(c1, c0), test.mDir), FZero()) & 1;
								stack[stackIndex++] = children + bit;
								node = children + (1 - bit);
								PX_ASSERT(stackIndex < RAW_TRAVERSAL_STACK_SIZE);
							}
							else if (b0)
								node = children;
							else if (b1)
								node = children + 1;
							else
								goto skip_leaf_code;
						}

						oldMaxDist = maxDist; // we copy since maxDist can be updated in the callback and md<maxDist test below can fail

						if (!doLeafTest<tInflate>(node, test, md, oldMaxDist,
							objects, boxes, tree,
							maxDist,
							pcb))
							return false;
					skip_leaf_code:;
					}
				}
				return true;
			}
		};
	}
}

#endif   // SQ_AABBTREEQUERY_H
