/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexCustomBufferIterator.h"
#include "PsString.h"

namespace nvidia
{
namespace apex
{

ApexCustomBufferIterator::ApexCustomBufferIterator() :
	mData(NULL),
	mElemSize(0),
	mMaxTriangles(0)
{
}

void ApexCustomBufferIterator::setData(void* data, uint32_t elemSize, uint32_t maxTriangles)
{
	mData = (uint8_t*)data;
	mElemSize = elemSize;
	mMaxTriangles = maxTriangles;
}

void ApexCustomBufferIterator::addCustomBuffer(const char* name, RenderDataFormat::Enum format, uint32_t offset)
{
	CustomBuffer buffer;
	buffer.name = name;
	buffer.offset = offset;
	buffer.format = format;

	mCustomBuffers.pushBack(buffer);
}
void* ApexCustomBufferIterator::getVertex(uint32_t triangleIndex, uint32_t vertexIndex) const
{
	if (mData == NULL || triangleIndex >= mMaxTriangles)
	{
		return NULL;
	}

	return mData + mElemSize * (triangleIndex * 3 + vertexIndex);
}
int32_t ApexCustomBufferIterator::getAttributeIndex(const char* attributeName) const
{
	if (attributeName == NULL || attributeName[0] == 0)
	{
		return -1;
	}

	for (uint32_t i = 0; i < mCustomBuffers.size(); i++)
	{
		if (nvidia::strcmp(mCustomBuffers[i].name, attributeName) == 0)
		{
			return (int32_t)i;
		}
	}
	return -1;
}
void* ApexCustomBufferIterator::getVertexAttribute(uint32_t triangleIndex, uint32_t vertexIndex, const char* attributeName, RenderDataFormat::Enum& outFormat) const
{
	outFormat = RenderDataFormat::UNSPECIFIED;

	uint8_t* elementData = (uint8_t*)getVertex(triangleIndex, vertexIndex);
	if (elementData == NULL)
	{
		return NULL;
	}


	for (uint32_t i = 0; i < mCustomBuffers.size(); i++)
	{
		if (nvidia::strcmp(mCustomBuffers[i].name, attributeName) == 0)
		{
			outFormat = mCustomBuffers[i].format;
			return elementData + mCustomBuffers[i].offset;
		}
	}
	return NULL;
}

void* ApexCustomBufferIterator::getVertexAttribute(uint32_t triangleIndex, uint32_t vertexIndex, uint32_t attributeIndex, RenderDataFormat::Enum& outFormat, const char*& outName) const
{
	outFormat = RenderDataFormat::UNSPECIFIED;
	outName = NULL;

	uint8_t* elementData = (uint8_t*)getVertex(triangleIndex, vertexIndex);
	if (elementData == NULL || attributeIndex >= mCustomBuffers.size())
	{
		return NULL;
	}

	outName = mCustomBuffers[attributeIndex].name;
	outFormat = mCustomBuffers[attributeIndex].format;
	return elementData + mCustomBuffers[attributeIndex].offset;
}

}
} // end namespace nvidia::apex
