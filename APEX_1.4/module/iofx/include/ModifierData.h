/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __MODIFIER_DATA_H__
#define __MODIFIER_DATA_H__

#include "ApexUsingNamespace.h"
#include "PxVec3.h"
#include <PxMat33.h>
#include "InplaceTypes.h"
#include "InplaceStorage.h"
#include "RandState.h"

#include "../../../include/iofx/IofxRenderCallback.h"

namespace nvidia
{
namespace iofx
{

#ifndef __CUDACC__
PX_INLINE float saturate(float x)
{
	return (x < 0.0f) ? 0.0f : (1.0f < x) ? 1.0f : x;
}
#endif

// output color is RenderDataFormat::B8G8R8A8
#define FLT_TO_BYTE(x) ( (unsigned int)(saturate(PxAbs(x)) * 255) )
#define MAKE_COLOR_UBYTE4(r, g, b, a) ( ((r) << 16) | ((g) << 8) | ((b) << 0) | ((a) << 24) )


class IosObjectBaseData;

//struct ModifierCommonParams
#define INPLACE_TYPE_STRUCT_NAME ModifierCommonParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(InplaceBool,		inputHasCollision) \
	INPLACE_TYPE_FIELD(InplaceBool,		inputHasDensity) \
	INPLACE_TYPE_FIELD(InplaceBool,		inputHasUserData) \
	INPLACE_TYPE_FIELD(PxVec3,	upVector) \
	INPLACE_TYPE_FIELD(PxVec3,	eyePosition) \
	INPLACE_TYPE_FIELD(PxVec3,	eyeDirection) \
	INPLACE_TYPE_FIELD(PxVec3,	eyeAxisX) \
	INPLACE_TYPE_FIELD(PxVec3,	eyeAxisY) \
	INPLACE_TYPE_FIELD(float,	zNear) \
	INPLACE_TYPE_FIELD(float,	deltaTime)
#include INPLACE_TYPE_BUILD()


// Mesh structs
struct MeshInput
{
	PxVec3		position;
	float		mass;
	PxVec3		velocity;
	float		liferemain;
	float		density;
	PxVec3		collisionNormal;
	uint32_t	collisionFlags;
	uint32_t	userData;

	PX_INLINE void load(const IosObjectBaseData& objData, uint32_t pos);
};

struct MeshPublicState
{
	PxMat33		rotation;
	PxVec3		scale;

	float	color[4];

	static APEX_CUDA_CALLABLE PX_INLINE void initDefault(MeshPublicState& state, float objectScale)
	{
		state.rotation = PxMat33(PxIdentity);
		state.scale = PxVec3(objectScale);

		state.color[0] = 1.0f;
		state.color[1] = 1.0f;
		state.color[2] = 1.0f;
		state.color[3] = 1.0f;
	}
};

/* TODO: Private state size should be declared by each IOFX asset, so the IOS can allocate
 * the private buffer dynamically based on the IOFX assets used with the IOS.  Each asset would
 * in turn be given an offset for their private data in this buffer.
 */
struct MeshPrivateState
{
	PxMat33			rotation;

	static APEX_CUDA_CALLABLE PX_INLINE void initDefault(MeshPrivateState& state)
	{
		state.rotation = PxMat33(PxIdentity);
	}
};


//struct MeshOutputLayout
#define INPLACE_TYPE_STRUCT_NAME MeshOutputLayout
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	stride) \
	INPLACE_TYPE_FIELD_N(uint32_t,	offsets, IofxMeshRenderLayoutElement::MAX_COUNT)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

#ifdef __CUDACC__
#define WRITE_TO_FLOAT(data) { *((volatile float*)(sdata + ((offset) >> 2) * pitch) + idx) = (data); offset += 4; }
#define WRITE_TO_UINT(data) { *((volatile unsigned int*)(sdata + ((offset) >> 2) * pitch + idx)) = (data); offset += 4; }

