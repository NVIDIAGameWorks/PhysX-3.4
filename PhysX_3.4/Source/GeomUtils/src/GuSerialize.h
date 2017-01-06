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
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_SERIALIZE_H
#define GU_SERIALIZE_H

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxIO.h"
#include "CmPhysXCommon.h"
#include "PxPhysXCommonConfig.h"

namespace physx
{
	PX_INLINE void flip(PxU16& v)
	{
		PxU8* b = reinterpret_cast<PxU8*>(&v);
		PxU8 temp = b[0];
		b[0] = b[1];
		b[1] = temp;
	}

	PX_INLINE void flip(PxI16& v)
	{
		PxI8* b = reinterpret_cast<PxI8*>(&v);
		PxI8 temp = b[0];
		b[0] = b[1];
		b[1] = temp;
	}

	PX_INLINE void flip(PxU32& v)
	{
		PxU8* b = reinterpret_cast<PxU8*>(&v);

        PxU8 temp = b[0];
		b[0] = b[3];
		b[3] = temp;
		temp = b[1];
		b[1] = b[2];
		b[2] = temp;
	}

	// MS: It is important to modify the value directly and not use a temporary variable or a return
	//     value. The reason for this is that a flipped float might have a bit pattern which indicates
	//     an invalid float. If such a float is assigned to another float, the bit pattern
	//     can change again (maybe to map invalid floats to a common invalid pattern?).
	//     When reading the float and flipping again, the changed bit pattern will result in a different
	//     float than the original one.
	PX_INLINE void flip(PxF32& v)
	{
		PxU8* b = reinterpret_cast<PxU8*>(&v);

        PxU8 temp = b[0];
		b[0] = b[3];
		b[3] = temp;
		temp = b[1];
		b[1] = b[2];
		b[2] = temp;
	}

	PX_INLINE void 	writeChunk(PxI8 a, PxI8 b, PxI8 c, PxI8 d, PxOutputStream& stream)
	{
		stream.write(&a, sizeof(PxI8));
		stream.write(&b, sizeof(PxI8));
		stream.write(&c, sizeof(PxI8));
		stream.write(&d, sizeof(PxI8));
	}

					void 	readChunk(PxI8& a, PxI8& b, PxI8& c, PxI8& d, PxInputStream& stream);

					PxU16 	readWord(bool mismatch, PxInputStream& stream);
PX_PHYSX_COMMON_API	PxU32 	readDword(bool mismatch, PxInputStream& stream);
					PxF32 	readFloat(bool mismatch, PxInputStream& stream);

PX_PHYSX_COMMON_API	void 	writeWord(PxU16 value, bool mismatch, PxOutputStream& stream);
PX_PHYSX_COMMON_API	void 	writeDword(PxU32 value, bool mismatch, PxOutputStream& stream);
PX_PHYSX_COMMON_API	void 	writeFloat(PxF32 value, bool mismatch, PxOutputStream& stream);

					bool 	readFloatBuffer(PxF32* dest, PxU32 nbFloats, bool mismatch, PxInputStream& stream);
PX_PHYSX_COMMON_API	void 	writeFloatBuffer(const PxF32* src, PxU32 nb, bool mismatch, PxOutputStream& stream);
PX_PHYSX_COMMON_API	void 	writeWordBuffer(const PxU16* src, PxU32 nb, bool mismatch, PxOutputStream& stream);
					void	readWordBuffer(PxU16* dest, PxU32 nb, bool mismatch, PxInputStream& stream);

PX_PHYSX_COMMON_API	bool 	writeHeader(PxI8 a, PxI8 b, PxI8 c, PxI8 d, PxU32 version, bool mismatch, PxOutputStream& stream);
					bool 	readHeader(PxI8 a, PxI8 b, PxI8 c, PxI8 d, PxU32& version, bool& mismatch, PxInputStream& stream);

PX_INLINE	bool	readIntBuffer(PxU32* dest, PxU32 nbInts, bool mismatch, PxInputStream& stream)
{
	return readFloatBuffer(reinterpret_cast<PxF32*>(dest), nbInts, mismatch, stream);
}

PX_INLINE	void	writeIntBuffer(const PxU32* src, PxU32 nb, bool mismatch, PxOutputStream& stream)
{
	writeFloatBuffer(reinterpret_cast<const PxF32*>(src), nb, mismatch, stream);
}

PX_INLINE	bool	ReadDwordBuffer(PxU32* dest, PxU32 nb, bool mismatch, PxInputStream& stream)
{
	return readFloatBuffer(reinterpret_cast<float*>(dest), nb, mismatch, stream);
}

PX_INLINE	void	WriteDwordBuffer(const PxU32* src, PxU32 nb, bool mismatch, PxOutputStream& stream)
{
	writeFloatBuffer(reinterpret_cast<const float*>(src), nb, mismatch, stream);
}

PX_PHYSX_COMMON_API PxU32 	computeMaxIndex(const PxU32* indices, PxU32 nbIndices);
PX_PHYSX_COMMON_API PxU16 	computeMaxIndex(const PxU16* indices, PxU32 nbIndices);
PX_PHYSX_COMMON_API void 	storeIndices(PxU32 maxIndex, PxU32 nbIndices, const PxU32* indices, PxOutputStream& stream, bool platformMismatch);
PX_PHYSX_COMMON_API void 	readIndices(PxU32 maxIndex, PxU32 nbIndices, PxU32* indices, PxInputStream& stream, bool platformMismatch);


// PT: TODO: copied from IceSerialize.h, still needs to be refactored/cleaned up.
namespace Gu
{
	PX_PHYSX_COMMON_API bool WriteHeader(PxU8 a, PxU8 b, PxU8 c, PxU8 d, PxU32 version, bool mismatch, PxOutputStream& stream);
	PX_PHYSX_COMMON_API bool ReadHeader(PxU8 a_, PxU8 b_, PxU8 c_, PxU8 d_, PxU32& version, bool& mismatch, PxInputStream& stream);

	PX_PHYSX_COMMON_API void StoreIndices(PxU32 maxIndex, PxU32 nbIndices, const PxU32* indices, PxOutputStream& stream, bool platformMismatch);
						void ReadIndices(PxU32 maxIndex, PxU32 nbIndices, PxU32* indices, PxInputStream& stream, bool platformMismatch);

	PX_PHYSX_COMMON_API void StoreIndices(PxU16 maxIndex, PxU32 nbIndices, const PxU16* indices, PxOutputStream& stream, bool platformMismatch);
						void ReadIndices(PxU16 maxIndex, PxU32 nbIndices, PxU16* indices, PxInputStream& stream, bool platformMismatch);
}


}

#endif
