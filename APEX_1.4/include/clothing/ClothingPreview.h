/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CLOTHING_PREVIEW_H
#define CLOTHING_PREVIEW_H

#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT



/**
\brief A lightweight preview of a ClothingActor that will not perform any simulation
*/
class ClothingPreview : public AssetPreview
{

public:

	/**
	\brief Updates all internal bone matrices.
	\see ClothingActor::updateState
	*/
	virtual void updateState(const PxMat44& globalPose, const PxMat44* newBoneMatrices, uint32_t boneMatricesByteStride, uint32_t numBoneMatrices) = 0;

};


PX_POP_PACK

}
} // namespace nvidia

#endif // CLOTHING_PREVIEW_H
