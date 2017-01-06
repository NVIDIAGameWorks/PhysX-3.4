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

#include "PxPhysXConfig.h"

#if PX_USE_PARTICLE_SYSTEM_API

#include "RenderParticleSystemActor.h"
#include "RendererParticleSystemShape.h"
#include "RendererSimpleParticleSystemShape.h"
#include "ParticleSystem.h"
#include "RendererMemoryMacros.h"

using namespace SampleRenderer;

RenderParticleSystemActor::RenderParticleSystemActor(SampleRenderer::Renderer& renderer, 
													ParticleSystem* ps,
													bool _mesh_instancing,
													bool _fading,
													PxReal fadingPeriod,
													PxReal debriScaleFactor) : mRenderer(renderer), 
																			mPS(ps),
																			mUseMeshInstancing(_mesh_instancing),
																			mFading(_fading)
{
	if(mRenderer.isSpriteRenderingSupported())
	{
		RendererShape* rs = new SampleRenderer::RendererParticleSystemShape(mRenderer, 
											mPS->getPxParticleBase()->getMaxParticles(), 
											mUseMeshInstancing,
											mFading,
											fadingPeriod,
											debriScaleFactor,
											mPS->getCudaContextManager());
		setRenderShape(rs);
		mUseSpritesMesh = true;
	}
	else
	{
		RendererShape* rs = new SampleRenderer::RendererSimpleParticleSystemShape(mRenderer, 
											mPS->getPxParticleBase()->getMaxParticles());
		setRenderShape(rs);
		mUseSpritesMesh = false;
	}
}

RenderParticleSystemActor::~RenderParticleSystemActor() 
{
	DELETESINGLE(mPS);
}

void RenderParticleSystemActor::update(float deltaTime) 
{
	setTransform(PxTransform(PxIdentity));

#if defined(RENDERER_ENABLE_CUDA_INTEROP)

	SampleRenderer::RendererParticleSystemShape* shape = 
		static_cast<SampleRenderer::RendererParticleSystemShape*>(getRenderShape());

	if (shape->isInteropEnabled() && (mPS->getPxParticleSystem().getParticleBaseFlags()&PxParticleBaseFlag::eGPU))
	{
		PxParticleReadData* data = mPS->getPxParticleSystem().lockParticleReadData(PxDataAccessFlag::eREADABLE | PxDataAccessFlag::eDEVICE);

		if(data)
		{
			if(mUseMeshInstancing) 
			{
				shape->updateInstanced(mPS->getValidParticleRange(),
					reinterpret_cast<CUdeviceptr>(&data->positionBuffer[0]),			
					mPS->getValiditiesDevice(),
					mPS->getOrientationsDevice(),
					data->nbValidParticles);
			} 
			else 
			{
				CUdeviceptr lifetimes = 0;
				if(mFading && mPS->useLifetime()) 
					lifetimes = mPS->getLifetimesDevice();

				shape->updateBillboard(mPS->getValidParticleRange(),
					reinterpret_cast<CUdeviceptr>(&data->positionBuffer[0]),			
					mPS->getValiditiesDevice(),
					lifetimes,
					data->nbValidParticles);
			}

			data->unlock();
		}
	}
	else

#endif

	{
		PxParticleReadData* data = mPS->getPxParticleSystem().lockParticleReadData(PxDataAccessFlag::eREADABLE);

		if(data)
		{
			if(mUseMeshInstancing) 
			{
				SampleRenderer::RendererParticleSystemShape* shape = 
					static_cast<SampleRenderer::RendererParticleSystemShape*>(getRenderShape());
				shape->updateInstanced(mPS->getValidParticleRange(),
								&(mPS->getPositions()[0]), 
								mPS->getValidity(),
								&(mPS->getOrientations()[0]));

			} 
			else 
			{
				if(mUseSpritesMesh)
				{
					SampleRenderer::RendererParticleSystemShape* shape = 
						static_cast<SampleRenderer::RendererParticleSystemShape*>(getRenderShape());
					const PxReal* lifetimes = NULL;
					if(mFading && mPS->useLifetime()) 
					{
						lifetimes = &(mPS->getLifetimes()[0]);
					}
					shape->updateBillboard(mPS->getValidParticleRange(),
									&(mPS->getPositions()[0]), 
									mPS->getValidity(),
									lifetimes);
				}
				else
				{
					SampleRenderer::RendererSimpleParticleSystemShape* shape = 
						static_cast<SampleRenderer::RendererSimpleParticleSystemShape*>(getRenderShape());
					const PxReal* lifetimes = NULL;
					if(mFading && mPS->useLifetime()) 
					{
						lifetimes = &(mPS->getLifetimes()[0]);
					}
					shape->updateBillboard(mPS->getValidParticleRange(),
						&(mPS->getPositions()[0]), 
						mPS->getValidity(),
						lifetimes);
				}
			}

			data->unlock();
		}
	}
}

void RenderParticleSystemActor::updateSubstep(float deltaTime)
{
	mPS->update(deltaTime);
}

#endif // PX_USE_PARTICLE_SYSTEM_API
