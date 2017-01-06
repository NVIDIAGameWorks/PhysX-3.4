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

#include "SamplePreprocessor.h"
#include "RendererMemoryMacros.h"
#include "RenderTexture.h"
#include "Renderer.h"
#include "RendererTexture2DDesc.h"

using namespace physx;
using namespace SampleRenderer;

RenderTexture::RenderTexture(Renderer& renderer, PxU32 id, PxU32 width, PxU32 height, const void* data) :
	mID				(id),
	mTexture		(NULL),
	mOwnsTexture	(true)
{
	RendererTexture2DDesc tdesc;

#if !defined(RENDERER_PS3)
	tdesc.format	= RendererTexture2D::FORMAT_B8G8R8A8;
#else
	tdesc.format	= RendererTexture2D::FORMAT_R8G8B8A8;
#endif

	tdesc.width		= width;
	tdesc.height	= height;
	tdesc.numLevels	= 1;
/*
	tdesc.filter;
	tdesc.addressingU;
	tdesc.addressingV;
	tdesc.renderTarget;
*/
	PX_ASSERT(tdesc.isValid());
	mTexture		= renderer.createTexture2D(tdesc);
	PX_ASSERT(mTexture);

	const PxU32 componentCount = 4;

	if(mTexture)
	{
		PxU32 pitch = 0;
		void* buffer = mTexture->lockLevel(0, pitch);
		PX_ASSERT(buffer);
		if(buffer)
		{
			PxU8* levelDst			= (PxU8*)buffer;
			const PxU8* levelSrc	= (PxU8*)data;
			const PxU32 levelWidth	= mTexture->getWidthInBlocks();
			const PxU32 levelHeight	= mTexture->getHeightInBlocks();
			const PxU32 rowSrcSize	= levelWidth * mTexture->getBlockSize();
			PX_UNUSED(rowSrcSize);
			PX_ASSERT(rowSrcSize <= pitch); // the pitch can't be less than the source row size.
			for(PxU32 row=0; row<levelHeight; row++)
			{ 
				// copy per pixel to handle RBG case, based on component count
				for(PxU32 col=0; col<levelWidth; col++)
				{
					*levelDst++ = levelSrc[0];
					*levelDst++ = levelSrc[1];
					*levelDst++ = levelSrc[2];
					*levelDst++ = 0xFF; //alpha
					levelSrc += componentCount;
				}
			}
		}
		mTexture->unlockLevel(0);
	}
}

RenderTexture::RenderTexture(Renderer& renderer, PxU32 id, RendererTexture2D* texture) :
	mID				(id),
	mTexture		(texture),
	mOwnsTexture	(false)
{
}

RenderTexture::~RenderTexture()
{
	if(mOwnsTexture)
		SAFE_RELEASE(mTexture);
}
