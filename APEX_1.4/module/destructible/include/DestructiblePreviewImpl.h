/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __DESTRUCTIBLE_PREVIEW_IMPL_H__
#define __DESTRUCTIBLE_PREVIEW_IMPL_H__

#include "Apex.h"
#include "ApexPreview.h"
#include "RenderMesh.h"
#include "DestructiblePreview.h"

namespace nvidia
{
namespace destructible
{
class DestructiblePreviewImpl : public ApexResource, public ApexPreview
{
public:

	DestructibleAssetImpl*		m_asset;
	DestructiblePreview*	m_api;

	uint32_t			m_chunkDepth;
	float			m_explodeAmount;

	RenderMeshActor*		m_renderMeshActors[DestructibleActorMeshType::Count];	// Indexed by DestructibleActorMeshType::Enum

	physx::Array<RenderMeshActor*>	m_instancedChunkRenderMeshActors;	// One per render mesh actor per instanced chunk
	physx::Array<uint16_t>			m_instancedActorVisiblePart;

	physx::Array<uint16_t>			m_instancedChunkActorMap;	// from instanced chunk instanceInfo index to actor index

	physx::Array<UserRenderInstanceBuffer*>	m_chunkInstanceBuffers;
	physx::Array< physx::Array< DestructibleAssetImpl::ChunkInstanceBufferDataElement > >	m_chunkInstanceBufferData;

	bool					m_drawUnexpandedChunksStatically;

	void*					m_userData;

	void				setExplodeView(uint32_t depth, float explode);

	void				updateRenderResources(bool rewriteBuffers, void* userRenderData);

	RenderMeshActor*	getRenderMeshActor() const
	{
		return m_renderMeshActors[(!m_drawUnexpandedChunksStatically || m_explodeAmount != 0.0f) ? DestructibleActorMeshType::Skinned : DestructibleActorMeshType::Static];
	}

	void				updateRenderResources(void* userRenderData);
	void				dispatchRenderResources(UserRenderer& renderer);

	// ApexPreview methods
	void				setPose(const PxMat44& pose);
	Renderable*	getRenderable()
	{
		return DYNAMIC_CAST(Renderable*)(m_api);
	}
	void				release();
	void				destroy();

	DestructiblePreviewImpl(DestructiblePreview* _api, DestructibleAssetImpl& _asset, const NvParameterized::Interface* params);
	virtual				~DestructiblePreviewImpl();

protected:
	void					setChunkVisibility(uint16_t index, bool visibility)
	{
		PX_ASSERT((int32_t)index < m_asset->mParams->chunks.arraySizes[0]);
		if (visibility)
		{
			mVisibleChunks.use(index);
		}
		else
		{
			mVisibleChunks.free(index);
		}
		DestructibleAssetParametersNS::Chunk_Type& sourceChunk = m_asset->mParams->chunks.buf[index];
		if ((sourceChunk.flags & DestructibleAssetImpl::Instanced) == 0)
		{
			// Not instanced - need to choose the static or dynamic mesh, and set visibility for the render mesh actor
			const DestructibleActorMeshType::Enum typeN = (m_explodeAmount != 0.0f || !m_drawUnexpandedChunksStatically) ?
					DestructibleActorMeshType::Skinned : DestructibleActorMeshType::Static;
			m_renderMeshActors[typeN]->setVisibility(visibility, sourceChunk.meshPartIndex);
		}
	}


	void				setInstancedChunkCount(uint32_t count);

	IndexBank<uint16_t>	mVisibleChunks;
	uint16_t				m_instancedChunkCount;
};

}
} // end namespace nvidia

#endif // __DESTRUCTIBLEACTOR_H__
