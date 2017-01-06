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


#ifndef PX_FOUNDATION_PSRENDERBUFFER_H
#define PX_FOUNDATION_PSRENDERBUFFER_H

#include "common/PxRenderBuffer.h"
#include "CmPhysXCommon.h"
#include "PsArray.h"
#include "PsUserAllocated.h"

namespace physx
{
namespace Cm
{
	/**
	Implementation of PxRenderBuffer.
	*/
	class RenderBuffer : public PxRenderBuffer, public Ps::UserAllocated
	{

		template <typename T>
		void append(Ps::Array<T>& dst, const T* src, PxU32 count)
		{
			dst.reserve(dst.size() + count);
			for(const T* end=src+count; src<end; ++src)
				dst.pushBack(*src);
		}

	public:

		RenderBuffer() :
			mPoints(PX_DEBUG_EXP("renderBufferPoints")),
			mLines(PX_DEBUG_EXP("renderBufferLines")),
			mTriangles(PX_DEBUG_EXP("renderBufferTriangles")),
			mTexts(PX_DEBUG_EXP("renderBufferTexts")),
			mCharBuf(PX_DEBUG_EXP("renderBufferCharBuf"))
		{}
		

		virtual PxU32 getNbPoints() const { return mPoints.size(); }
		virtual const PxDebugPoint* getPoints() const { return mPoints.begin(); }
		virtual PxU32 getNbLines() const { return mLines.size(); }
		virtual const PxDebugLine* getLines() const { return mLines.begin(); }
		virtual PxU32 getNbTriangles() const { return mTriangles.size(); }
		virtual const PxDebugTriangle* getTriangles() const { return mTriangles.begin(); }
		virtual PxU32 getNbTexts() const { return mTexts.size(); }
		virtual const PxDebugText* getTexts() const { return mTexts.begin(); }

		virtual void append(const PxRenderBuffer& other)
		{
			append(mPoints, other.getPoints(), other.getNbPoints());
			append(mLines, other.getLines(), other.getNbLines());
			append(mTriangles, other.getTriangles(), other.getNbTriangles());
			append(mTexts, other.getTexts(), other.getNbTexts());
		}

		virtual void clear()
		{
			mPoints.clear(); 
			mLines.clear();
			mTriangles.clear();
			mTexts.clear();
			mCharBuf.clear();
		}

		bool empty() const 
		{
			return mPoints.empty() && mLines.empty() && mTriangles.empty() && mTexts.empty()&& mCharBuf.empty();
		}

		void shift(const PxVec3& delta)
		{
			for(PxU32 i=0; i < mPoints.size(); i++)
				mPoints[i].pos += delta;

			for(PxU32 i=0; i < mLines.size(); i++)
			{
				mLines[i].pos0 += delta;
				mLines[i].pos1 += delta;
			}

			for(PxU32 i=0; i < mTriangles.size(); i++)
			{
				mTriangles[i].pos0 += delta;
				mTriangles[i].pos1 += delta;
				mTriangles[i].pos2 += delta;
			}

			for(PxU32 i=0; i < mTexts.size(); i++)
				mTexts[i].position += delta;
		}

		Ps::Array<PxDebugPoint>		mPoints;
		Ps::Array<PxDebugLine>		mLines;
		Ps::Array<PxDebugTriangle>	mTriangles;
		Ps::Array<PxDebugText>		mTexts;
		Ps::Array<char>				mCharBuf;
	};

} // Cm

}

#endif
