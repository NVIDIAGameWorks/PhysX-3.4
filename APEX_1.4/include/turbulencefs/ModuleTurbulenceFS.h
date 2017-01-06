/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_TURBULENCE_FS_H
#define MODULE_TURBULENCE_FS_H

#include "Apex.h"
#include "Shape.h"
#include "VelocitySourceActor.h"
#include "HeatSourceActor.h"
#include "SubstanceSourceActor.h"
#include "FlameEmitterActor.h"
#include "TestBase.h"
#include "PxFiltering.h"

namespace nvidia
{
namespace apex
{

class TurbulenceFSAsset;
class TurbulenceFSAssetAuthoring;

class TurbulenceRenderCallback;

/**
\brief Class for TurbulenceFS module.
*/
class ModuleTurbulenceFS : public Module
{
public:

	/// Set custom timestep parameters for the specified scene (only for one simulation call)
	virtual bool                        setCustomTimestep(const Scene& apexScene, float timestep, int numIterations) = 0;

	/// Enable/disable multi-solve feature for the specified scene (disable by default)
	virtual bool                        setMultiSolveEnabled(const Scene& apexScene, bool enabled) = 0;

	/// Enable output velocity field to UserRenderSurfaceBuffer
	virtual void						enableOutputVelocityField() = 0;

	/// Enable output density field to UserRenderSurfaceBuffer
	virtual void						enableOutputDensityField() = 0;

	/// Enable output flame field to UserRenderSurfaceBuffer
	virtual void						enableOutputFlameField() = 0;

	/// Set custum filter shader to filter collision shapes interacting with Turbulence grids
	virtual void						setCustomFilterShader(physx::PxSimulationFilterShader shader, const void* shaderData, uint32_t shaderDataSize) = 0;

	///Returns pointer to TestBase. Internal feature, return NULL for distribution builds
	virtual const TestBase*				getTestBase(Scene* apexScene) const = 0;

	/// get grid size power. Grid size of all actors will be multiplied by 2^(grid size power). But final grid size restricted by defined values
	virtual int32_t						getGridSizePow() const = 0;

	/// set grid size power. Grid size of all actors will be multiplied by 2^(grid size power). But final grid size restricted by defined values
	virtual void						setGridSizePow(int32_t power) = 0;

	///Prepare renderables for rendering, should be called each frame before using renderables to update them with the most recent data
	virtual void						prepareRenderables(const Scene&) = 0;

	/**
	 \brief Set TurbulenceRenderCallback
	 \see TurbulenceRenderCallback
	 */
	virtual bool						setTurbulenceRenderCallback(const Scene& apexScene, TurbulenceRenderCallback* ) = 0;
	
	/**
	 \brief Get TurbulenceRenderCallback
	 \see TurbulenceRenderCallback
	 */
	virtual TurbulenceRenderCallback*	getTurbulenceRenderCallback(const Scene& apexScene) const = 0;

protected:
	virtual ~ModuleTurbulenceFS() {}
};

#if !defined(_USRDLL)
/** 
 \brief If this module is distributed as a static library, the user must call this
		function before calling ApexSDK::createModule("Turbulence")
 */
void instantiateModuleTurbulenceFS();
#endif

}
} // end namespace nvidia

#endif // MODULE_TURBULENCE_FS_H
