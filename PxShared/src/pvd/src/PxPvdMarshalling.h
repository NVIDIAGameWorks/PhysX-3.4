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
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.

#ifndef PXPVDSDK_PXPVDMARSHALLING_H
#define PXPVDSDK_PXPVDMARSHALLING_H

#include "foundation/PxIntrinsics.h"

#include "PxPvdObjectModelBaseTypes.h"
#include "PxPvdBits.h"

namespace physx
{
namespace pvdsdk
{

// Define marshalling

template <typename TSmallerType, typename TLargerType>
struct PvdMarshalling
{
	bool canMarshal;
	PvdMarshalling() : canMarshal(false)
	{
	}
};

template <typename smtype, typename lgtype>
static inline void marshalSingleT(const uint8_t* srcData, uint8_t* destData)
{
	smtype incoming;

	physx::intrinsics::memCopy(&incoming, srcData, sizeof(smtype));
	lgtype outgoing = static_cast<lgtype>(incoming);
	physx::intrinsics::memCopy(destData, &outgoing, sizeof(lgtype));
}

template <typename smtype, typename lgtype>
static inline void marshalBlockT(const uint8_t* srcData, uint8_t* destData, uint32_t numBytes)
{
	for(const uint8_t* item = srcData, *end = srcData + numBytes; item < end;
	    item += sizeof(smtype), destData += sizeof(lgtype))
		marshalSingleT<smtype, lgtype>(item, destData);
}

#define PVD_TYPE_MARSHALLER(smtype, lgtype)                                                                            \
	template <>                                                                                                        \
	struct PvdMarshalling<smtype, lgtype>                                                                              \
	{                                                                                                                  \
		uint32_t canMarshal;                                                                                           \
		static void marshalSingle(const uint8_t* srcData, uint8_t* destData)                                           \
		{                                                                                                              \
			marshalSingleT<smtype, lgtype>(srcData, destData);                                                         \
		}                                                                                                              \
		static void marshalBlock(const uint8_t* srcData, uint8_t* destData, uint32_t numBytes)                         \
		{                                                                                                              \
			marshalBlockT<smtype, lgtype>(srcData, destData, numBytes);                                                \
		}                                                                                                              \
	};

// define marshalling tables.
PVD_TYPE_MARSHALLER(int8_t, int16_t)
PVD_TYPE_MARSHALLER(int8_t, uint16_t)
PVD_TYPE_MARSHALLER(int8_t, int32_t)
PVD_TYPE_MARSHALLER(int8_t, uint32_t)
PVD_TYPE_MARSHALLER(int8_t, int64_t)
PVD_TYPE_MARSHALLER(int8_t, uint64_t)
PVD_TYPE_MARSHALLER(int8_t, PvdF32)
PVD_TYPE_MARSHALLER(int8_t, PvdF64)

PVD_TYPE_MARSHALLER(uint8_t, int16_t)
PVD_TYPE_MARSHALLER(uint8_t, uint16_t)
PVD_TYPE_MARSHALLER(uint8_t, int32_t)
PVD_TYPE_MARSHALLER(uint8_t, uint32_t)
PVD_TYPE_MARSHALLER(uint8_t, int64_t)
PVD_TYPE_MARSHALLER(uint8_t, uint64_t)
PVD_TYPE_MARSHALLER(uint8_t, PvdF32)
PVD_TYPE_MARSHALLER(uint8_t, PvdF64)

PVD_TYPE_MARSHALLER(int16_t, int32_t)
PVD_TYPE_MARSHALLER(int16_t, uint32_t)
PVD_TYPE_MARSHALLER(int16_t, int64_t)
PVD_TYPE_MARSHALLER(int16_t, uint64_t)
PVD_TYPE_MARSHALLER(int16_t, PvdF32)
PVD_TYPE_MARSHALLER(int16_t, PvdF64)

PVD_TYPE_MARSHALLER(uint16_t, int32_t)
PVD_TYPE_MARSHALLER(uint16_t, uint32_t)
PVD_TYPE_MARSHALLER(uint16_t, int64_t)
PVD_TYPE_MARSHALLER(uint16_t, uint64_t)
PVD_TYPE_MARSHALLER(uint16_t, PvdF32)
PVD_TYPE_MARSHALLER(uint16_t, PvdF64)

PVD_TYPE_MARSHALLER(int32_t, int64_t)
PVD_TYPE_MARSHALLER(int32_t, uint64_t)
PVD_TYPE_MARSHALLER(int32_t, PvdF64)
PVD_TYPE_MARSHALLER(int32_t, PvdF32)

PVD_TYPE_MARSHALLER(uint32_t, int64_t)
PVD_TYPE_MARSHALLER(uint32_t, uint64_t)
PVD_TYPE_MARSHALLER(uint32_t, PvdF64)
PVD_TYPE_MARSHALLER(uint32_t, PvdF32)

PVD_TYPE_MARSHALLER(PvdF32, PvdF64)
PVD_TYPE_MARSHALLER(PvdF32, uint32_t)
PVD_TYPE_MARSHALLER(PvdF32, int32_t)

PVD_TYPE_MARSHALLER(uint64_t, PvdF64)
PVD_TYPE_MARSHALLER(int64_t, PvdF64)
PVD_TYPE_MARSHALLER(PvdF64, uint64_t)
PVD_TYPE_MARSHALLER(PvdF64, int64_t)

template <typename TMarshaller>
static inline bool getMarshalOperators(TSingleMarshaller&, TBlockMarshaller&, TMarshaller&, bool)
{
	return false;
}

template <typename TMarshaller>
static inline bool getMarshalOperators(TSingleMarshaller& single, TBlockMarshaller& block, TMarshaller&, uint32_t)
{
	single = TMarshaller::marshalSingle;
	block = TMarshaller::marshalBlock;
	return true;
}

template <typename smtype, typename lgtype>
static inline bool getMarshalOperators(TSingleMarshaller& single, TBlockMarshaller& block)
{
	single = NULL;
	block = NULL;
	PvdMarshalling<smtype, lgtype> marshaller = PvdMarshalling<smtype, lgtype>();
	return getMarshalOperators(single, block, marshaller, marshaller.canMarshal);
}

template <typename smtype>
static inline bool getMarshalOperators(TSingleMarshaller& single, TBlockMarshaller& block, int32_t lgtypeId)
{
	switch(lgtypeId)
	{
	case PvdBaseType::PvdI8: // int8_t:
		return getMarshalOperators<smtype, int8_t>(single, block);
	case PvdBaseType::PvdU8: // uint8_t:
		return getMarshalOperators<smtype, uint8_t>(single, block);
	case PvdBaseType::PvdI16: // int16_t:
		return getMarshalOperators<smtype, int16_t>(single, block);
	case PvdBaseType::PvdU16: // uint16_t:
		return getMarshalOperators<smtype, uint16_t>(single, block);
	case PvdBaseType::PvdI32: // int32_t:
		return getMarshalOperators<smtype, int32_t>(single, block);
	case PvdBaseType::PvdU32: // uint32_t:
		return getMarshalOperators<smtype, uint32_t>(single, block);
	case PvdBaseType::PvdI64: // int64_t:
		return getMarshalOperators<smtype, int64_t>(single, block);
	case PvdBaseType::PvdU64: // uint64_t:
		return getMarshalOperators<smtype, uint64_t>(single, block);
	case PvdBaseType::PvdF32:
		return getMarshalOperators<smtype, PvdF32>(single, block);
	case PvdBaseType::PvdF64:
		return getMarshalOperators<smtype, PvdF64>(single, block);
	}
	return false;
}

static inline bool getMarshalOperators(TSingleMarshaller& single, TBlockMarshaller& block, int32_t smtypeId,
                                       int32_t lgtypeId)
{
	switch(smtypeId)
	{
	case PvdBaseType::PvdI8: // int8_t:
		return getMarshalOperators<int8_t>(single, block, lgtypeId);
	case PvdBaseType::PvdU8: // uint8_t:
		return getMarshalOperators<uint8_t>(single, block, lgtypeId);
	case PvdBaseType::PvdI16: // int16_t:
		return getMarshalOperators<int16_t>(single, block, lgtypeId);
	case PvdBaseType::PvdU16: // uint16_t:
		return getMarshalOperators<uint16_t>(single, block, lgtypeId);
	case PvdBaseType::PvdI32: // int32_t:
		return getMarshalOperators<int32_t>(single, block, lgtypeId);
	case PvdBaseType::PvdU32: // uint32_t:
		return getMarshalOperators<uint32_t>(single, block, lgtypeId);
	case PvdBaseType::PvdI64: // int64_t:
		return getMarshalOperators<int64_t>(single, block, lgtypeId);
	case PvdBaseType::PvdU64: // uint64_t:
		return getMarshalOperators<uint64_t>(single, block, lgtypeId);
	case PvdBaseType::PvdF32:
		return getMarshalOperators<PvdF32>(single, block, lgtypeId);
	case PvdBaseType::PvdF64:
		return getMarshalOperators<PvdF64>(single, block, lgtypeId);
	}
	return false;
}
}
}

#endif // PXPVDSDK_PXPVDMARSHALLING_H
