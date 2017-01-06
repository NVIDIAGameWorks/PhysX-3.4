/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_RENDER_PROXY_IMPL_H
#define CLOTHING_RENDER_PROXY_IMPL_H

#include "ClothingRenderProxy.h"
#include "PxMat44.h"
#include "PxBounds3.h"
#include "PsHashMap.h"
#include "ApexString.h"
#include "ApexRWLockable.h"

namespace nvidia
{
namespace apex
{
class RenderMeshActorIntl;
class RenderMeshAssetIntl;
}
namespace clothing
{
class ClothingActorParam;
class ClothingScene;


class ClothingRenderProxyImpl : public ClothingRenderProxy, public UserAllocated, public ApexRWLockable
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ClothingRenderProxyImpl(RenderMeshAssetIntl* rma, bool useFallbackSkinning, bool useCustomVertexBuffer, const nvidia::HashMap<uint32_t, ApexSimpleString>& overrideMaterials, const PxVec3* morphTargetNewPositions, const uint32_t* morphTargetVertexOffsets, ClothingScene* scene);
	virtual ~ClothingRenderProxyImpl();

	// from ApexInterface
	virtual void release();

	// from Renderable
	virtual void dispatchRenderResources(UserRenderer& api);
	virtual PxBounds3 getBounds() const
	{
		return mBounds;
	}
	void setBounds(const PxBounds3& bounds)
	{
		mBounds = bounds;
	}

	// from RenderDataProvider.h
	virtual void lockRenderResources();
	virtual void unlockRenderResources();
	virtual void updateRenderResources(bool rewriteBuffers = false, void* userRenderData = 0);

	void setPose(const PxMat44& pose)
	{
		mPose = pose;
	}

	// from ClothingRenderProxy.h
	virtual bool hasSimulatedData() const;

	RenderMeshActorIntl* getRenderMeshActor();
	RenderMeshAssetIntl* getRenderMeshAsset();

	bool usesFallbackSkinning() const
	{
		return mUseFallbackSkinning;
	}

	bool usesCustomVertexBuffer() const
	{
		return renderingDataPosition != NULL;
	}

	const PxVec3* getMorphTargetBuffer() const
	{
		return mMorphTargetNewPositions;
	}

	void setOverrideMaterial(uint32_t i, const char* overrideMaterialName);
	bool overrideMaterialsEqual(const nvidia::HashMap<uint32_t, ApexSimpleString>& overrideMaterials);

	uint32_t getTimeInPool()  const;
	void setTimeInPool(uint32_t time);

	void notifyAssetRelease();

	PxVec3* renderingDataPosition;
	PxVec3* renderingDataNormal;
	PxVec4* renderingDataTangent;

private:
	RenderMeshActorIntl* createRenderMeshActor(RenderMeshAssetIntl* renderMeshAsset, ClothingActorParam* actorDesc);

	PxBounds3 mBounds;
	PxMat44 mPose;

	RenderMeshActorIntl* mRenderMeshActor;
	RenderMeshAssetIntl* mRenderMeshAsset;

	ClothingScene* mScene;

	bool mUseFallbackSkinning;
	HashMap<uint32_t, ApexSimpleString> mOverrideMaterials;
	const PxVec3* mMorphTargetNewPositions; // just to compare, only read it in constructor (it may be released)

	uint32_t mTimeInPool;

	Mutex mRMALock;
};

}
} // namespace nvidia

#endif // CLOTHING_RENDER_PROXY_IMPL_H
