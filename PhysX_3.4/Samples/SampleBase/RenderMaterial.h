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

#ifndef RENDER_MATERIAL_H
#define RENDER_MATERIAL_H

#include "RenderBaseObject.h"
#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxVec3.h"

namespace SampleRenderer
{
	class Renderer;
	class RendererMaterial;
	class RendererMaterialInstance;
}

	class RenderTexture;

	class RenderMaterial : public RenderBaseObject
	{
		public:
															RenderMaterial(SampleRenderer::Renderer& renderer, 
																			const PxVec3& diffuseColor, 
																			PxReal opacity, 
																			bool doubleSided, 
																			PxU32 id, 
																			RenderTexture* texture,
																			bool lit = true,
																			bool flat = false,
																			bool instanced = false);

															RenderMaterial(SampleRenderer::Renderer& renderer, 
																			SampleRenderer::RendererMaterial* mat, 
																			SampleRenderer::RendererMaterialInstance* matInstance, 
																			PxU32 id);
		virtual												~RenderMaterial();

		// the intent of this function is to update shaders variables, when needed (e.g. on resize)
		virtual void										update(SampleRenderer::Renderer& renderer);
				void										setDiffuseColor(const PxVec4& color);
				void										setParticleSize(const PxReal particleSize);
				void										setShadeMode(bool flat);

				SampleRenderer::RendererMaterial*			mRenderMaterial;
				SampleRenderer::RendererMaterialInstance*	mRenderMaterialInstance;
				PxU32										mID;
				bool										mDoubleSided;
				bool										mOwnsRendererMaterial;
	};

#endif
