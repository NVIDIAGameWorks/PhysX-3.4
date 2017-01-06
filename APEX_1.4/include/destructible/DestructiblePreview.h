/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DESTRUCTIBLE_PREVIEW_H
#define DESTRUCTIBLE_PREVIEW_H

#include "AssetPreview.h"
#include "RenderMeshActor.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT


/**
\brief Destructible module's derivation AssetPreview.
*/

class DestructiblePreview : public AssetPreview
{
public:

	/**
	\brief Returns render mesh actor
	*/
	virtual const RenderMeshActor* getRenderMeshActor() const = 0;

	/**
	\brief Sets the preview chunk depth and "explode" amount (pushes the chunks out by that relative distance).
	\param [in] depth Must be nonnegative.  Internally the value will be clamped to the depth range in the asset.
	\param [in] explode must be nonnegative.
	*/
	virtual void	setExplodeView(uint32_t depth, float explode) = 0;

protected:
	DestructiblePreview() {}
};

PX_POP_PACK

}
} // end namespace nvidia

#endif // DESTRUCTIBLE_PREVIEW_H
