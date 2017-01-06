/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_VERTEX_BUFFER_H
#define APEX_VERTEX_BUFFER_H

#include "RenderMeshAssetIntl.h"
#include "ApexVertexFormat.h"
#include "VertexBufferParameters.h"
#include <nvparameterized/NvParameterized.h>
#include "ApexSharedUtils.h"
#include "ApexInteropableBuffer.h"

namespace nvidia
{
namespace apex
{

class ApexVertexBuffer : public VertexBufferIntl, public ApexInteropableBuffer, public NvParameterized::SerializationCallback
{
public:
	ApexVertexBuffer();
	~ApexVertexBuffer();

	// from VertexBuffer
	const VertexFormat&	getFormat() const
	{
		return mFormat;
	}
	uint32_t			getVertexCount() const
	{
		return mParams->vertexCount;
	}
	void*					getBuffer(uint32_t bufferIndex);
	void*					getBufferAndFormatWritable(RenderDataFormat::Enum& format, uint32_t bufferIndex)
	{
		return getBufferAndFormat(format, bufferIndex);
	}

	void*					getBufferAndFormat(RenderDataFormat::Enum& format, uint32_t bufferIndex)
	{
		format = getFormat().getBufferFormat(bufferIndex);
		return getBuffer(bufferIndex);
	}
	bool					getBufferData(void* dstBuffer, nvidia::RenderDataFormat::Enum dstBufferFormat, uint32_t dstBufferStride, uint32_t bufferIndex,
	                                      uint32_t startVertexIndex, uint32_t elementCount) const;
	PX_INLINE const void*	getBuffer(uint32_t bufferIndex) const
	{
		return (const void*)((ApexVertexBuffer*)this)->getBuffer(bufferIndex);
	}
	PX_INLINE const void*	getBufferAndFormat(RenderDataFormat::Enum& format, uint32_t bufferIndex) const
	{
		return (const void*)((ApexVertexBuffer*)this)->getBufferAndFormat(format, bufferIndex);
	}

	// from VertexBufferIntl
	void					build(const VertexFormat& format, uint32_t vertexCount);

	VertexFormat&			getFormatWritable()
	{
		return mFormat;
	}
	void					applyTransformation(const PxMat44& transformation);
	void					applyScale(float scale);
	bool					mergeBinormalsIntoTangents();

	void					copy(uint32_t dstIndex, uint32_t srcIndex, ApexVertexBuffer* srcBufferPtr = NULL);
	void					resize(uint32_t vertexCount);

	// from NvParameterized::SerializationCallback

	void					preSerialize(void* userData_);

	void					setParams(VertexBufferParameters* param);
	VertexBufferParameters* getParams()
	{
		return mParams;
	}

	uint32_t			getAllocationSize() const;

	void					applyPermutation(const Array<uint32_t>& permutation);

protected:
	VertexBufferParameters*			mParams;

	ApexVertexFormat				mFormat;	// Wrapper class for mParams->vertexFormat
};


} // namespace apex
} // namespace nvidia


#endif // APEX_VERTEX_BUFFER_H
