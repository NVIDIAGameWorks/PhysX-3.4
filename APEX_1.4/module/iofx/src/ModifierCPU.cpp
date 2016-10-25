/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ModifierImpl.h"

#pragma warning (disable : 4127)

#include "ModifierData.h"

namespace nvidia
{
namespace apex
{

#define MODIFIER_DECL
#define CURVE_TYPE const nvidia::apex::Curve*
#define EVAL_CURVE(curve, value) curve->evaluate(value)
#define PARAMS_NAME(name) name ## ParamsCPU

#include "ModifierSrc.h"

#undef MODIFIER_DECL
#undef CURVE_TYPE
#undef EVAL_CURVE
#undef PARAMS_NAME

}

namespace iofx
{

#define _MODIFIER(name) \
	void name ## ModifierImpl :: mapParamsCPU(ModifierParamsMapperCPU& mapper) const \
	{ \
		mapParams(mapper, (name ## ParamsCPU *)NULL); \
	} \
	 
#define _MODIFIER_SPRITE(name) \
	void updateSprite_##name (const void* params, const SpriteInput& input, SpritePublicState& pubState, SpritePrivateState& privState, const ModifierCommonParams& common, RandState& randState) \
	{ \
		modifier##name <void, false, ModifierUsage_Sprite> (*static_cast<const name##ParamsCPU *>(params), input, pubState, privState, common, randState); \
	} \
	void updateSpriteOnSpawn_##name (const void* params, const SpriteInput& input, SpritePublicState& pubState, SpritePrivateState& privState, const ModifierCommonParams& common, RandState& randState) \
	{ \
		modifier##name <void, true, ModifierUsage_Sprite> (*static_cast<const name##ParamsCPU *>(params), input, pubState, privState, common, randState); \
	} \
	ModifierImpl::updateSpriteFunc name##ModifierImpl :: getUpdateSpriteFunc(ModifierStage stage) const \
	{ \
		switch (stage) { \
		case ModifierStage_Spawn: return &updateSpriteOnSpawn_##name; \
		case ModifierStage_Continuous: return &updateSprite_##name; \
		default: \
			PX_ALWAYS_ASSERT(); \
			return 0; \
		} \
	}

#define _MODIFIER_MESH(name) \
	void updateMesh_##name (const void* params, const MeshInput& input, MeshPublicState& pubState, MeshPrivateState& privState, const ModifierCommonParams& common, RandState& randState) \
	{ \
		modifier##name <void, false, ModifierUsage_Mesh> (*static_cast<const name##ParamsCPU *>(params), input, pubState, privState, common, randState); \
	} \
	void updateMeshOnSpawn_##name (const void* params, const MeshInput& input, MeshPublicState& pubState, MeshPrivateState& privState, const ModifierCommonParams& common, RandState& randState) \
	{ \
		modifier##name <void, true, ModifierUsage_Mesh> (*static_cast<const name##ParamsCPU *>(params), input, pubState, privState, common, randState); \
	} \
	ModifierImpl::updateMeshFunc name##ModifierImpl :: getUpdateMeshFunc(ModifierStage stage) const \
	{ \
		switch (stage) { \
		case ModifierStage_Spawn: return &updateMeshOnSpawn_##name; \
		case ModifierStage_Continuous: return &updateMesh_##name; \
		default: \
			PX_ALWAYS_ASSERT(); \
			return 0; \
		} \
	}

#include "ModifierList.h"

}
} // namespace nvidia
