/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef TURBULENCE_RENDER_CALLBACK_H
#define TURBULENCE_RENDER_CALLBACK_H

#include "foundation/Px.h"
#include "UserRenderCallback.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class TurbulenceRenderable;

/**
\brief Velocity format for Turbulence rendering
*/
struct TurbulenceVelocityFormat
{
	/**
	 \brief Enum of velocity format for Turbulence rendering
	 */
	enum Enum
	{
		DEFAULT = 0,
		HALF4 = RenderDataFormat::HALF4,
		FLOAT4 = RenderDataFormat::FLOAT4,
	};
};

/**
\brief Density format for Turbulence rendering
*/
struct TurbulenceDensityFormat
{
	/**
	\brief Enum of density format for Turbulence rendering
	*/
	enum Enum
	{
		DEFAULT = 0,
		UBYTE1 = RenderDataFormat::UBYTE1,
		HALF1 = RenderDataFormat::HALF1,
		FLOAT1 = RenderDataFormat::FLOAT1,
	};
};

/**
\brief Flame format for Turbulence rendering
*/
struct TurbulenceFlameFormat
{
	/**
	\brief Enum of flame format for Turbulence rendering
	*/
	enum Enum
	{
		DEFAULT = 0,
		HALF4 = RenderDataFormat::HALF4,
		FLOAT4 = RenderDataFormat::FLOAT4,
	};
};

/**
\brief Render layout for Turbulence rendering
*/
struct TurbulenceRenderLayout
{
	TurbulenceRenderLayout()
	{
		setDefaults();
	}

	///set default formats
	void setDefaults()
	{
		velocityFormat = TurbulenceVelocityFormat::DEFAULT;
		densityFormat = TurbulenceDensityFormat::DEFAULT;
		flameFormat = TurbulenceFlameFormat::DEFAULT;
	}

	///Velocity format
	TurbulenceVelocityFormat::Enum	velocityFormat;
	
	///Density format
	TurbulenceDensityFormat::Enum	densityFormat;
	
	///Flame format
	TurbulenceFlameFormat::Enum		flameFormat;
};

/**
\brief User defined callback for Turbulence rendering
*/
class TurbulenceRenderCallback : public UserRenderCallback
{
public:
	///Called just after a new Turbulence renderable was created
	virtual void onCreatedTurbulenceRenderable(TurbulenceRenderable& ) {}

	///Called just after an Turbulence renderable was updated
	virtual void onUpdatedTurbulenceRenderable(TurbulenceRenderable& ) {}

	///Called just before an Turbulence renderable is going to be released
	virtual void onReleasingTurbulenceRenderable(TurbulenceRenderable& ) {}

	///Returns render layout
	virtual bool getTurbulenceRenderLayout(TurbulenceRenderLayout& renderLayout, uint32_t sizeX, uint32_t sizeY, uint32_t sizeZ, uint32_t densitySizeMultiplier, uint32_t flameSizeMultiplier)
	{
		PX_UNUSED(renderLayout);
		PX_UNUSED(sizeX);
		PX_UNUSED(sizeY);
		PX_UNUSED(sizeZ);
		PX_UNUSED(densitySizeMultiplier);
		PX_UNUSED(flameSizeMultiplier);
		return false;
	}
};

PX_POP_PACK

}
} // end namespace nvidia

#endif // TURBULENCE_RENDER_CALLBACK_H
