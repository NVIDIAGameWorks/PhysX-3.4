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

#ifndef GU_BV4_H
#define GU_BV4_H

#include "foundation/PxBounds3.h"
#include "PxSerialFramework.h"
#include "PsUserAllocated.h"
#include "GuBV4Settings.h"

#define V4LoadU_Safe	V4LoadU
#define V4LoadA_Safe	V4LoadA
#define V4StoreA_Safe	V4StoreA
#define V4StoreU_Safe	V4StoreU

namespace physx
{
namespace Gu
{

	struct VertexPointers
	{
		const PxVec3*	Vertex[3];
	};

	class IndTri32 : public physx::shdfnd::UserAllocated
	{
		public:
		PX_FORCE_INLINE				IndTri32()								{}
		PX_FORCE_INLINE				IndTri32(PxU32 r0, PxU32 r1, PxU32 r2)	{ mRef[0]=r0; mRef[1]=r1; mRef[2]=r2; }
		PX_FORCE_INLINE				IndTri32(const IndTri32& triangle)
									{
										mRef[0] = triangle.mRef[0];
										mRef[1] = triangle.mRef[1];
										mRef[2] = triangle.mRef[2];
									}
		PX_FORCE_INLINE				~IndTri32()								{}
						PxU32		mRef[3];
	};
	PX_COMPILE_TIME_ASSERT(sizeof(IndTri32)==12);

	class IndTri16 : public physx::shdfnd::UserAllocated
	{
		public:
		PX_FORCE_INLINE				IndTri16()								{}
		PX_FORCE_INLINE				IndTri16(PxU16 r0, PxU16 r1, PxU16 r2)	{ mRef[0]=r0; mRef[1]=r1; mRef[2]=r2; }
		PX_FORCE_INLINE				IndTri16(const IndTri16& triangle)
									{
										mRef[0] = triangle.mRef[0];
										mRef[1] = triangle.mRef[1];
										mRef[2] = triangle.mRef[2];
									}
		PX_FORCE_INLINE				~IndTri16()								{}
						PxU16		mRef[3];
	};
	PX_COMPILE_TIME_ASSERT(sizeof(IndTri16)==6);

	PX_FORCE_INLINE void getVertexReferences(PxU32& vref0, PxU32& vref1, PxU32& vref2, PxU32 index, const IndTri32* T32, const IndTri16* T16)
	{
		if(T32)
		{
			const IndTri32* PX_RESTRICT tri = T32 + index;
			vref0 = tri->mRef[0];
			vref1 = tri->mRef[1];
			vref2 = tri->mRef[2];
		}
		else
		{
			const IndTri16* PX_RESTRICT tri = T16 + index;
			vref0 = tri->mRef[0];
			vref1 = tri->mRef[1];
			vref2 = tri->mRef[2];
		}
	}

	class SourceMesh : public physx::shdfnd::UserAllocated
	{
		public:
		PX_PHYSX_COMMON_API				SourceMesh();
		PX_PHYSX_COMMON_API				~SourceMesh();
		// PX_SERIALIZATION
						SourceMesh(const PxEMPTY)	{}
		static			void			getBinaryMetaData(PxOutputStream& stream);
		//~PX_SERIALIZATION

						void			reset();
						void			operator = (SourceMesh& v);

						PxU32			mNbVerts;
						const PxVec3*	mVerts;
						PxU32			mNbTris;
						IndTri32*		mTriangles32;
						IndTri16*		mTriangles16;

		PX_FORCE_INLINE	PxU32			getNbTriangles()	const	{ return mNbTris;		}
		PX_FORCE_INLINE	PxU32			getNbVertices()		const	{ return mNbVerts;		}
		PX_FORCE_INLINE	const IndTri32*	getTris32()			const	{ return mTriangles32;	}
		PX_FORCE_INLINE	const IndTri16*	getTris16()			const	{ return mTriangles16;	}
		PX_FORCE_INLINE	const PxVec3*	getVerts()			const	{ return mVerts;		}

		PX_FORCE_INLINE	void			setNbTriangles(PxU32 nb)	{ mNbTris = nb;			}
		PX_FORCE_INLINE	void			setNbVertices(PxU32 nb)		{ mNbVerts = nb;		}

		PX_FORCE_INLINE	void			setPointers(IndTri32* tris32, IndTri16* tris16, const PxVec3* verts)
										{
											mTriangles32	= tris32;
											mTriangles16	= tris16;
											mVerts			= verts;
										}