	__device__ PX_INLINE void write(volatile unsigned int* sdata, unsigned int idx, unsigned int pitch, const MeshInput& input, const MeshPublicState& state, unsigned int outputID) const
	{
#else
#define WRITE_TO_FLOAT(data) { *(float*)(outputPtr + outputID * stride + offset) = (data); offset += 4; }
#define WRITE_TO_UINT(data) { *(unsigned int*)(outputPtr + outputID * stride + offset) = (data); offset += 4; }

	PX_INLINE void write(uint32_t outputID, const MeshInput& input, const MeshPublicState& state, const uint8_t* outputPtr) const
	{
		if (outputPtr == NULL)
		{
			return;
		}
#endif
		uint32_t offset;
		if ((offset = offsets[IofxMeshRenderLayoutElement::POSITION_FLOAT3]) != static_cast<uint32_t>(-1)) //POSITION: 3 dwords
		{
			WRITE_TO_FLOAT( input.position.x )
			WRITE_TO_FLOAT( input.position.y )
			WRITE_TO_FLOAT( input.position.z )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::ROTATION_SCALE_FLOAT3x3]) != static_cast<uint32_t>(-1)) //ROTATION_SCALE: 9 dwords
		{
			PxVec3 axis0 = state.rotation.column0 * state.scale.x;
			PxVec3 axis1 = state.rotation.column1 * state.scale.y;
			PxVec3 axis2 = state.rotation.column2 * state.scale.z;

			WRITE_TO_FLOAT( axis0.x )
			WRITE_TO_FLOAT( axis0.y )
			WRITE_TO_FLOAT( axis0.z )

			WRITE_TO_FLOAT( axis1.x )
			WRITE_TO_FLOAT( axis1.y )
			WRITE_TO_FLOAT( axis1.z )

			WRITE_TO_FLOAT( axis2.x )
			WRITE_TO_FLOAT( axis2.y )
			WRITE_TO_FLOAT( axis2.z )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::VELOCITY_LIFE_FLOAT4]) != static_cast<uint32_t>(-1)) //VELOCITY: 3 dwords
		{
			WRITE_TO_FLOAT( input.velocity.x )
			WRITE_TO_FLOAT( input.velocity.y )
			WRITE_TO_FLOAT( input.velocity.z )
			WRITE_TO_FLOAT( input.liferemain )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::DENSITY_FLOAT1]) != static_cast<uint32_t>(-1)) //DENSITY: 1 dword
		{
			WRITE_TO_FLOAT( input.density )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::COLOR_BGRA8]) != static_cast<uint32_t>(-1)) //COLOR: 1 dword
		{
			WRITE_TO_UINT( MAKE_COLOR_UBYTE4( FLT_TO_BYTE(state.color[0]), 
											FLT_TO_BYTE(state.color[1]), 
											FLT_TO_BYTE(state.color[2]), 
											FLT_TO_BYTE(state.color[3]) ) )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::COLOR_RGBA8]) != static_cast<uint32_t>(-1)) //COLOR: 1 dword
		{
			WRITE_TO_UINT( MAKE_COLOR_UBYTE4( FLT_TO_BYTE(state.color[2]), 
											FLT_TO_BYTE(state.color[1]), 
											FLT_TO_BYTE(state.color[0]), 
											FLT_TO_BYTE(state.color[3]) ) )
											
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::COLOR_FLOAT4]) != static_cast<uint32_t>(-1)) //COLOR_FLOAT4: 4 dword
		{
			WRITE_TO_FLOAT( state.color[0] )
			WRITE_TO_FLOAT( state.color[1] )
			WRITE_TO_FLOAT( state.color[2] )
			WRITE_TO_FLOAT( state.color[3] )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::USER_DATA_UINT1]) != static_cast<uint32_t>(-1)) //USER_DATA: 1 dword
		{
			WRITE_TO_UINT( input.userData )
		}
		if ((offset = offsets[IofxMeshRenderLayoutElement::POSE_FLOAT3x4]) != static_cast<uint32_t>(-1)) //POSE: 12 dwords
		{
			PxVec3 axis0 = state.rotation.column0 * state.scale.x;
			PxVec3 axis1 = state.rotation.column1 * state.scale.y;
			PxVec3 axis2 = state.rotation.column2 * state.scale.z;

			WRITE_TO_FLOAT( axis0.x )
			WRITE_TO_FLOAT( axis1.x )
			WRITE_TO_FLOAT( axis2.x )
			WRITE_TO_FLOAT( input.position.x )

			WRITE_TO_FLOAT( axis0.y )
			WRITE_TO_FLOAT( axis1.y )
			WRITE_TO_FLOAT( axis2.y )
			WRITE_TO_FLOAT( input.position.y )

			WRITE_TO_FLOAT( axis0.z )
			WRITE_TO_FLOAT( axis1.z )
			WRITE_TO_FLOAT( axis2.z )
			WRITE_TO_FLOAT( input.position.z )
		}
	}
