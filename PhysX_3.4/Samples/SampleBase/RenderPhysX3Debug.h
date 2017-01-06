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

#ifndef RENDER_PHYSX3_DEBUG_H
#define RENDER_PHYSX3_DEBUG_H

#include <SamplePointDebugRender.h>
#include <SampleLineDebugRender.h>
#include <SampleTriangleDebugRender.h>
#include "SampleAllocator.h"

	enum RenderPhysX3DebugFlag
	{
		RENDER_DEBUG_WIREFRAME	= (1<<0),
		RENDER_DEBUG_SOLID		= (1<<1),

		RENDER_DEBUG_DEFAULT	= RENDER_DEBUG_SOLID//RENDER_DEBUG_WIREFRAME//|RENDER_DEBUG_SOLID
	};

	namespace physx
	{
		class PxRenderBuffer;
		class PxConvexMeshGeometry;
		class PxCapsuleGeometry;
		class PxSphereGeometry;
		class PxBoxGeometry;
		class PxGeometry;
	}
	class Camera;

	class RenderPhysX3Debug : public SampleFramework::SamplePointDebugRender
							, public SampleFramework::SampleLineDebugRender
							, public SampleFramework::SampleTriangleDebugRender
							, public SampleAllocateable
	{
		public:
							RenderPhysX3Debug(SampleRenderer::Renderer& renderer, SampleFramework::SampleAssetManager& assetmanager);
			virtual			~RenderPhysX3Debug();
		
					void	addAABB(const PxBounds3& box, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					void	addOBB(const PxVec3& boxCenter, const PxVec3& boxExtents, const PxMat33& boxRot, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					void	addSphere(const PxVec3& sphereCenter, float sphereRadius, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);

					void	addBox(const PxBoxGeometry& bg, const PxTransform& tr, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);

					void	addSphere(const PxSphereGeometry& sg, const PxTransform& tr,  const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					void	addCone(float radius, float height, const PxTransform& tr, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);

					void	addSphereExt(const PxVec3& sphereCenter, float sphereRadius, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);					
					void	addConeExt(float radius0, float radius1, const PxVec3& p0, const PxVec3& p1 , const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					
					void	addCylinder(float radius, float height, const PxTransform& tr, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					void	addStar(const PxVec3& p, const float size, const SampleRenderer::RendererColor& color );
					
					void	addCapsule(const PxVec3& p0, const PxVec3& p1, const float radius, const float height, const PxTransform& tr, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					void	addCapsule(const PxCapsuleGeometry& cg, const PxTransform& tr, const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					
					void	addGeometry(const PxGeometry& geom, const PxTransform& tr,  const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					
					void	addRectangle(float width, float length, const PxTransform& tr, const SampleRenderer::RendererColor& color);
					void	addConvex(const PxConvexMeshGeometry& cg, const PxTransform& tr,  const SampleRenderer::RendererColor& color, PxU32 renderFlags = RENDER_DEBUG_DEFAULT);
					
					void	addArrow(const PxVec3& posA, const PxVec3& posB, const SampleRenderer::RendererColor& color);

					void	update(const PxRenderBuffer& debugRenderable);
					void	update(const PxRenderBuffer& debugRenderable, const Camera& camera);
					void	queueForRender();
					void	clear();
		private:
					void	addBox(const PxVec3* pts, const SampleRenderer::RendererColor& color, PxU32 renderFlags);
					void	addCircle(PxU32 nbPts, const PxVec3* pts, const SampleRenderer::RendererColor& color, const PxVec3& offset);
	};

#endif
