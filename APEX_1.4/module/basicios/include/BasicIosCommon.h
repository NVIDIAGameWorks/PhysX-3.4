/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __BASIC_IOS_COMMON_H__
#define __BASIC_IOS_COMMON_H__

#include "PxMat44.h"
#include "PxBounds3.h"
#include "PxVec3.h"
#include "InplaceTypes.h"

namespace nvidia
{
	namespace apex
	{
		class ApexCpuInplaceStorage;
	}
	namespace basicios
	{

//struct InjectorParams
#define INPLACE_TYPE_STRUCT_NAME InjectorParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	mLODMaxDistance) \
	INPLACE_TYPE_FIELD(float,	mLODDistanceWeight) \
	INPLACE_TYPE_FIELD(float,	mLODSpeedWeight) \
	INPLACE_TYPE_FIELD(float,	mLODLifeWeight) \
	INPLACE_TYPE_FIELD(float,	mLODBias) \
	INPLACE_TYPE_FIELD(uint32_t,	mLocalIndex)
#include INPLACE_TYPE_BUILD()

		typedef InplaceArray<InjectorParams> InjectorParamsArray;

//struct CollisionData
#define INPLACE_TYPE_STRUCT_NAME CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxVec3,	bodyCMassPosition) \
	INPLACE_TYPE_FIELD(PxVec3,	bodyLinearVelocity) \
	INPLACE_TYPE_FIELD(PxVec3,	bodyAngluarVelocity) \
	INPLACE_TYPE_FIELD(float,	materialRestitution)
#include INPLACE_TYPE_BUILD()

//struct CollisionSphereData
#define INPLACE_TYPE_STRUCT_NAME CollisionSphereData
#define INPLACE_TYPE_STRUCT_BASE CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxBounds3,		aabb) \
	INPLACE_TYPE_FIELD(PxTransform,	pose) \
	INPLACE_TYPE_FIELD(PxTransform,	inversePose) \
	INPLACE_TYPE_FIELD(float,			radius)
#include INPLACE_TYPE_BUILD()

//struct CollisionCapsuleData
#define INPLACE_TYPE_STRUCT_NAME CollisionCapsuleData
#define INPLACE_TYPE_STRUCT_BASE CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxBounds3,		aabb) \
	INPLACE_TYPE_FIELD(PxTransform,	pose) \
	INPLACE_TYPE_FIELD(PxTransform,	inversePose) \
	INPLACE_TYPE_FIELD(float,			halfHeight) \
	INPLACE_TYPE_FIELD(float,			radius)
#include INPLACE_TYPE_BUILD()

//struct CollisionBoxData
#define INPLACE_TYPE_STRUCT_NAME CollisionBoxData
#define INPLACE_TYPE_STRUCT_BASE CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxBounds3,		aabb) \
	INPLACE_TYPE_FIELD(PxTransform,	pose) \
	INPLACE_TYPE_FIELD(PxTransform,	inversePose) \
	INPLACE_TYPE_FIELD(PxVec3,			halfSize)
#include INPLACE_TYPE_BUILD()

//struct CollisionHalfSpaceData
#define INPLACE_TYPE_STRUCT_NAME CollisionHalfSpaceData
#define INPLACE_TYPE_STRUCT_BASE CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxVec3,	normal) \
	INPLACE_TYPE_FIELD(PxVec3,	origin)
#include INPLACE_TYPE_BUILD()

//struct CollisionConvexMeshData
#define INPLACE_TYPE_STRUCT_NAME CollisionConvexMeshData
#define INPLACE_TYPE_STRUCT_BASE CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxBounds3,		aabb) \
	INPLACE_TYPE_FIELD(PxTransform,	pose) \
	INPLACE_TYPE_FIELD(PxTransform,	inversePose) \
	INPLACE_TYPE_FIELD(uint32_t,			numPolygons) \
	INPLACE_TYPE_FIELD(uint32_t,			firstPlane) \
	INPLACE_TYPE_FIELD(uint32_t,			firstVertex) \
	INPLACE_TYPE_FIELD(uint32_t,			polygonsDataOffset)
#include INPLACE_TYPE_BUILD()

//struct CollisionTriMeshData
#define INPLACE_TYPE_STRUCT_NAME CollisionTriMeshData
#define INPLACE_TYPE_STRUCT_BASE CollisionData
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxBounds3,		aabb) \
	INPLACE_TYPE_FIELD(PxTransform,	pose) \
	INPLACE_TYPE_FIELD(PxTransform,	inversePose) \
	INPLACE_TYPE_FIELD(uint32_t,			numTriangles) \
	INPLACE_TYPE_FIELD(uint32_t,			firstIndex) \
	INPLACE_TYPE_FIELD(uint32_t,			firstVertex)
#include INPLACE_TYPE_BUILD()

//struct SimulationParams
#define INPLACE_TYPE_STRUCT_NAME SimulationParams
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,							collisionThreshold) \
	INPLACE_TYPE_FIELD(float,							collisionDistance) \
	INPLACE_TYPE_FIELD(InplaceArray<CollisionBoxData>,			boxes) \
	INPLACE_TYPE_FIELD(InplaceArray<CollisionSphereData>,		spheres) \
	INPLACE_TYPE_FIELD(InplaceArray<CollisionCapsuleData>,		capsules) \
	INPLACE_TYPE_FIELD(InplaceArray<CollisionHalfSpaceData>,	halfSpaces) \
	INPLACE_TYPE_FIELD(InplaceArray<CollisionConvexMeshData>,	convexMeshes) \
	INPLACE_TYPE_FIELD(InplaceArray<CollisionTriMeshData>,		trimeshes) \
	INPLACE_TYPE_FIELD(PxPlane*,						convexPlanes) \
	INPLACE_TYPE_FIELD(PxVec4*,							convexVerts) \
	INPLACE_TYPE_FIELD(uint32_t*,								convexPolygonsData) \
	INPLACE_TYPE_FIELD(PxVec4*,							trimeshVerts) \
	INPLACE_TYPE_FIELD(uint32_t*,								trimeshIndices)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	APEX_CUDA_CALLABLE PX_INLINE SimulationParams() : convexPlanes(NULL), convexVerts(NULL), convexPolygonsData(NULL) {}
	};


	struct GridDensityParams
	{
		bool Enabled;
		float GridSize;
		uint32_t GridMaxCellCount;
		uint32_t GridResolution;
		PxVec3 DensityOrigin;
		GridDensityParams(): Enabled(false) {}
	};
		
	struct GridDensityFrustumParams
	{
		float nearDimX;
		float farDimX;
		float nearDimY;
		float farDimY;
		float dimZ; 
	};