#undef WRITE_TO_UINT
#undef WRITE_TO_FLOAT
};


// Sprite structs
struct SpriteInput
{
	PxVec3		position;
	float		mass;
	PxVec3		velocity;
	float		liferemain;
	float		density;
	uint32_t	userData;

	PX_INLINE void load(const IosObjectBaseData& objData, uint32_t pos);
};


struct SpritePublicState
{
	PxVec3			scale;
	float			subTextureId;
	float			rotation;

	float			color[4];

	static APEX_CUDA_CALLABLE PX_INLINE void initDefault(SpritePublicState& state, float objectScale)
	{
		state.scale = PxVec3(objectScale);

		state.subTextureId = 0;
		state.rotation = 0;

		state.color[0] = 1.0f;
		state.color[1] = 1.0f;
		state.color[2] = 1.0f;
		state.color[3] = 1.0f;
	}
};

/* TODO: Private state size should be declared by each IOFX asset, so the IOS can allocate
 * the private buffer dynamically based on the IOFX assets used with the IOS.  Each asset would
 * in turn be given an offset for their private data in this buffer.
 */
struct SpritePrivateState
{
	float	rotation;
	float	scale;

	static APEX_CUDA_CALLABLE PX_INLINE void initDefault(SpritePrivateState& state)
	{
		state.rotation = 0.0f;
		state.scale = 1.0f;
	}
};

//struct SpriteOutputLayout
#define INPLACE_TYPE_STRUCT_NAME SpriteOutputLayout
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	stride) \
	INPLACE_TYPE_FIELD_N(uint32_t,	offsets, IofxSpriteRenderLayoutElement::MAX_COUNT)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

