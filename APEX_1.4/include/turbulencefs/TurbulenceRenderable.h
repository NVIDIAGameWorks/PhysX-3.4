/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef TURBULENCE_RENDERABLE_H
#define TURBULENCE_RENDERABLE_H

#include "foundation/Px.h"
#include "ApexInterface.h"
#include "Renderable.h"
#include "TurbulenceRenderCallback.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
 \brief Render surface field type
 */
struct TurbulenceFieldType
{
	/**
	 \brief Enum of render surface field types
	 */
	enum Enum
	{
		VELOCITY = 0,
		DENSITY,
		FLAME,

		MAX_COUNT
	};
};

/**
 \brief TurbulenceRenderData stores render data for one Turbulence Renderable.
 */
struct TurbulenceRenderData
{
	///UserRenderSurfaceDesc array for all field types
	UserRenderSurfaceDesc	fieldSurfaceDescs[TurbulenceFieldType::MAX_COUNT];
	
	///UserRenderSurface array for all field types
	UserRenderSurface*		fieldSurfaces[TurbulenceFieldType::MAX_COUNT];
};

/**
 \brief The Turbulence renderable represents a unit of rendering.
 
 It contains complete information to render one Turbulence Actor.
*/
class TurbulenceRenderable : public ApexInterface
{
public:
	///Return render data.
	virtual const TurbulenceRenderData*	getRenderData() const = 0;

	///Return AABB of this renderable.
	virtual const physx::PxBounds3&		getBounds() const = 0;

protected:
	virtual	~TurbulenceRenderable() {}

};

PX_POP_PACK

}
} // end namespace nvidia

#endif // TURBULENCE_RENDERABLE_H
