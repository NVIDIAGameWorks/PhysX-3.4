/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RENDERMESH_ASSET_AUTHORING_H
#define APEX_RENDERMESH_ASSET_AUTHORING_H

#include "ApexRenderMeshAsset.h"
#include "ApexSharedUtils.h"
#include "RenderMeshAssetIntl.h"
#include "ResourceProviderIntl.h"
#include "ApexResource.h"
#include "ApexActor.h"
#include "ApexAssetAuthoring.h"
#include "ApexString.h"
#include "ApexVertexFormat.h"
#include "ApexSDKImpl.h"
#include "ApexUsingNamespace.h"
#include "ApexRWLockable.h"

#ifndef WITHOUT_APEX_AUTHORING

namespace nvidia
{
namespace apex
{

// PHTODO, put this into the authoring asset
struct VertexReductionExtraData
{
	void	set(const ExplicitRenderTriangle& xTriangle)
	{
		smoothingMask = xTriangle.smoothingMask;
	}

	bool	canMerge(const VertexReductionExtraData& other) const
	{
		return (smoothingMask & other.smoothingMask) != 0 || smoothingMask == 0 || other.smoothingMask == 0;
	}

	uint32_t smoothingMask;
};


class ApexRenderMeshAssetAuthoring : public ApexRenderMeshAsset, public ApexAssetAuthoring, public RenderMeshAssetAuthoringIntl
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexRenderMeshAssetAuthoring(ResourceList& list, RenderMeshAssetParameters* params, const char* name);

	void						release()
	{
		GetApexSDK()->releaseAssetAuthoring(*this);
	}

	void						createRenderMesh(const MeshDesc& desc, bool createMappingInformation);
	uint32_t				createReductionMap(uint32_t* map, const Vertex* vertices, const uint32_t* smoothingGroups, uint32_t vertexCount,
	        const PxVec3& positionTolerance, float normalTolerance, float UVTolerance);

	void						deleteStaticBuffersAfterUse(bool set)
	{
		ApexRenderMeshAsset::deleteStaticBuffersAfterUse(set);
	}

	const char*					getName(void) const
	{
		return ApexRenderMeshAsset::getName();
	}
	const char* 				getObjTypeName() const
	{
		return ApexRenderMeshAsset::getClassName();
	}
	bool						prepareForPlatform(nvidia::apex::PlatformTag)
	{
		APEX_INVALID_OPERATION("Not Implemented.");
		return false;
	}
	void						setToolString(const char* toolName, const char* toolVersion, uint32_t toolChangelist)
	{
		ApexAssetAuthoring::setToolString(toolName, toolVersion, toolChangelist);
	}
	uint32_t				getSubmeshCount() const
	{
		return ApexRenderMeshAsset::getSubmeshCount();
	}
	uint32_t				getPartCount() const
	{
		return ApexRenderMeshAsset::getPartCount();
	}
	const char* 				getMaterialName(uint32_t submeshIndex) const
	{
		return ApexRenderMeshAsset::getMaterialName(submeshIndex);
	}
	void						setMaterialName(uint32_t submeshIndex, const char* name);
	virtual void				setWindingOrder(uint32_t submeshIndex, RenderCullMode::Enum winding);
	virtual RenderCullMode::Enum	getWindingOrder(uint32_t submeshIndex) const;
	const RenderSubmesh&		getSubmesh(uint32_t submeshIndex) const
	{
		return ApexRenderMeshAsset::getSubmesh(submeshIndex);
	}
	RenderSubmesh&			getSubmeshWritable(uint32_t submeshIndex)
	{
		return *mSubmeshes[submeshIndex];
	}
	const PxBounds3&		getBounds(uint32_t partIndex = 0) const
	{
		return ApexRenderMeshAsset::getBounds(partIndex);
	}
	void						getStats(RenderMeshAssetStats& stats) const
	{
		ApexRenderMeshAsset::getStats(stats);
	}

	// From RenderMeshAssetAuthoringIntl
	RenderSubmeshIntl&		getInternalSubmesh(uint32_t submeshIndex)
	{
		return *ApexRenderMeshAsset::mSubmeshes[submeshIndex];
	}
	void						permuteBoneIndices(const physx::Array<int32_t>& old2new)
	{
		ApexRenderMeshAsset::permuteBoneIndices(old2new);
	}
	void						applyTransformation(const PxMat44& transformation, float scale)
	{
		ApexRenderMeshAsset::applyTransformation(transformation, scale);
	}
	void						applyScale(float scale)
	{
		ApexRenderMeshAsset::applyScale(scale);
	}
	void						reverseWinding()
	{
		ApexRenderMeshAsset::reverseWinding();
	}
	NvParameterized::Interface*	getNvParameterized() const
	{
		return mParams;
	}
	/**
	 * \brief Releases the ApexAsset but returns the NvParameterized::Interface and *ownership* to the caller.
	 */
	virtual NvParameterized::Interface* releaseAndReturnNvParameterizedInterface(void)
	{
		NvParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}

protected:
	// helper structs
	struct VertexPart
	{
		uint32_t	part, vertexIndex;
		PX_INLINE bool operator()(const VertexPart& a, const VertexPart& b) const
		{
			if (a.part != b.part)
			{
				return a.part < b.part;
			}
			return a.vertexIndex < b.vertexIndex;
		}
		PX_INLINE static int cmp(const void* A, const void* B)
		{
			// Sorts by part, then vertexIndex
			const int delta = (int)((VertexPart*)A)->part - (int)((VertexPart*)B)->part;
			return delta != 0 ? delta : ((int)((VertexPart*)A)->vertexIndex - (int)((VertexPart*)B)->vertexIndex);
		}
	};

	// helper methods
	template<typename PxU>
	bool fillSubmeshMap(physx::Array<VertexPart>& submeshMap, const void* const partIndicesVoid, uint32_t numParts,
	                    const void* const vertexIndicesVoid, uint32_t numSubmeshIndices, uint32_t numSubmeshVertices);

	// protected constructors
	ApexRenderMeshAssetAuthoring(ResourceList& list);
	virtual ~ApexRenderMeshAssetAuthoring();
};

}
} // end namespace nvidia::apex

#endif // WITHOUT_APEX_AUTHORING

#endif // APEX_RENDERMESH_ASSET_H