		PX_FORCE_INLINE	void			initRemap()			{ mRemap = NULL;				}
		PX_FORCE_INLINE	const PxU32*	getRemap()	const	{ return mRemap;				}
		PX_FORCE_INLINE	void			releaseRemap()		{ PX_FREE_AND_RESET(mRemap);	}
						void			remapTopology(const PxU32* order);

						bool			isValid()		const;

		PX_FORCE_INLINE	void			getTriangle(VertexPointers& vp, PxU32 index)	const
										{
											PxU32 VRef0, VRef1, VRef2;
											getVertexReferences(VRef0, VRef1, VRef2, index, mTriangles32, mTriangles16);
											vp.Vertex[0] = mVerts + VRef0;
											vp.Vertex[1] = mVerts + VRef1;
											vp.Vertex[2] = mVerts + VRef2;
										}
		private:
						PxU32*			mRemap;
	};

	struct LocalBounds
	{
										LocalBounds() : mCenter(PxVec3(0.0f)), mExtentsMagnitude(0.0f)	{}

						PxVec3			mCenter;
						float			mExtentsMagnitude;

		PX_FORCE_INLINE	void			init(const PxBounds3& bounds)
										{
											mCenter = bounds.getCenter();
											// PT: TODO: compute mag first, then multiplies by 0.5f (TA34704)
											mExtentsMagnitude = bounds.getExtents().magnitude();
										}
	};

#ifdef GU_BV4_QUANTIZED_TREE
	class QuantizedAABB
	{
		public:

		struct Data
		{
				PxU16		mExtents;	//!< Quantized extents
				PxI16		mCenter;	//!< Quantized center
		};
				Data		mData[3];
	};
	PX_COMPILE_TIME_ASSERT(sizeof(QuantizedAABB)==12);
#endif

	/////

	#define GU_BV4_CHILD_OFFSET_SHIFT_COUNT	11

	struct BVDataPacked : public physx::shdfnd::UserAllocated
	{
#ifdef GU_BV4_QUANTIZED_TREE
						QuantizedAABB	mAABB;
#else
						CenterExtents	mAABB;
#endif
						PxU32			mData;

		PX_FORCE_INLINE	PxU32			isLeaf()			const	{ return mData&1;								}
		PX_FORCE_INLINE	PxU32			getPrimitive()		const	{ return mData>>1;								}
		PX_FORCE_INLINE	PxU32			getChildOffset()	const	{ return mData>>GU_BV4_CHILD_OFFSET_SHIFT_COUNT;}
		PX_FORCE_INLINE	PxU32			getChildType()		const	{ return (mData>>1)&3;							}
		PX_FORCE_INLINE	PxU32			getChildData()		const	{ return mData;									}

		PX_FORCE_INLINE	void			encodePNS(PxU32 code)
										{
											PX_ASSERT(code<256);
											mData |= code<<3;
										}
		PX_FORCE_INLINE	PxU32			decodePNSNoShift()	const	{ return mData;			}
	};

	// PT: TODO: align class to 16? (TA34704)
	class BV4Tree : public physx::shdfnd::UserAllocated
	{
		public:
		// PX_SERIALIZATION
										BV4Tree(const PxEMPTY);
						void			exportExtraData(PxSerializationContext&);
						void			importExtraData(PxDeserializationContext& context);
		static			void			getBinaryMetaData(PxOutputStream& stream);
		//~PX_SERIALIZATION

		PX_PHYSX_COMMON_API				BV4Tree();
		PX_PHYSX_COMMON_API				BV4Tree(SourceMesh* meshInterface, const PxBounds3& localBounds);
		PX_PHYSX_COMMON_API				~BV4Tree();

						bool			load(PxInputStream& stream, bool mismatch);

						void			reset();
						void			operator = (BV4Tree& v);

						bool			init(SourceMesh* meshInterface, const PxBounds3& localBounds);
						void			release();

						SourceMesh*		mMeshInterface;
						LocalBounds		mLocalBounds;

						PxU32			mNbNodes;
						BVDataPacked*	mNodes;
						PxU32			mInitData;
#ifdef GU_BV4_QUANTIZED_TREE
						PxVec3			mCenterOrMinCoeff;	// PT: dequantization coeff, either for Center or Min (depending on AABB format)
						PxVec3			mExtentsOrMaxCoeff;	// PT: dequantization coeff, either for Extents or Max (depending on AABB format)
#endif
						bool			mUserAllocated;	// PT: please keep these 4 bytes right after mCenterOrMinCoeff/mExtentsOrMaxCoeff for safe V4 loading
						bool			mPadding[3];
	};

} // namespace Gu
}

#endif // GU_BV4_H
