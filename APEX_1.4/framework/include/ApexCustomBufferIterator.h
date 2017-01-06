/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CUSTOM_BUFFER_ITERARTOR_H__
#define __APEX_CUSTOM_BUFFER_ITERARTOR_H__

#include "CustomBufferIterator.h"
#include <PsUserAllocated.h>
#include <PsArray.h>
#include <ApexUsingNamespace.h>

namespace nvidia
{
namespace apex
{

class ApexCustomBufferIterator : public CustomBufferIterator, public UserAllocated
{
public:
	ApexCustomBufferIterator();

	// CustomBufferIterator methods

	virtual void		setData(void* data, uint32_t elemSize, uint32_t maxTriangles);

	virtual void		addCustomBuffer(const char* name, RenderDataFormat::Enum format, uint32_t offset);

	virtual void*		getVertex(uint32_t triangleIndex, uint32_t vertexIndex) const;

	virtual int32_t		getAttributeIndex(const char* attributeName) const;

	virtual void*		getVertexAttribute(uint32_t triangleIndex, uint32_t vertexIndex, const char* attributeName, RenderDataFormat::Enum& outFormat) const;

	virtual void*		getVertexAttribute(uint32_t triangleIndex, uint32_t vertexIndex, uint32_t attributeIndex, RenderDataFormat::Enum& outFormat, const char*& outName) const;

private:
	uint8_t* mData;
	uint32_t mElemSize;
	uint32_t mMaxTriangles;
	struct CustomBuffer
	{
		const char* name;
		uint32_t offset;
		RenderDataFormat::Enum format;
	};
	physx::Array<CustomBuffer> mCustomBuffers;
};

}
} // end namespace nvidia::apex


#endif // __APEX_CUSTOM_BUFFER_ITERARTOR_H__
