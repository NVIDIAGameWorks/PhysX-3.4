// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

PX_INLINE uint32_t PlatformABI::align(uint32_t len, uint32_t border)
{
	uint32_t n = (len + border - 1) / border;
	return n * border;
}

PX_INLINE bool PlatformABI::isNormal() const
{
	return 1 == sizes.Char && 1 == sizes.Bool				//Wide (> 1) bytes not supported
		&& 4 == sizes.real									//Some code relies on short float (float)
		&& ( 4 == sizes.pointer || 8 == sizes.pointer );	//Some code relies on pointers being either 32- or 64-bit
}

PX_INLINE uint32_t PlatformABI::getMetaEntryAlignment() const
{
	return physx::PxMax(aligns.i32, aligns.pointer);
}

PX_INLINE uint32_t PlatformABI::getMetaInfoAlignment() const
{
	return NvMax3(getHintAlignment(), aligns.i32, aligns.pointer);
}

PX_INLINE uint32_t PlatformABI::getHintAlignment() const
{
	return physx::PxMax(aligns.i32, getHintValueAlignment());
}

PX_INLINE uint32_t PlatformABI::getHintValueAlignment() const
{
	return NvMax3(aligns.pointer, aligns.i64, aligns.f64);
}

PX_INLINE uint32_t PlatformABI::getHintValueSize() const
{
	return align(8, getHintValueAlignment()); //Size of union = aligned size of maximum element
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<bool>() const
{
	return aligns.Bool;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<int8_t>() const
{
	return aligns.i8;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<int16_t>() const
{
	return aligns.i16;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<int32_t>() const
{
	return aligns.i32;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<int64_t>() const
{
	return aligns.i64;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<uint8_t>() const
{
	return getAlignment<int8_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<uint16_t>() const
{
	return getAlignment<int16_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<uint32_t>() const
{
	return getAlignment<int32_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<uint64_t>() const
{
	return getAlignment<int64_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<float>() const
{
	return aligns.f32;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<double>() const
{
	return aligns.f64;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxVec2>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxVec3>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxVec4>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxQuat>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxMat33>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxMat44>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxBounds3>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<void *>() const
{
	return aligns.pointer;
}

template <> PX_INLINE uint32_t PlatformABI::getAlignment<physx::PxTransform>() const
{
	return aligns.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<bool>() const
{
	return sizes.Bool;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<int8_t>() const
{
	return 1;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<int16_t>() const
{
	return 2;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<int32_t>() const
{
	return 4;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<int64_t>() const
{
	return 8;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<uint8_t>() const
{
	return getSize<int8_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getSize<uint16_t>() const
{
	return getSize<int16_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getSize<uint32_t>() const
{
	return getSize<int32_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getSize<uint64_t>() const
{
	return getSize<int64_t>();
}

template <> PX_INLINE uint32_t PlatformABI::getSize<float>() const
{
	return 4;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<double>() const
{
	return 8;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxVec2>() const
{
	return 2 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxVec3>() const
{
	return 3 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxVec4>() const
{
	return 4 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxQuat>() const
{
	return 4 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxMat33>() const
{
	return 9 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxMat44>() const
{
	return 16 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxBounds3>() const
{
	return 6 * sizes.real;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<void *>() const
{
	return sizes.pointer;
}

template <> PX_INLINE uint32_t PlatformABI::getSize<physx::PxTransform>() const
{
	return 7 * sizes.real;
}