#ifdef __CUDACC__
#define WRITE_TO_FLOAT(data) { *((volatile float*)(sdata + ((offset) >> 2) * pitch) + idx) = (data); offset += 4; }
#define WRITE_TO_UINT(data) { *((volatile unsigned int*)(sdata + ((offset) >> 2) * pitch + idx)) = (data); offset += 4; }

	__device__ PX_INLINE void write(volatile unsigned int* sdata, unsigned int idx, unsigned int pitch, const SpriteInput& input, const SpritePublicState& state, unsigned int outputID) const
	{
#else
#define WRITE_TO_FLOAT(data) { *(float*)(outputPtr + outputID * stride + offset) = (data); offset += 4; }
#define WRITE_TO_UINT(data) { *(unsigned int*)(outputPtr + outputID * stride + offset) = (data); offset += 4; }

	PX_INLINE void write(uint32_t outputID, const SpriteInput& input, const SpritePublicState& state, const uint8_t* outputPtr) const
	{
		if (outputPtr == NULL)
		{
			return;
		}
#endif
		uint32_t offset;
		if((offset = offsets[IofxSpriteRenderLayoutElement::POSITION_FLOAT3]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( input.position.x )
			WRITE_TO_FLOAT( input.position.y )
			WRITE_TO_FLOAT( input.position.z )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::COLOR_BGRA8]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_UINT( MAKE_COLOR_UBYTE4(FLT_TO_BYTE(state.color[0]), 
											FLT_TO_BYTE(state.color[1]), 
											FLT_TO_BYTE(state.color[2]), 
											FLT_TO_BYTE(state.color[3])) )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::COLOR_RGBA8]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_UINT( MAKE_COLOR_UBYTE4(FLT_TO_BYTE(state.color[2]), 
											FLT_TO_BYTE(state.color[1]), 
											FLT_TO_BYTE(state.color[0]), 
											FLT_TO_BYTE(state.color[3])) )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::COLOR_FLOAT4]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( state.color[0] )
			WRITE_TO_FLOAT( state.color[1] )
			WRITE_TO_FLOAT( state.color[2] )
			WRITE_TO_FLOAT( state.color[3] )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::VELOCITY_FLOAT3]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( input.velocity.x )
			WRITE_TO_FLOAT( input.velocity.y )
			WRITE_TO_FLOAT( input.velocity.z )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::SCALE_FLOAT2]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( state.scale.x )
			WRITE_TO_FLOAT( state.scale.y )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::LIFE_REMAIN_FLOAT1]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( input.liferemain )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::DENSITY_FLOAT1]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( input.density )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::SUBTEXTURE_FLOAT1]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( state.subTextureId )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::ORIENTATION_FLOAT1]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_FLOAT( state.rotation )
		}
		if((offset = offsets[IofxSpriteRenderLayoutElement::USER_DATA_UINT1]) != static_cast<uint32_t>(-1))
		{
			WRITE_TO_UINT( input.userData )
		}
	}
#undef WRITE_TO_UINT
#undef WRITE_TO_FLOAT
};

struct TextureOutputData
{
	uint16_t    layout;
	uint8_t     widthShift;
	uint8_t     pitchShift;
};

struct SpriteTextureOutputLayout
{
	uint32_t      textureCount;
	TextureOutputData textureData[4];
	uint8_t*      texturePtr[4];

#ifdef __CUDACC__
//#define CHECK_PTR(N) true
#define WRITE_TO_FLOAT4(N, e0, e1, e2, e3) surf2Dwrite<float4>(make_float4(e0, e1, e2, e3), KERNEL_SURF_REF(APEX_CUDA_CONCAT(Output, N)), (x << 4), y);
#define WRITE_TO_UINT(N, data) surf2Dwrite<uint32_t>(data, KERNEL_SURF_REF(APEX_CUDA_CONCAT(Output, N)), (x << 2), y);

	__device__ PX_INLINE void write(volatile unsigned int* sdata, unsigned int idx, unsigned int pitch, const SpriteInput& input, const SpritePublicState& state, unsigned int outputID) const
#else
//#define CHECK_PTR(N) (texturePtr[N] != NULL)
#define WRITE_TO_FLOAT4(N, e0, e1, e2, e3) { *(PxVec4*)(texturePtr[N] + (y << textureData[N].pitchShift) + (x << 4)) = PxVec4(e0, e1, e2, e3); }
#define WRITE_TO_UINT(N, data) { *(unsigned int*)(texturePtr[N] + (y << textureData[N].pitchShift) + (x << 2)) = data; }

