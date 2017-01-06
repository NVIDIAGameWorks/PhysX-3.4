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

#ifndef GU_TRIANGLE_CACHE_H
#define GU_TRIANGLE_CACHE_H
#include "PsHash.h"
#include "PsUtilities.h"

namespace physx
{
	namespace Gu
	{
		struct CachedEdge
		{
		protected:
			PxU32 mId0, mId1;
		public:
			CachedEdge(PxU32 i0, PxU32 i1)
			{
				mId0 = PxMin(i0, i1);
				mId1 = PxMax(i0, i1);
			}

			CachedEdge()
			{
			}

			PxU32 getId0() const { return mId0; }
			PxU32 getId1() const { return mId1; }

			bool operator == (const CachedEdge& other) const
			{
				return mId0 == other.mId0 && mId1 == other.mId1;
			}

			PxU32 getHashCode() const
			{
				return Ps::hash(mId0 << 16 | mId1);
			}
		};

		struct CachedVertex
		{
		private:
			PxU32 mId;
		public:
			CachedVertex(PxU32 id)
			{
				mId = id;
			}

			CachedVertex()
			{
			}

			PxU32 getId() const { return mId; }

			PxU32 getHashCode() const
			{
				return mId;
			}

			bool operator == (const CachedVertex& other) const 
			{
				return mId == other.mId;
			}
		};

		template <typename Elem, PxU32 MaxCount>
		struct CacheMap
		{
			PX_COMPILE_TIME_ASSERT(MaxCount < 0xFF);
			Elem mCache[MaxCount];
			PxU8 mNextInd[MaxCount];
			PxU8 mIndex[MaxCount];
			PxU32 mSize;

			CacheMap() : mSize(0)
			{
				for(PxU32 a = 0; a < MaxCount; ++a)
				{
					mIndex[a] = 0xFF;
				}
			}

			bool addData(const Elem& data)
			{
				if(mSize == MaxCount)
					return false;

				const PxU8 hash = PxU8(data.getHashCode() % MaxCount);

				PxU8 index = hash;
				PxU8 nextInd = mIndex[hash];
				while(nextInd != 0xFF)
				{
					index = nextInd;
					if(mCache[index] == data)
						return false;
					nextInd = mNextInd[nextInd];
				}

				if(mIndex[hash] == 0xFF)
				{
					mIndex[hash] = Ps::to8(mSize);
				}
				else
				{
					mNextInd[index] = Ps::to8(mSize);
				}
				mNextInd[mSize] = 0xFF;
				mCache[mSize++] = data;
				return true;
			}

			bool contains(const Elem& data) const
			{
				PxU32 hash = (data.getHashCode() % MaxCount);
				PxU8 index = mIndex[hash];

				while(index != 0xFF)
				{
					if(mCache[index] == data)
						return true;
					index = mNextInd[index];
				}
				return false;
			}

			const Elem* get(const Elem& data) const
			{
				PxU32 hash = (data.getHashCode() % MaxCount);
				PxU8 index = mIndex[hash];

				while(index != 0xFF)
				{
					if(mCache[index] == data)
						return &mCache[index];
					index = mNextInd[index];
				}
				return NULL;
			}
		};

		template <PxU32 MaxTriangles>
		struct TriangleCache
		{
			PxVec3 mVertices[3*MaxTriangles];
			PxU32 mIndices[3*MaxTriangles];
			PxU32 mTriangleIndex[MaxTriangles];
			PxU8 mEdgeFlags[MaxTriangles];
			PxU32 mNumTriangles;

			TriangleCache() : mNumTriangles(0)
			{
			}

			PX_FORCE_INLINE bool isEmpty() const { return mNumTriangles == 0; }
			PX_FORCE_INLINE bool isFull() const { return mNumTriangles == MaxTriangles; }
			PX_FORCE_INLINE void reset() { mNumTriangles = 0; }

			void addTriangle(const PxVec3* verts, const PxU32* indices, PxU32 triangleIndex, PxU8 edgeFlag)
			{
				PX_ASSERT(mNumTriangles < MaxTriangles);
				PxU32 triInd = mNumTriangles++;
				PxU32 triIndMul3 = triInd*3;
				mVertices[triIndMul3] = verts[0];
				mVertices[triIndMul3+1] = verts[1];
				mVertices[triIndMul3+2] = verts[2];
				mIndices[triIndMul3] = indices[0];
				mIndices[triIndMul3+1] = indices[1];
				mIndices[triIndMul3+2] = indices[2];
				mTriangleIndex[triInd] = triangleIndex;
				mEdgeFlags[triInd] = edgeFlag;
			}
		};
	}
}

#endif

