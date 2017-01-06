/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef APEX_RENDER_RESOURCES_H
#define APEX_RENDER_RESOURCES_H

#include "ApexRenderMaterial.h"

#include "Apex.h"
#include "Utils.h"

#include <vector>

using namespace physx;
using namespace nvidia;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampleApexRendererVertexBuffer : public apex::UserRenderVertexBuffer
{
  public:
	SampleApexRendererVertexBuffer(ID3D11Device& d3dDevice, const apex::UserRenderVertexBufferDesc& desc);
	virtual ~SampleApexRendererVertexBuffer(void);

	void bind(ID3D11DeviceContext& context, uint32_t streamID, uint32_t firstVertex);
	void unbind(ID3D11DeviceContext& context, uint32_t streamID);
	void addVertexElements(uint32_t streamIndex, std::vector<D3D11_INPUT_ELEMENT_DESC>& vertexElements) const;

  protected:
	virtual void writeBuffer(const apex::RenderVertexBufferData& data, uint32_t firstVertex, uint32_t numVerts);

  private:
	uint32_t mStride;
	uint32_t mVerticesCount;
	uint8_t mPointers[RenderVertexSemantic::NUM_SEMANTICS];
	RenderDataFormat::Enum mDataFormat[RenderVertexSemantic::NUM_SEMANTICS];

	ID3D11Device& mDevice;
	ID3D11Buffer* mVertexBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampleApexRendererIndexBuffer : public apex::UserRenderIndexBuffer
{
  public:
	SampleApexRendererIndexBuffer(ID3D11Device& d3dDevice, const apex::UserRenderIndexBufferDesc& desc);
	virtual ~SampleApexRendererIndexBuffer(void);

	void bind(ID3D11DeviceContext& context);
	void unbind(ID3D11DeviceContext& context);

	uint32_t GetIndicesCount()
	{
		return mIndicesCount;
	}

  private:
	virtual void writeBuffer(const void* srcData, uint32_t srcStride, uint32_t firstDestElement, uint32_t numElements);

  private:
	uint32_t mIndicesCount;
	uint32_t mStride;

	ID3D11Device& mDevice;
	ID3D11Buffer* mIndexBuffer;
	DXGI_FORMAT mFormat;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampleApexRendererBoneBuffer : public apex::UserRenderBoneBuffer
{
  public:
	SampleApexRendererBoneBuffer(ID3D11Device& d3dDevice, const apex::UserRenderBoneBufferDesc& desc);
	virtual ~SampleApexRendererBoneBuffer(void);

	void bind(uint32_t slot);
	void unbind();

  public:
	virtual void writeBuffer(const apex::RenderBoneBufferData& data, uint32_t firstBone, uint32_t numBones);

  private:
	uint32_t mMaxBones;
	PxMat44* mBones;

	ID3D11Device& mDevice;

	ID3D11Texture2D* mTexture;
	ID3D11ShaderResourceView* mShaderResourceView;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampleApexRendererSpriteBuffer : public apex::UserRenderSpriteBuffer
{
public:
	SampleApexRendererSpriteBuffer(ID3D11Device& d3dDevice, const apex::UserRenderSpriteBufferDesc& desc);
	virtual ~SampleApexRendererSpriteBuffer(void);

	void addVertexElements(uint32_t streamIndex, std::vector<D3D11_INPUT_ELEMENT_DESC>& vertexElements) const;

	void bind(ID3D11DeviceContext& context, uint32_t streamID, uint32_t firstVertex);
	void unbind(ID3D11DeviceContext& context, uint32_t streamID);

	virtual void writeBuffer(const void* data, uint32_t firstSprite, uint32_t numSprites);
	virtual void writeTexture(uint32_t textureId, uint32_t numSprites, const void* srcData, size_t srcSize);

private:
	ID3D11Device& mDevice;

	uint32_t mStride;
	RenderDataFormat::Enum mDataFormat[RenderVertexSemantic::NUM_SEMANTICS];

	ID3D11Buffer* mVertexBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampleApexRendererInstanceBuffer : public apex::UserRenderInstanceBuffer
{
public:
	SampleApexRendererInstanceBuffer(ID3D11Device& d3dDevice, const apex::UserRenderInstanceBufferDesc& desc);
	virtual ~SampleApexRendererInstanceBuffer(void);

	void addVertexElements(uint32_t streamIndex, std::vector<D3D11_INPUT_ELEMENT_DESC>& vertexElements) const;

	void bind(ID3D11DeviceContext& context, uint32_t streamID, uint32_t firstVertex);
	void unbind(ID3D11DeviceContext& context, uint32_t streamID);

	virtual void writeBuffer(const void* data, uint32_t firstInstance, uint32_t numInstances);

private:
	ID3D11Device& mDevice;

	uint32_t mStride;
	vector<RenderDataFormat::Enum> mDataFormat;

	ID3D11Buffer* mInstanceBuffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SampleApexRendererMesh : public apex::UserRenderResource
{
  public:
	SampleApexRendererMesh(ID3D11Device& d3dDevice, const apex::UserRenderResourceDesc& desc);
	virtual ~SampleApexRendererMesh();

  public:
	virtual void setVertexBufferRange(uint32_t firstVertex, uint32_t numVerts);
	virtual void setIndexBufferRange(uint32_t firstIndex, uint32_t numIndices);
	virtual void setBoneBufferRange(uint32_t firstBone, uint32_t numBones);
	virtual void setInstanceBufferRange(uint32_t firstInstance, uint32_t numInstances);
	virtual void setSpriteBufferRange(uint32_t firstSprite, uint32_t numSprites);

	virtual void setMaterial(void* material);

	uint32_t getNbVertexBuffers() const
	{
		return mNumVertexBuffers;
	}

	apex::UserRenderVertexBuffer* getVertexBuffer(uint32_t index) const
	{
		apex::UserRenderVertexBuffer* buffer = 0;
		PX_ASSERT(index < mNumVertexBuffers);
		if(index < mNumVertexBuffers)
		{
			buffer = mVertexBuffers[index];
		}
		return buffer;
	}

	apex::UserRenderIndexBuffer* getIndexBuffer() const
	{
		return mIndexBuffer;
	}

	apex::UserRenderBoneBuffer* getBoneBuffer() const
	{
		return mBoneBuffer;
	}

	apex::UserRenderInstanceBuffer* getInstanceBuffer() const
	{
		return NULL;
	}

	apex::UserRenderSpriteBuffer* getSpriteBuffer() const
	{
		return mSpriteBuffer;
	}

	void render(const apex::RenderContext& context);

  private:
	SampleApexRendererVertexBuffer** mVertexBuffers;
	uint32_t mNumVertexBuffers;
	uint32_t mFirstVertex;
	uint32_t mNumVertices;

	SampleApexRendererIndexBuffer* mIndexBuffer;
	uint32_t mFirstIndex;
	uint32_t mNumIndices;

	SampleApexRendererBoneBuffer* mBoneBuffer;
	uint32_t mFirstBone;
	uint32_t mNumBones;

	SampleApexRendererSpriteBuffer* mSpriteBuffer;
	uint32_t mFirstSprite;
	uint32_t mNumSprites;

	SampleApexRendererInstanceBuffer* mInstanceBuffer;
	uint32_t mFirstInstance;
	uint32_t mNumInstances;

	PxMat44 mMeshTransform;
	apex::RenderCullMode::Enum mCullMode;

	ApexRenderMaterial* mMaterial;
	ApexRenderMaterial::Instance* mMaterialInstance;

	D3D11_PRIMITIVE_TOPOLOGY mPrimitiveTopology;

	ID3D11Device& mDevice;
};

#endif