#ifdef __CUDACC__

#define SIM_FETCH_PLANE(plane, name, idx) { float4 f4 = tex1Dfetch(KERNEL_TEX_REF(name), idx); plane = PxPlane(f4.x, f4.y, f4.z, f4.w); }
#define SIM_FETCH(name, idx) tex1Dfetch(KERNEL_TEX_REF(name), idx)
#define SIM_FLOAT4 float4
#define SIM_INT_AS_FLOAT(x) __int_as_float(x)
#define SIM_INJECTOR_ARRAY const InjectorParamsArray&
#define SIM_FETCH_INJECTOR(injectorArray, injParams, injector) injectorArray.fetchElem(INPLACE_STORAGE_ARGS_VAL, injParams, injector)

	__device__ PX_INLINE float splitFloat4(PxVec3& v3, const SIM_FLOAT4& f4)
	{
		v3.x = f4.x;
		v3.y = f4.y;
		v3.z = f4.z;
		return f4.w;
	}
	__device__ PX_INLINE SIM_FLOAT4 combineFloat4(const PxVec3& v3, float w)
	{
		return make_float4(v3.x, v3.y, v3.z, w);
	}
#else

#define SIM_FETCH_PLANE(plane, name, idx) plane = mem##name[idx];
#define SIM_FETCH(name, idx) mem##name[idx]
#define SIM_FLOAT4 PxVec4
#define SIM_INT_AS_FLOAT(x) *(const float*)(&x)
#define SIM_INJECTOR_ARRAY const InjectorParams*
#define SIM_FETCH_INJECTOR(injectorArray, injParams, injector) injParams = injectorArray[injector];

	PX_INLINE float splitFloat4(PxVec3& v3, const SIM_FLOAT4& f4)
	{
		v3 = f4.getXYZ();
		return f4.w;
	}
	PX_INLINE SIM_FLOAT4 combineFloat4(const PxVec3& v3, float w)
	{
		return PxVec4(v3.x, v3.y, v3.z, w);
	}

#endif

	}
} // namespace nvidia

#endif
