/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONVERTEXBUFFERPARAMETERS_0P0_0P1H_H
#define MODULE_CONVERSIONVERTEXBUFFERPARAMETERS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "VertexBufferParameters_0p0.h"
#include "VertexBufferParameters_0p1.h"

#include "VertexFormatParameters.h"

#include <BufferF32x1.h>
#include <BufferU16x1.h>

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::VertexBufferParameters_0p0, 
						nvidia::parameterized::VertexBufferParameters_0p1, 
						nvidia::parameterized::VertexBufferParameters_0p0::ClassVersion, 
						nvidia::parameterized::VertexBufferParameters_0p1::ClassVersion>
						ConversionVertexBufferParameters_0p0_0p1Parent;

class ConversionVertexBufferParameters_0p0_0p1: public ConversionVertexBufferParameters_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionVertexBufferParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionVertexBufferParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionVertexBufferParameters_0p0_0p1(NvParameterized::Traits* t) : ConversionVertexBufferParameters_0p0_0p1Parent(t) {}

	const NvParameterized::PrefVer* getPreferredVersions() const
	{
		static NvParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char *)longName, (uint32_t)preferredVersion }

			{ "vertexFormat", 0 },
			{ 0, 0 } // Terminator (do not remove!)
		};

		return prefVers;
	}

	bool convert()
	{
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData has already been initialized with default values
		//		- same-named/same-typed members have already been copied
		//			from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.



		// if one of these fails it means that they have been upgraded. Then the proper (_0p0.h) files need to be included
		// and the types also need the _0p0 suffix. Then it should work again.
		PX_COMPILE_TIME_ASSERT(VertexFormatParameters::ClassVersion == 0);
		PX_COMPILE_TIME_ASSERT(BufferF32x1::ClassVersion == 0);
		PX_COMPILE_TIME_ASSERT(BufferU16x1::ClassVersion == 0);


		VertexFormatParameters* format = static_cast<VertexFormatParameters*>(mNewData->vertexFormat);

		int32_t boneWeightID = -1;
		int32_t boneIndexID = -1;
		uint32_t numBoneWeights = 0;
		uint32_t numBoneIndices = 0;

		for (uint32_t formatID = 0; formatID < (uint32_t)format->bufferFormats.arraySizes[0]; formatID++)
		{
			const uint32_t semantic = (uint32_t)format->bufferFormats.buf[formatID].semantic;
			if (semantic == RenderVertexSemantic::BONE_INDEX)
			{
				boneIndexID = (int32_t)formatID;
				switch (format->bufferFormats.buf[formatID].format)
				{
				case RenderDataFormat::USHORT1:	
					numBoneIndices = 1;
					break;
				case RenderDataFormat::USHORT2:
					numBoneIndices = 2;
					break;
				case RenderDataFormat::USHORT3:
					numBoneIndices = 3;
					break;
				case RenderDataFormat::USHORT4:
					numBoneIndices = 4;
					break;
				}
			}
			else if (semantic == RenderVertexSemantic::BONE_WEIGHT)
			{
				boneWeightID = (int32_t)formatID;
				switch (format->bufferFormats.buf[formatID].format)
				{
				case RenderDataFormat::FLOAT1:
					numBoneWeights = 1;
					break;
				case RenderDataFormat::FLOAT2:
					numBoneWeights = 2;
					break;
				case RenderDataFormat::FLOAT3:
					numBoneWeights = 3;
					break;
				case RenderDataFormat::FLOAT4:
					numBoneWeights = 4;
					break;
				}
			}
		}


		// sort bone weights
		if (numBoneIndices > 1 && numBoneWeights == numBoneIndices)
		{
			float* boneWeightBuffer = static_cast<BufferF32x1*>(mNewData->buffers.buf[boneWeightID])->data.buf;
			uint16_t* boneIndexBuffer = static_cast<BufferU16x1*>(mNewData->buffers.buf[boneIndexID])->data.buf;

			for (uint32_t vi = 0; vi < mNewData->vertexCount; vi++)
			{
				float* verifyWeights = boneWeightBuffer + vi * numBoneWeights;
				uint16_t* verifyIndices = boneIndexBuffer + vi * numBoneWeights;

				float sum = 0.0f;
				for (uint32_t j = 0; j < numBoneWeights; j++)
				{
					sum += verifyWeights[j];
				}

				if (physx::PxAbs(1 - sum) > 0.001)
				{
					if (sum > 0.0f)
					{
						for (uint32_t j = 0; j < numBoneWeights; j++)
						{
							verifyWeights[j] /= sum;
						}
					}
				}

				// PH: bubble sort, don't kill me for this
				for (uint32_t j = 1; j < numBoneWeights; j++)
				{
					for (uint32_t k = 1; k < numBoneWeights; k++)
					{
						if (verifyWeights[k - 1] < verifyWeights[k])
						{
							nvidia::swap(verifyWeights[k - 1], verifyWeights[k]);
							nvidia::swap(verifyIndices[k - 1], verifyIndices[k]);
						}
					}
				}
			}
		}

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
