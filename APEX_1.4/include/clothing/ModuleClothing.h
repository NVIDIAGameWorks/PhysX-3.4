/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CLOTHING_H
#define MODULE_CLOTHING_H

#include "Module.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class ClothingAsset;
class ClothingAssetAuthoring;
class ClothingPhysicalMesh;

class IProgressListener;
class RenderMeshAssetAuthoring;

/**
\mainpage APEX Clothing API Documentation

\section overview Overview

This document contains a full API documentation for all public classes.
*/



/**
\brief APEX Clothing Module

Used to generate simulated clothing on (mostly humanoid) characters.
*/
class ModuleClothing : public Module
{
public:
	/**
	\brief creates an empty physical mesh. A custom mesh can be assigned to it.
	*/
	virtual ClothingPhysicalMesh* createEmptyPhysicalMesh() = 0;

	/**
	\brief creates a physical mesh based on a render mesh asset. This will be a 1 to 1 copy of the render mesh

	\param [in] asset			The render mesh that is used as source for the physical mesh
	\param [in] subdivision		Modify the physical mesh such that all edges that are longer than (bounding box diagonal / subdivision) are split up. Must be <= 200
	\param [in] mergeVertices	All vertices with the same position will be welded together.
	\param [in] closeHoles		Close any hole found in the mesh.
	\param [in] progress		An optional callback for progress display.
	*/
	virtual ClothingPhysicalMesh* createSingleLayeredMesh(RenderMeshAssetAuthoring* asset, uint32_t subdivision, bool mergeVertices, bool closeHoles, IProgressListener* progress) = 0;

protected:
	virtual ~ModuleClothing() {}
};

#if !defined(_USRDLL)
/**
* If this module is distributed as a static library, the user must call this
* function before calling ApexSDK::createModule("Clothing")
*/
void instantiateModuleClothing();
#endif


PX_POP_PACK

}
} // namespace nvidia

#endif // MODULE_CLOTHING_H
