/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef _MODIFIER
#define _MODIFIER(name)
#endif

#ifndef _MODIFIER_SPRITE
#define _MODIFIER_SPRITE(name)
#endif

#ifndef _MODIFIER_MESH
#define _MODIFIER_MESH(name)
#endif

//All
_MODIFIER(Rotation)
_MODIFIER(SimpleScale)
_MODIFIER(RandomScale)
_MODIFIER(ScaleByMass)
_MODIFIER(ColorVsLife)
_MODIFIER(ColorVsDensity)
_MODIFIER(SubtextureVsLife)
_MODIFIER(OrientAlongVelocity)
_MODIFIER(ScaleAlongVelocity)
_MODIFIER(RandomSubtexture)
_MODIFIER(RandomRotation)
_MODIFIER(ScaleVsLife)
_MODIFIER(ScaleVsDensity)
_MODIFIER(ScaleVsCameraDistance)
_MODIFIER(ViewDirectionSorting)
_MODIFIER(RotationRate)
_MODIFIER(RotationRateVsLife)
_MODIFIER(OrientScaleAlongScreenVelocity)
_MODIFIER(ColorVsVelocity)

//Sprite
_MODIFIER_SPRITE(SimpleScale)
_MODIFIER_SPRITE(RandomScale)
_MODIFIER_SPRITE(ScaleByMass)
_MODIFIER_SPRITE(ColorVsLife)
_MODIFIER_SPRITE(ColorVsDensity)
_MODIFIER_SPRITE(SubtextureVsLife)
_MODIFIER_SPRITE(RandomSubtexture)
_MODIFIER_SPRITE(RandomRotation)
_MODIFIER_SPRITE(ScaleVsLife)
_MODIFIER_SPRITE(ScaleVsDensity)
_MODIFIER_SPRITE(ScaleVsCameraDistance)
_MODIFIER_SPRITE(ViewDirectionSorting)
_MODIFIER_SPRITE(RotationRate)
_MODIFIER_SPRITE(RotationRateVsLife)
_MODIFIER_SPRITE(OrientScaleAlongScreenVelocity)
_MODIFIER_SPRITE(ColorVsVelocity)

//Mesh
_MODIFIER_MESH(Rotation)
_MODIFIER_MESH(SimpleScale)
_MODIFIER_MESH(RandomScale)
_MODIFIER_MESH(ScaleByMass)
_MODIFIER_MESH(ColorVsLife)
_MODIFIER_MESH(ColorVsDensity)
_MODIFIER_MESH(OrientAlongVelocity)
_MODIFIER_MESH(ScaleAlongVelocity)
_MODIFIER_MESH(ScaleVsLife)
_MODIFIER_MESH(ScaleVsDensity)
_MODIFIER_MESH(ScaleVsCameraDistance)
_MODIFIER_MESH(ColorVsVelocity)

#undef _MODIFIER
#undef _MODIFIER_SPRITE
#undef _MODIFIER_MESH
