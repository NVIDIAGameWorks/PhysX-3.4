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


#ifndef RAW_LOADER_H
#define RAW_LOADER_H

#include "SampleAllocator.h"
#include "foundation/PxTransform.h"

namespace SampleRenderer
{
	class RendererColor;
}

	class RAWObject : public SampleAllocateable
	{
		public:
					RAWObject();

		const char*	mName;
		PxTransform	mTransform;
	};

	class RAWTexture : public RAWObject
	{
		public:
						RAWTexture();

		PxU32			mID;
		PxU32			mWidth;
		PxU32			mHeight;
		bool			mHasAlpha;
		const SampleRenderer::RendererColor*	mPixels;
	};

	class RAWMesh : public RAWObject
	{
		public:
						RAWMesh();

		PxU32			mNbVerts;
		PxU32			mNbFaces;
		PxU32			mMaterialID;
		const PxVec3*	mVerts;
		const PxVec3*	mVertexNormals;
		const PxVec3*	mVertexColors;
		const PxReal*	mUVs;
		const PxU32*	mIndices;
	};

	class RAWShape : public RAWObject
	{
		public:
						RAWShape();

		PxU32			mNbVerts;
		const PxVec3*	mVerts;
	};

	class RAWHelper : public RAWObject
	{
		public:
						RAWHelper();
	};

	class RAWMaterial : public SampleAllocateable
	{
		public:
						RAWMaterial();

		PxU32			mID;
		PxU32			mDiffuseID;
		PxReal			mOpacity;
		PxVec3			mAmbientColor;
		PxVec3			mDiffuseColor;
		PxVec3			mSpecularColor;
		bool			mDoubleSided;
	};

	class RAWImportCallback
	{
		public:
		virtual ~RAWImportCallback() {}
		virtual	void	newMaterial(const RAWMaterial&)	= 0;
		virtual	void	newMesh(const RAWMesh&)			= 0;
		virtual	void	newShape(const RAWShape&)		= 0;
		virtual	void	newHelper(const RAWHelper&)		= 0;
		virtual	void	newTexture(const RAWTexture&)	= 0;
	};

	bool loadRAWfile(const char* filename, RAWImportCallback& cb, PxReal scale);

#endif
