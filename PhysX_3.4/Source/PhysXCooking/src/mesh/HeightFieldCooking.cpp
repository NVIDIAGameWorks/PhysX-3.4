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

#include "foundation/PxIO.h"
#include "GuHeightField.h"
#include "GuSerialize.h"

using namespace physx;
using namespace Gu;

namespace physx
{

bool saveHeightField(const HeightField& hf, PxOutputStream& stream, bool endian)
{
	// write header
	if(!writeHeader('H', 'F', 'H', 'F', PX_HEIGHTFIELD_VERSION, endian, stream))
		return false;

	const Gu::HeightFieldData& hfData = hf.getData();

	// write mData members
	writeDword(hfData.rows, endian, stream);
	writeDword(hfData.columns, endian, stream);
	writeFloat(hfData.rowLimit, endian, stream);
	writeFloat(hfData.colLimit, endian, stream);
	writeFloat(hfData.nbColumns, endian, stream);
	writeFloat(hfData.thickness, endian, stream);
	writeFloat(hfData.convexEdgeThreshold, endian, stream);
	writeWord(hfData.flags, endian, stream);
	writeDword(hfData.format, endian, stream);

	writeFloat(hfData.mAABB.getMin(0), endian, stream);
	writeFloat(hfData.mAABB.getMin(1), endian, stream);
	writeFloat(hfData.mAABB.getMin(2), endian, stream);
	writeFloat(hfData.mAABB.getMax(0), endian, stream);
	writeFloat(hfData.mAABB.getMax(1), endian, stream);
	writeFloat(hfData.mAABB.getMax(2), endian, stream);

	// write this-> members
	writeDword(hf.mSampleStride, endian, stream);
	writeDword(hf.mNbSamples, endian, stream);
	writeFloat(hf.mMinHeight, endian, stream);
	writeFloat(hf.mMaxHeight, endian, stream);

	// write samples
	for(PxU32 i = 0; i < hf.mNbSamples; i++)
	{
		const PxHeightFieldSample& s = hfData.samples[i];
		writeWord(PxU16(s.height), endian, stream);
		stream.write(&s.materialIndex0, sizeof(s.materialIndex0));
		stream.write(&s.materialIndex1, sizeof(s.materialIndex1));
	}

	return true;
}

}
