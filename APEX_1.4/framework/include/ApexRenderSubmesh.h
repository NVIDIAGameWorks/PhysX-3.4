/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RENDER_SUBMESH_H
#define APEX_RENDER_SUBMESH_H

#include "RenderMeshAssetIntl.h"
#include "ApexVertexBuffer.h"
#include "SubmeshParameters.h"

namespace nvidia
{
namespace apex
{

class ApexRenderSubmesh : public RenderSubmeshIntl, public UserAllocated
{
public:
	ApexRenderSubmesh() : mParams(NULL) {}
	~ApexRenderSubmesh() {}

	// from RenderSubmesh
	virtual uint32_t				getVertexCount(uint32_t partIndex) const
	{
		return mParams->vertexPartition.buf[partIndex + 1] - mParams->vertexPartition.buf[partIndex];
	}

	virtual const VertexBufferIntl&	getVertexBuffer() const
	{
		return mVertexBuffer;
	}

	virtual uint32_t				getFirstVertexIndex(uint32_t partIndex) const
	{
		return mParams->vertexPartition.buf[partIndex];
	}

	virtual uint32_t				getIndexCount(uint32_t partIndex) const
	{
		return mParams->indexPartition.buf[partIndex + 1] - mParams->indexPartition.buf[partIndex];
	}

	virtual const uint32_t*			getIndexBuffer(uint32_t partIndex) const
	{
		return mParams->indexBuffer.buf + mParams->indexPartition.buf[partIndex];
	}

	virtual const uint32_t*			getSmoothingGroups(uint32_t partIndex) const
	{
		return mParams->smoothingGroups.buf != NULL ? (mParams->smoothingGroups.buf + mParams->indexPartition.buf[partIndex]/3) : NULL;
	}


	// from RenderSubmeshIntl
	virtual VertexBufferIntl&			getVertexBufferWritable()
	{
		return mVertexBuffer;
	}

	virtual uint32_t*						getIndexBufferWritable(uint32_t partIndex)
	{
		return mParams->indexBuffer.buf + mParams->indexPartition.buf[partIndex];
	}

	virtual void						applyPermutation(const Array<uint32_t>& old2new, const Array<uint32_t>& new2old);

	// own methods

	uint32_t						getTotalIndexCount() const
	{
		return (uint32_t)mParams->indexBuffer.arraySizes[0];
	}

	uint32_t*						getIndexBufferWritable(uint32_t partIndex) const
	{
		return mParams->indexBuffer.buf + mParams->indexPartition.buf[partIndex];
	}

	bool								createFromParameters(SubmeshParameters* params);

	void								setParams(SubmeshParameters* submeshParams, VertexBufferParameters* vertexBufferParams);

	void								addStats(RenderMeshAssetStats& stats) const;

	void								buildVertexBuffer(const VertexFormat& format, uint32_t vertexCount);

	SubmeshParameters*  mParams;

private:
	ApexVertexBuffer    mVertexBuffer;

	// No assignment
	ApexRenderSubmesh&					operator = (const ApexRenderSubmesh&);
};

} // namespace apex
} // namespace nvidia

#endif // APEX_RENDER_SUBMESH_H
