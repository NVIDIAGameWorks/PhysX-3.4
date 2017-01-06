/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_IOFX_H
#define MODULE_IOFX_H

#include "Apex.h"
#include "TestBase.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class IofxAsset;
class IofxAssetAuthoring;
class RenderVolume;
class IofxRenderCallback;
class IofxRenderable;


/**
\brief Iterate over all renderables in IOFX module. IMPORTANT: release() should be called for each returned not-NULL renderable!
*/
class IofxRenderableIterator : public ApexInterface
{
public:

	/// Return the first renderable in IOFX module
	virtual IofxRenderable* getFirst() = 0;
	
	/// Return the next renderable IOFX module
	virtual IofxRenderable* getNext() = 0;
};


/**
 \brief IOFX Module. 
 
 This module is used to convert physical parameters of particles into visual parameters
 that can be used to render particles.  ApexRenderVolume provide the ability to partition world space
 into separate renderables.
 */
class ModuleIofx : public Module
{
protected:
	virtual							~ModuleIofx() {}

public:
	/// Disable use of OGL/D3D Interop.  Must be called before emitters are created to have any effect
	virtual void                    disableCudaInterop() = 0;

	/// Disable use of CUDA IOFX modifiers. Must be called before emitters are created to have any effect
	virtual void                    disableCudaModifiers() = 0;

	/** \brief Disables deferred allocation of IOFX actors
	 *
	 * By default, IOFX actors and their render resources will not be
	 * created until it has objects to render.
	 */
	virtual void                    disableDeferredRenderableAllocation() = 0;

	/** \brief Create a new render volume
	 *
	 * Render volumes may be created at any time, unlike most other APEX
	 * asset and actor types.  Their insertion into the scene is
	 * deferred if the simulation is active.
	 */
	virtual RenderVolume*     		createRenderVolume(const Scene& apexScene, const PxBounds3& b, uint32_t priority, bool allIofx) = 0;

	/** \brief Release a render volume
	 *
	 * Render volumes may be released at any time, unlike most other APEX
	 * asset and actor types.  If the simulation is active, their
	 * resources are released at the end of the simulation step.
	 */
	virtual void                    releaseRenderVolume(RenderVolume& volume) = 0;

	/// Get TestBase implementation of Iofx scene
	virtual const TestBase*			getTestBase(Scene* apexScene) const = 0;

	/**
	 \brief Set IofxRenderCallback
	 \see IofxRenderCallback
	 */
	virtual bool                    setIofxRenderCallback(const Scene& apexScene, IofxRenderCallback* ) = 0;
	
	/**
	 \brief Get IofxRenderCallback
	 \see IofxRenderCallback
	 */
	virtual IofxRenderCallback*   	getIofxRenderCallback(const Scene& apexScene) const = 0;

	/**
	\brief Create an iterator for all renderables in IOFX module
	*/
	virtual IofxRenderableIterator* createIofxRenderableIterator(const Scene&) = 0;

	/**
	\brief Prepare renderables for rendering, should be called each frame before using renderables to update them with the most recent data
	*/
	virtual void                    prepareRenderables(const Scene&) = 0;
};


PX_POP_PACK

}
} // namespace nvidia

#endif // MODULE_IOFX_H
