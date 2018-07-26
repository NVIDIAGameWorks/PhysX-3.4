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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "PsIntrinsics.h"
#include "PsUtilities.h"
#include "GuSerialize.h"
#include "PsUserAllocated.h"
#include "PsAllocator.h"
#include "PsFPU.h"

using namespace physx;
using namespace Gu;

void physx::readChunk(PxI8& a, PxI8& b, PxI8& c, PxI8& d, PxInputStream& stream)
{
	stream.read(&a, sizeof(PxI8));
	stream.read(&b, sizeof(PxI8));
	stream.read(&c, sizeof(PxI8));
	stream.read(&d, sizeof(PxI8));	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxU16 physx::readWord(bool mismatch, PxInputStream& stream)
{
	PxU16 d;
	stream.read(&d, sizeof(PxU16));
	
	if(mismatch)
		flip(d);
	return d;
}

PxU32 physx::readDword(bool mismatch, PxInputStream& stream)
{
	PxU32 d;
	stream.read(&d, sizeof(PxU32));

	if(mismatch)
		flip(d);
	return d;
}

PxF32 physx::readFloat(bool mismatch, PxInputStream& stream)
{
	union
	{
		PxU32 d;
		PxF32 f;
	} u;

	stream.read(&u.d, sizeof(PxU32));

	if(mismatch)
		flip(u.d);
	return u.f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void physx::writeWord(PxU16 value, bool mismatch, PxOutputStream& stream)
{
	if(mismatch)
		flip(value);
	stream.write(&value, sizeof(PxU16));
}

void physx::writeDword(PxU32 value, bool mismatch, PxOutputStream& stream)
{
	if(mismatch)
		flip(value);
	stream.write(&value, sizeof(PxU32));
}

void physx::writeFloat(PxF32 value, bool mismatch, PxOutputStream& stream)
{
	if(mismatch)
		flip(value);
	stream.write(&value, sizeof(PxF32));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool physx::readFloatBuffer(PxF32* dest, PxU32 nbFloats, bool mismatch, PxInputStream& stream)
{
	stream.read(dest, sizeof(PxF32)*nbFloats);
	if(mismatch)
	{
		for(PxU32 i=0;i<nbFloats;i++)
			flip(dest[i]);
	}
	return true;
}

void physx::writeFloatBuffer(const PxF32* src, PxU32 nb, bool mismatch, PxOutputStream& stream)
{
	while(nb--)
	{
		PxF32 f = *src++;
		if(mismatch)
			flip(f);
		stream.write(&f, sizeof(PxF32));
	}
}

void physx::writeWordBuffer(const PxU16* src, PxU32 nb, bool mismatch, PxOutputStream& stream)
{
	while(nb--)
	{
		PxU16 w = *src++;
		if(mismatch)
			flip(w);
		stream.write(&w, sizeof(PxU16));
	}
}

void physx::readWordBuffer(PxU16* dest, PxU32 nb, bool mismatch, PxInputStream& stream)
{
	stream.read(dest, sizeof(PxU16)*nb);
	if(mismatch)
	{
		for(PxU32 i=0;i<nb;i++)
		{
			flip(dest[i]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool physx::writeHeader(PxI8 a, PxI8 b, PxI8 c, PxI8 d, PxU32 version, bool mismatch, PxOutputStream& stream)
{
	// Store endianness
	PxI8 streamFlags = Ps::littleEndian();
	if(mismatch)
		streamFlags^=1;

	// Export header
	writeChunk('N', 'X', 'S', streamFlags, stream);	// "Novodex stream" identifier
	writeChunk(a, b, c, d, stream);					// Chunk identifier
	writeDword(version, mismatch, stream);
	return true;
}

bool Gu::WriteHeader(PxU8 a, PxU8 b, PxU8 c, PxU8 d, PxU32 version, bool mismatch, PxOutputStream& stream)
{
	// Store endianness
	PxU8 streamFlags = PxU8(Ps::littleEndian());
	if(mismatch)
		streamFlags^=1;

	// Export header
	writeChunk('I', 'C', 'E', PxI8(streamFlags), stream);	// ICE identifier
	writeChunk(PxI8(a), PxI8(b), PxI8(c), PxI8(d), stream);	// Chunk identifier
	writeDword(version, mismatch, stream);
	return true;
}

bool physx::readHeader(PxI8 a_, PxI8 b_, PxI8 c_, PxI8 d_, PxU32& version, bool& mismatch, PxInputStream& stream)
{
	// Import header
	PxI8 a, b, c, d;
	readChunk(a, b, c, d, stream);
	if(a!='N' || b!='X' || c!='S')
		return false;

	const PxI8 fileLittleEndian = d&1;
	mismatch = fileLittleEndian!=Ps::littleEndian();

	readChunk(a, b, c, d, stream);
	if(a!=a_ || b!=b_ || c!=c_ || d!=d_)
		return false;

	version = readDword(mismatch, stream);
	return true;
}

bool Gu::ReadHeader(PxU8 a_, PxU8 b_, PxU8 c_, PxU8 d_, PxU32& version, bool& mismatch, PxInputStream& stream)
{
	// Import header
	PxI8 a, b, c, d;
	readChunk(a, b, c, d, stream);
	if(a!='I' || b!='C' || c!='E')
		return false;

	const PxU8 FileLittleEndian = PxU8(d&1);
	mismatch = FileLittleEndian!=Ps::littleEndian();

	readChunk(a, b, c, d, stream);
	if(a!=a_ || b!=b_ || c!=c_ || d!=d_)
		return false;

	version = readDword(mismatch, stream);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PxU32 physx::computeMaxIndex(const PxU32* indices, PxU32 nbIndices)
{
	PxU32 maxIndex=0;
	while(nbIndices--)
	{
		PxU32 currentIndex = *indices++;
		if(currentIndex>maxIndex)
			maxIndex = currentIndex;
	}
	return maxIndex;
}
PxU16 physx::computeMaxIndex(const PxU16* indices, PxU32 nbIndices)
{
	PxU16 maxIndex=0;
	while(nbIndices--)
	{
		PxU16 currentIndex = *indices++;
		if(currentIndex>maxIndex)
			maxIndex = currentIndex;
	}
	return maxIndex;
}

void physx::storeIndices(PxU32 maxIndex, PxU32 nbIndices, const PxU32* indices, PxOutputStream& stream, bool platformMismatch)
{
	if(maxIndex<=0xff)
	{
		for(PxU32 i=0;i<nbIndices;i++)
		{
			PxU8 data = PxU8(indices[i]);
			stream.write(&data, sizeof(PxU8));	
		}
	}
	else if(maxIndex<=0xffff)
	{
		for(PxU32 i=0;i<nbIndices;i++)
			writeWord(Ps::to16(indices[i]), platformMismatch, stream);
	}
	else
	{
		writeIntBuffer(indices, nbIndices, platformMismatch, stream);
	}
}

void physx::readIndices(PxU32 maxIndex, PxU32 nbIndices, PxU32* indices, PxInputStream& stream, bool platformMismatch)
{
	if(maxIndex<=0xff)
	{
		PxU8 data;
		for(PxU32 i=0;i<nbIndices;i++)
		{
			stream.read(&data, sizeof(PxU8));
			indices[i] = data;
		}
	}
	else if(maxIndex<=0xffff)
	{
		for(PxU32 i=0;i<nbIndices;i++)
			indices[i] = readWord(platformMismatch, stream);
	}
	else
	{
		readIntBuffer(indices, nbIndices, platformMismatch, stream);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Gu::StoreIndices(PxU32 maxIndex, PxU32 nbIndices, const PxU32* indices, PxOutputStream& stream, bool platformMismatch)
{
	if(maxIndex<=0xff)
	{
		for(PxU32 i=0;i<nbIndices;i++)
		{
			PxU8 data = PxU8(indices[i]);
			stream.write(&data, sizeof(PxU8));	
		}
	}
	else if(maxIndex<=0xffff)
	{
		for(PxU32 i=0;i<nbIndices;i++)
			writeWord(Ps::to16(indices[i]), platformMismatch, stream);
	}
	else
	{
//		WriteDwordBuffer(indices, nbIndices, platformMismatch, stream);
		for(PxU32 i=0;i<nbIndices;i++)
			writeDword(indices[i], platformMismatch, stream);
	}
}

void Gu::ReadIndices(PxU32 maxIndex, PxU32 nbIndices, PxU32* indices, PxInputStream& stream, bool platformMismatch)
{
	if(maxIndex<=0xff)
	{
		PxU8* tmp = reinterpret_cast<PxU8*>(PxAlloca(nbIndices*sizeof(PxU8)));
		stream.read(tmp, nbIndices*sizeof(PxU8));
		for(PxU32 i=0;i<nbIndices;i++)
			indices[i] = tmp[i];
//		for(PxU32 i=0;i<nbIndices;i++)
//			indices[i] = stream.ReadByte();
	}
	else if(maxIndex<=0xffff)
	{
		PxU16* tmp = reinterpret_cast<PxU16*>(PxAlloca(nbIndices*sizeof(PxU16)));
		readWordBuffer(tmp, nbIndices, platformMismatch, stream);
		for(PxU32 i=0;i<nbIndices;i++)
			indices[i] = tmp[i];
//		for(PxU32 i=0;i<nbIndices;i++)
//			indices[i] = ReadWord(platformMismatch, stream);
	}
	else
	{
		ReadDwordBuffer(indices, nbIndices, platformMismatch, stream);
	}
}

void Gu::StoreIndices(PxU16 maxIndex, PxU32 nbIndices, const PxU16* indices, PxOutputStream& stream, bool platformMismatch)
{
	if(maxIndex<=0xff)
	{
		for(PxU32 i=0;i<nbIndices;i++)
		{
			PxU8 data = PxU8(indices[i]);
			stream.write(&data, sizeof(PxU8));	
		}
	}
	else
	{
		for(PxU32 i=0;i<nbIndices;i++)
			writeWord(indices[i], platformMismatch, stream);
	}
}

void Gu::ReadIndices(PxU16 maxIndex, PxU32 nbIndices, PxU16* indices, PxInputStream& stream, bool platformMismatch)
{
	if(maxIndex<=0xff)
	{
		PxU8* tmp = reinterpret_cast<PxU8*>(PxAlloca(nbIndices*sizeof(PxU8)));
		stream.read(tmp, nbIndices*sizeof(PxU8));
		for(PxU32 i=0;i<nbIndices;i++)
			indices[i] = tmp[i];
	}
	else
	{
		readWordBuffer(indices, nbIndices, platformMismatch, stream);
	}
}
