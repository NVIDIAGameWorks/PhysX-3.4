/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODIFIER_DEFS_H
#define MODIFIER_DEFS_H

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
 \brief Roll type of a particle
 */
struct ApexMeshParticleRollType
{
	/**
	 \brief Enum of roll type of a particle
	 */
	enum Enum
	{
		SPHERICAL = 0, ///< roll as a sphere
		CUBIC,		   ///< as a cubical object
		FLAT_X,		   ///< as a flat object (shortest axis is x)
		FLAT_Y,		   ///< as a flat object (shortest axis is y)
		FLAT_Z,		   ///< as a flat object (shortest axis is z)
		LONG_X,		   ///< as a long object (longest axis is x)
		LONG_Y,		   ///< as a long object (longest axis is y)
		LONG_Z,		   ///< as a long object (longest axis is z)
		SPRITE,		   ///< as a sprite

		COUNT
	};
};

/**
 \brief Modifiers list
 \note 	These are serialized to disk, so if you reorder them or change existing modifier types, you
		will need to version the stream and map the old values to the new values.
		If new values are just appended, no other special care needs to be handled.
 */
enum ModifierTypeEnum
{
	ModifierType_Invalid						= 0,
	ModifierType_Rotation						= 1,
	ModifierType_SimpleScale					= 2,
	ModifierType_RandomScale					= 3,
	ModifierType_ColorVsLife					= 4,
	ModifierType_ColorVsDensity					= 5,
	ModifierType_SubtextureVsLife				= 6,
	ModifierType_OrientAlongVelocity			= 7,
	ModifierType_ScaleAlongVelocity				= 8,
	ModifierType_RandomSubtexture				= 9,
	ModifierType_RandomRotation					= 10,
	ModifierType_ScaleVsLife					= 11,
	ModifierType_ScaleVsDensity					= 12,
	ModifierType_ScaleVsCameraDistance			= 13,
	ModifierType_ViewDirectionSorting			= 14,
	ModifierType_RotationRate					= 15,
	ModifierType_RotationRateVsLife				= 16,
	ModifierType_OrientScaleAlongScreenVelocity	= 17,
	ModifierType_ScaleByMass					= 18,
	ModifierType_ColorVsVelocity				= 19,

	ModifierType_Count
};

/**
 \brief Stage at which the modifier is applied
 */
enum ModifierStage
{
	ModifierStage_Spawn = 0,		///< at the spawn
	ModifierStage_Continuous = 1,	///< on every frame

	ModifierStage_Count
};

/**
 \brief Color channel
 */
enum ColorChannel
{
	ColorChannel_Red	= 0,
	ColorChannel_Green	= 1,
	ColorChannel_Blue	= 2,
	ColorChannel_Alpha	= 3
};

/**
 \brief Scale axis
 */
enum ScaleAxis
{
	ScaleAxis_X			= 0,
	ScaleAxis_Y			= 1,
	ScaleAxis_Z			= 2
};

/**
 \brief Modifier usage
 */
enum ModifierUsage
{
	ModifierUsage_Spawn			= 0x01,
	ModifierUsage_Continuous	= 0x02,

	ModifierUsage_Sprite		= 0x04,
	ModifierUsage_Mesh			= 0x08,
};

PX_POP_PACK

}
} // namespace nvidia

#endif // MODIFIER_DEFS_H