	PX_INLINE void write(unsigned int outputID, const SpriteInput& input, const SpritePublicState& state, const uint8_t*) const
#endif
	{
#define WRITE_TO_TEXTURE(N) \
		if (N < textureCount) \
		{ \
			const uint32_t y = (outputID >> textureData[N].widthShift); \
			const uint32_t x = outputID - (y << textureData[N].widthShift); \
			switch (textureData[N].layout) \
			{ \
			case IofxSpriteRenderLayoutSurfaceElement::POSITION_FLOAT4: \
				WRITE_TO_FLOAT4(N, input.position.x, input.position.y, input.position.z, 1.0f ) \
				break; \
			case IofxSpriteRenderLayoutSurfaceElement::SCALE_ORIENT_SUBTEX_FLOAT4: \
				WRITE_TO_FLOAT4(N, state.scale.x, state.scale.y, state.rotation, state.subTextureId ) \
				break; \
			case IofxSpriteRenderLayoutSurfaceElement::COLOR_BGRA8: \
				WRITE_TO_UINT(N, MAKE_COLOR_UBYTE4( FLT_TO_BYTE(state.color[0]), FLT_TO_BYTE(state.color[1]), FLT_TO_BYTE(state.color[2]), FLT_TO_BYTE(state.color[3]) ) ) \
				break; \
			case IofxSpriteRenderLayoutSurfaceElement::COLOR_RGBA8: \
				WRITE_TO_UINT(N, MAKE_COLOR_UBYTE4( FLT_TO_BYTE(state.color[2]), FLT_TO_BYTE(state.color[1]), FLT_TO_BYTE(state.color[0]), FLT_TO_BYTE(state.color[3]) ) ) \
				break; \
			case IofxSpriteRenderLayoutSurfaceElement::COLOR_FLOAT4: \
				WRITE_TO_FLOAT4(N, state.color[0], state.color[1], state.color[2], state.color[3] ) \
				break; \
			} \
		}

		WRITE_TO_TEXTURE(0)
		WRITE_TO_TEXTURE(1)
		WRITE_TO_TEXTURE(2)
		WRITE_TO_TEXTURE(3)
	}
#undef WRITE_TO_TEXTURE
#undef WRITE_TO_UINT
#undef WRITE_TO_FLOAT4
//#undef CHECK_PTR
};

//struct CurvePoint
#define INPLACE_TYPE_STRUCT_NAME CurvePoint
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	x) \
	INPLACE_TYPE_FIELD(float,	y)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	APEX_CUDA_CALLABLE PX_INLINE CurvePoint() : x(0.0f), y(0.0f) {}
	APEX_CUDA_CALLABLE PX_INLINE CurvePoint(float _x, float _y) : x(_x), y(_y) {}
};

APEX_CUDA_CALLABLE PX_INLINE float lerpPoints(float x, const CurvePoint& p0, const CurvePoint& p1)
{
	return ((x - p0.x) / (p1.x - p0.x)) * (p1.y - p0.y) + p0.y;
}


//struct Curve
#define INPLACE_TYPE_STRUCT_NAME Curve
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(InplaceArray<CurvePoint>,	_pointArray)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

#ifndef __CUDACC__
	PX_INLINE void resize(InplaceStorage& storage, uint32_t numPoints)
	{
		_pointArray.resize(storage, numPoints);
	}

	PX_INLINE void setPoint(InplaceStorage& storage, const CurvePoint& point, uint32_t index)
	{
		_pointArray.updateElem(storage, point, index);
	}
#endif

	INPLACE_TEMPL_ARGS_DEF
	APEX_CUDA_CALLABLE PX_INLINE float evaluate(INPLACE_STORAGE_ARGS_DEF, float x) const
	{
		uint32_t count = _pointArray.getSize();
		if (count == 0)
		{
			return 0.0f;
		}

		CurvePoint begPoint;
		_pointArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, begPoint, 0);
		if (x <= begPoint.x)
		{
			return begPoint.y;
		}

		CurvePoint endPoint;
		_pointArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, endPoint, count - 1);
		if (x >= endPoint.x)
		{
			return endPoint.y;
		}

		//do binary search
		unsigned int beg = 0;
		unsigned int end = count;
		while (beg < end)
		{
			unsigned int mid = beg + ((end - beg) >> 1);
			CurvePoint midPoint;
			_pointArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, midPoint, mid);
			if (x < midPoint.x)
			{
				end = mid;
			}
			else
			{
				beg = mid + 1;
			}
		}
		beg = PxMin<uint32_t>(beg, count - 1);
		CurvePoint point0, point1;
		_pointArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, point0, beg - 1);
		_pointArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, point1, beg);
		return lerpPoints(x, point0, point1);
	}
};

}
} // namespace nvidia

#endif /* __MODIFIER_DATA_H__ */
