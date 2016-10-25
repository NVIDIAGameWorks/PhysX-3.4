/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __COMMON_H__
#define __COMMON_H__

#define APEX_CUDA_MODULE_PREFIX IOFX_

#include "ApexCuda.h"
#include "InplaceTypes.h"
#include "IofxManagerIntl.h"
#include "PxBounds3.h"
#include <float.h>

#if PX_WINDOWS_FAMILY
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4408)
#endif

#include <vector_types.h>

#if PX_WINDOWS_FAMILY
#pragma warning(pop)
#endif

/* Reduce functions assume warps per block <= 32 !! */


#define ACTOR_RANGE_KERNEL_CONFIG (2, WARP_SIZE * 2)

const unsigned int BBOX_BLOCK_BOUNDS_PAD = 1;
const unsigned int BBOX_WARP_BOUNDS_PAD = 1;
#define BBOX_KERNEL_CONFIG (BBOX_BLOCK_BOUNDS_PAD * 6, WARP_SIZE * 7, 0, 1, WARP_SIZE)

const unsigned int SPRITE_MAX_DWORDS_PER_OUTPUT = 18;
const unsigned int MESH_MAX_DWORDS_PER_OUTPUT = 22;

#define SPRITE_TEXTURE_MODIFIER_KERNEL_CONFIG (0, WARP_SIZE * 4)

const unsigned int RADIX_SORT_NBITS = 4;

#define SORT_KERNEL_CONFIG (0, WARP_SIZE * 2)

//+1 here is to avoid shared memory bank conflicts
const unsigned int NEW_SORT_WARP_STRIDE_PAD = 1;
const unsigned int NEW_SORT_KEY_DIGITS = (1U << RADIX_SORT_NBITS);
const unsigned int NEW_SORT_VECTOR_SIZE = 4;

/* step 1: (NEW_SORT_KEY_DIGITS * NEW_SORT_WARP_STRIDE_PAD, WARP_SIZE * 2 + NEW_SORT_KEY_DIGITS) */
/* step 2: (0, WARP_SIZE * 2)
/* step 3 & block: (1 + NEW_SORT_KEY_DIGITS * NEW_SORT_WARP_STRIDE_PAD, WARP_SIZE * 2 + WARP_SIZE * NEW_SORT_VECTOR_SIZE * 2 + NEW_SORT_KEY_DIGITS) */
#define NEW_SORT_KERNEL_CONFIG (1 + NEW_SORT_KEY_DIGITS * NEW_SORT_WARP_STRIDE_PAD, WARP_SIZE * 2 + WARP_SIZE * NEW_SORT_VECTOR_SIZE * 2 + NEW_SORT_KEY_DIGITS, 0, (NEW_SORT_KEY_DIGITS * MAX_BOUND_BLOCKS / 4) / WARP_SIZE)


const unsigned int STATE_ID_MASK = 0x7FFFFFFFu;
const unsigned int STATE_ID_DIST_SIGN = 0x80000000u;


namespace nvidia
{
namespace iofx
{

#ifdef __CUDACC__

APEX_CUDA_CALLABLE PX_INLINE IofxSlice uint4_to_IofxSlice(uint4 v)
{
	IofxSlice ret;
	ret.x = v.x;
	ret.y = v.y;
	ret.z = v.z;
	ret.w = v.w;
	return ret;
}

APEX_CUDA_CALLABLE PX_INLINE uint4 IofxSlice_to_uint4(IofxSlice s)
{
	return make_uint4(s.x, s.y, s.z, s.w);
}
#endif


//struct VolumeParams
#define INPLACE_TYPE_STRUCT_NAME VolumeParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxBounds3,	bounds) \
	INPLACE_TYPE_FIELD(uint32_t,		priority)
#include INPLACE_TYPE_BUILD()

typedef InplaceArray<VolumeParams> VolumeParamsArray;
typedef InplaceArray<uint32_t> ActorClassIDBitmapArray;

typedef InplaceArray<uint32_t> ActorIDRemapArray;

//struct ModifierListElem
#define INPLACE_TYPE_STRUCT_NAME ModifierListElem
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,		type) \
	INPLACE_TYPE_FIELD(InplaceHandleBase,	paramsHandle)
#include INPLACE_TYPE_BUILD()

typedef InplaceArray<ModifierListElem> ModifierList;

//struct AssetParams
#define INPLACE_TYPE_STRUCT_NAME AssetParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(ModifierList,	spawnModifierList) \
	INPLACE_TYPE_FIELD(ModifierList,	continuousModifierList)
#include INPLACE_TYPE_BUILD()

//struct ClientParams
#define INPLACE_TYPE_STRUCT_NAME ClientParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,				objectScale) \
	INPLACE_TYPE_FIELD(InplaceHandle<AssetParams>,	assetParamsHandle)
#include INPLACE_TYPE_BUILD()

typedef InplaceArray< InplaceHandle<ClientParams> > ClientParamsHandleArray;

struct SpritePrivateState;

struct SpritePrivateStateArgs
{
	IofxSlice* g_state[1];

#ifdef __CUDACC__
	static __device__ void read(const SpritePrivateStateArgs& args, SpritePrivateState& state, unsigned int pos);
	static __device__ void write(SpritePrivateStateArgs& args, const SpritePrivateState& state, unsigned int pos);
#endif
};

struct MeshPrivateState;

struct MeshPrivateStateArgs
{
	IofxSlice* g_state[3];

#ifdef __CUDACC__
	static __device__ void read(const MeshPrivateStateArgs& args, MeshPrivateState& state, unsigned int pos);
	static __device__ void write(MeshPrivateStateArgs& args, const MeshPrivateState& state, unsigned int pos);
#endif
};

}
} // namespace nvidia

#endif
