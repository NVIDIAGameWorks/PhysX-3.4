/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_ACTOR_DATA_H
#define CLOTHING_ACTOR_DATA_H

#include "ClothingAssetData.h"
#include "PsMutex.h"
#include "PxMat44.h"
#include "PxVec3.h"

#define BLOCK_SIZE_SKIN_PHYSICS (32768*6) + (8192*0)

namespace nvidia
{
namespace clothing
{

/*
 * A wrapper around the data contained in ClothingActorImpl minus all the interface stuff
 */
class ClothingActorData
{
public:

	ClothingActorData();
	~ClothingActorData();

	void skinToAnimation_NoPhysX(bool fromFetchResults);

	void skinPhysicsMaxDist0Normals_NoPhysx();

	void renderDataLock();

	void renderDataUnLock();

	void skinToPhysicalMesh_NoPhysX(bool fromFetchResults);

	void skinToImmediateMap(const uint32_t* immediateClothMap_, uint32_t numGraphicalVertices_, uint32_t numSrcVertices_,
	                        const PxVec3* srcPositions_);

	void skinToImmediateMap(const uint32_t* immediateClothMap_, uint32_t numGraphicalVertices_, uint32_t numSrcVertices_,
	                        const PxVec3* srcPositions_, const PxVec3* srcNormals_);

	void finalizeSkinning_NoPhysX(bool fromFetchResults);

	template <bool withNormals, bool withTangents>
	void computeTangentSpaceUpdate(AbstractMeshDescription& destMesh, const ClothingMeshAssetData& rendermesh, uint32_t submeshIndex, const uint32_t* compressedTangentW);

	void tickSynchAfterFetchResults_LocksPhysX();

	uint32_t skinPhysicsSimpleMem() const;
	void skinPhysicsMeshSimple();

	PxBounds3 getRenderMeshAssetBoundsTransformed();

	// is valid only after lodTick
	bool calcIfSimplePhysicsMesh() const;

	PX_ALIGN(16, nvidia::AtomicLockCopy mRenderLock);

	ClothingAssetData mAsset;

	PxBounds3 mNewBounds;

	PX_ALIGN(16, PxMat44 mGlobalPose);
	PxMat44 mInternalGlobalPose;

	PxMat44* mInternalBoneMatricesCur;
	PxMat44* mInternalBoneMatricesPrev;

	PxVec3* mRenderingDataPosition;
	PxVec3* mRenderingDataNormal;
	PxVec4* mRenderingDataTangent;
	PxVec3* mMorphDisplacementBuffer;

	PxVec3* mSdkWritebackNormal;
	PxVec3* mSdkWritebackPositions;

	PxVec3* mSkinnedPhysicsPositions;
	PxVec3* mSkinnedPhysicsNormals;

	uint32_t mInternalMatricesCount;
	uint32_t mMorphDisplacementBufferCount;
	uint32_t mSdkDeformableVerticesCount;
	uint32_t mSdkDeformableIndicesCount;

	uint32_t mCurrentGraphicalLodId;
	uint32_t mCurrentPhysicsSubmesh;

	float mActorScale;

	// Note - all these modified bools need to be written back to the actor!!!!
	bool bInternalFrozen;
	bool bShouldComputeRenderData;
	bool bIsInitialized;
	bool bIsSimulationMeshDirty;
	bool bRecomputeNormals;
	bool bRecomputeTangents;
	bool bCorrectSimulationNormals;
	bool bParallelCpuSkinning;
	bool bIsClothingSimulationNull;
};

}
} // namespace nvidia

#endif // CLOTHING_ACTOR_DATA_H
